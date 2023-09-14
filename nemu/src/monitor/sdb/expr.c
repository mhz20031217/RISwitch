/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <asm-generic/errno-base.h>
#include <isa.h>
#include "debug.h"
#include "memory/paddr.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <errno.h>


typedef enum {
  TK_NOTYPE = 256, TK_EQ, TK_NE,
  TK_POS, TK_NEG,
  TK_ADD, TK_MINUS, TK_MUL, TK_DIV,
  TK_LPAR, TK_RPAR,
  TK_AND, TK_OR, TK_DREF,

  TK_NUM, TK_HEX, TK_REG
  /* TODO: Add more token types */

} token_type;

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"[ \t\n]+", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"\\+", TK_ADD},
  {"-", TK_MINUS},
  {"\\*", TK_MUL},
  {"/", TK_DIV},
  {"\\(", TK_LPAR},
  {"\\)", TK_RPAR},
  {"\\$[a-zA-Z][a-zA-Z0-9]*", TK_REG},
  {"0x[0-9a-zA-Z]+", TK_HEX},
  {"[0-9]+", TK_NUM},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //   i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_REG:
            // excluding '$'
            strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_NUM:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_DREF:
          case TK_POS: case TK_NEG: 
          case TK_ADD: case TK_MINUS: case TK_MUL: case TK_DIV:
          case TK_EQ: case TK_NE:
          case TK_LPAR: case TK_RPAR:
          case TK_AND: case TK_OR: // fall through
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_NOTYPE: // simply discard
            break;
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      Error("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/*
  Change binary operator types to unary operator types, inspired by

*/ 

static int binary_to_unary_patch(Token tokens[], int length, token_type a, token_type b) {
  if (length == 0) return 0;
  int count = 0;
  if (tokens[0].type == a) {
    tokens[0].type = b;
    ++count;
  }
  for (int i = 1; i < length; ++i) {
    if (tokens[i].type == a) {
      if (!(tokens[i - 1].type == TK_NUM || tokens[i - 1].type == TK_HEX 
            || tokens[i - 1].type == TK_REG || tokens[i - 1].type == TK_RPAR)) {
        tokens[i].type = b;
        ++count;
      }
    }
  }
  return count;
}

static int pos_neg_dref_patch(Token tokens[], int length) {
  if (length == 0) return 0;
  int count = 0;
  count += binary_to_unary_patch(tokens, length, TK_ADD, TK_POS);
  count += binary_to_unary_patch(tokens, length, TK_MINUS, TK_NEG);
  count += binary_to_unary_patch(tokens, length, TK_MUL, TK_DREF);
  return count;
}

#define push_op(x) st_operator[pt_operator] = (x), pt_operator++
#define push_val(x) st_operand[pt_operand] = (x), pt_operand++
#define pop_op() (pt_operator--)
#define pop_val() (pt_operand--)
#define peek_op() (st_operator[pt_operator - 1])
#define peek_val() (st_operand[pt_operand - 1])
#define size_op() pt_operator
#define size_val() pt_operand
#define empty_op() (pt_operator == 0)
#define empty_val() (pt_operand == 0)


static token_type *st_operator;
static int64_t *st_operand;
static int pt_operator, pt_operand;

static int64_t apply_unary(token_type op, int64_t x) {
  switch (op) {
    case TK_POS:
      return x;
    case TK_NEG:
      return -x;
    case TK_DREF:
      return paddr_read((paddr_t) x, 4);
    default:
      TODO();
      return 0;
  }
  return 0;
}

static int64_t apply_binary(token_type op, int64_t a, int64_t b) {
  switch (op) {
    case TK_ADD:
      return a + b;
    case TK_MINUS:
      return a - b;
    case TK_MUL:
      return a * b;
    case TK_DIV:
      if (b == 0) {
        Warning("divided by zero, result set to 0.");
        return 0;
      }
      return a / b;
    case TK_AND:
      return a && b;
    case TK_OR:
      return a || b;
    case TK_EQ:
      return a == b;
    case TK_NE:
      return a != b;
    default:
      TODO();
      return 0;
  }
  return 0;
}

static void apply() {
  if (empty_op()) goto error;
  token_type op = peek_op(); pop_op();
  if (op == TK_POS || op == TK_NEG || op == TK_DREF) {
    if (empty_val()) goto error;
    int64_t x = peek_val(); pop_val();
    push_val(apply_unary(op, x));
  } else {
    int64_t a, b;
    if (empty_val()) goto error;
    b = peek_val(); pop_val();
    if (empty_val()) goto error;
    a = peek_val(); pop_val();
    push_val(apply_binary(op, a, b));
  }
  return;
error:
  errno = EINVAL;
  return;
}

static int operator_level(token_type o) {
  int l;
  switch (o) {
    // level 1
    case TK_DREF:
      l = 1; break;
    // level 2 (unary)
    case TK_POS: case TK_NEG:
      l = 2; break;
    // level 3 (binary arith: mul, div)
    case TK_MUL: case TK_DIV:
      l = 3; break;
    // level 4 (binary arith: add, sub)
    case TK_ADD: case TK_MINUS:
      l = 4; break;
    // level 8 (equation)
    case TK_EQ: case TK_NE:
      l = 8; break;
    // level 12 (&&)
    case TK_AND:
      l = 12; break;
    // level 13 (||)
    case TK_OR:
      l = 13; break;
    default:
      TODO();
      l = 1000;
  }
  return l;
}

static int compare_operator_level(token_type a, token_type b) {
  int al, bl;
  al = operator_level(a);
  bl = operator_level(b);
  return al - bl;
}

static int64_t eval(Token tokens[], int length) {
  int64_t value = 0;

  int p = 0;
  st_operator = calloc(length, sizeof(token_type));
  st_operand = calloc(length, sizeof(int64_t));
  if (st_operator == NULL || st_operand == NULL) goto error;

  pt_operator = pt_operand = 0;

  for (p = 0; p < length; ++p) {
    Token *token = &tokens[p];
    // Log("evaluating tokens[%d]: %d %s", p, token->type, token->str);

    if (token->type == TK_NUM) {
      // Log("case 1");
      push_val(strtoul(token->str, NULL, 10));
      if (errno) goto error;
    } else if (token->type == TK_HEX) {
      push_val(strtoul(token->str, NULL, 16));
      if (errno) goto error;
    } else if (token->type == TK_LPAR) {
      // Log("case 2");
      push_op(TK_LPAR);
    } else if (token->type == TK_RPAR) {
      // Log("case 3");
      while (!empty_op() && peek_op() != TK_LPAR) {
        apply();
        if (errno) goto error;
      }
      if (!empty_op()) pop_op();
    } else if (token->type == TK_REG) {
      // Log("case 4");
      bool success = 0;
      word_t result = isa_reg_str2val(token->str, &success);
      if (!success) goto error;
      push_val(result);
    } else{
      // Log("case 5");
      token_type nop = token->type;
      while (!empty_op() && peek_op() != TK_LPAR && compare_operator_level(nop, peek_op()) >= 0) {
        apply();
        if (errno) goto error;
      }
      push_op(nop);
    }

//    for (int i = 0; i < pt_operator; ++i) {
//      fprintf(stderr, "%d ", st_operator[i]);
//    }
//    fprintf(stderr, "\n");
//    for (int i = 0; i < pt_operand; ++i) {
//      fprintf(stderr, "%ld ", st_operand[i]);
//    }
//    fprintf(stderr, "\n");
  }

  while (!empty_op()) {
    apply();
    if (errno) goto error;
  }

  value = peek_val();

  pt_operand = pt_operator = 0;
  free(st_operand);
  free(st_operator);
  return value;
error:
  if (st_operand != NULL) free(st_operand);
  if (st_operator != NULL) free(st_operator);
  pt_operand = pt_operator = 0;
  errno = EINVAL;
  Error("syntax error near token number: %d", p);
  return 0;
}

#undef push_op
#undef push_val
#undef pop_op
#undef pop_val
#undef peek_op
#undef peek_val
#undef size_op
#undef size_val
#undef empty_op
#undef empty_val

word_t expr(char *e, bool *success) {
  memset(tokens, 0, sizeof(tokens));
  // Log("evaluating expression: %s", e);
  if (!make_token(e)) {
    *success = false;
    // Log("make_token failed.");
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // int replace_count = 
    pos_neg_dref_patch(tokens, nr_token);
  // Log("replaced %d TK_ADD (TK_MINUS) to TK_ADD (TK_NEG).", replace_count);

  // Log("tokenizing and patching done, now tokens:");
  for (int i = 0; i < nr_token; ++i) {
    // Log("Token %d: %d %s", i, tokens[i].type, tokens[i].str);
  }

  errno = 0;
  int64_t result = eval(tokens, nr_token);
  if (errno) {
    // Log("eval failed.");
    *success = false;
    errno = 0;
    return 0;
  }
  // Log("eval succeed. value: %ld, converted to %u", result, (word_t) result);
  *success = true;
  return result;
}
