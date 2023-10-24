# PA2 实验报告

**实验进度**：我完成了所有必做题和部分选做题。

## 必做题

### 程序是个状态机：理解YEMU如何执行程序

1. 加法程序的状态机

> State ID | PC | Reg         | Mem  
> 0          00   00 00 00 00   E6 04 E5 11 F7 10 21 00 00 00 00 00 00 00 00 00  
> 1          01   21 00 00 00   ...  
> 2          02   21 21 00 00   ...  
> 3          03   10 21 00 00   ...  
> 4          04   31 21 00 00   ...  
> 5          05   31 21 00 00   E6 04 E5 11 F7 10 21 31 00 00 00 00 00 00 00 00  
>   
>   
> Graph  
> 0 -> 1 -> 2 -> 3 -> 4 -> 5  

2. YEMU 如何执行一条指令

> 首先，在 `main` 中通过死循环不断执行指令，如果停机，则退出循环。
> 
> 每次通过 `exec_once` 执行一条指令。其中，先取指，然后译码，然后执行。
>
> 取指过程，因为 YEMU 为定长指令字架构，定义了 `inst_t`，用“位域”这一 C 语言特性，简化了译码的操作。
> 执行后，因为 YEMU 没有跳转指令，所以 `pc ++`。

两者间关系：YEMU 执行指令的过程，就是状态间转移的过程。

### RTFSC理解指令执行的过程

> 以下对指令执行的分析，从 `nemu/src/cpu/cpu-exec.c:exec_once` 开始。
>
> 首先，在 `nemu/src/cpu/cpu-exec.c:execute` 中已经被创建好的 `Decode` 类型结构体和当前 `pc` 被传入 `exec_once` 中。`exec_once` 根据接下来 `isa_exec_once` 的结果 `dnpc`（保存在 `Decode s` 中），更新全局变量 `cpu` 中 `pc` 的值。这是更新 PC 的步骤。
>
> 然后，取指阶段，译码阶段执行阶段因为和 ISA 相关，抽象在 `nemu/src/isa/riscv32/inst.c:isa_exec_once` 中。先进行 `inst_fetch` 取指令，然后调用 `decode_exec` 进行译码和执行。
>
> 取指的细节：`inst_fetch` 通过调用 `vaddr_ifetch` （猜测和后面支持虚拟内存有关，现在仅仅是直接转发到 `paddr_read` 直接读内存），在虚拟地址里取了指令，然后更新 `pc`。如果是 x86，变长指令字的支持在这里完成。
> 
> 执行的细节：在 `decode_exec` 中，首先解析出 `imm`，`src1`，`src2` 的值，然后匹配指令，执行对应的模拟代码。这里要特别感谢 yzh 老师重构的框架，`INSTPAT` 宏为我们添加指令提供了极大方便，而且没有降低运行效率。

### 实现更多的指令

目前已经实现了除了异常处理外的 RISC-V32IM 所有指令，通过了 `alu-test` 和所有 `cpu-test`（写这篇实验报告时已经完成了 klib 和 ioe）。

![cpu-tests](img/cpu-tests.png)

![alu-tests](img/alu-tests.png)

能够正确运行 typing-game，snake，mario 等。

![typing-game](img/typing-game.png)

![snake](img/snake.png)

![mario](img/mario.png)

### 实现 iringbuf

> 我的 iringbuf 实现大量使用了宏，算作 C 宏的一个小练习。提供的接口如下：
>
> ```c
> /* Instruction ringbuf */
> #define MAX_IRINGBUF_ENTRY 32
> static char iringbuf[MAX_IRINGBUF_ENTRY][128];
> static int iringbuf_l = 0, iringbuf_r = 0, iringbuf_size = 0;
> #define iringbuf_size() iringbuf_size
> #define iringbuf_empty() (iringbuf_size == 0)
> #define iringbuf_move(var) do { \
>   if (iringbuf_ ## var == MAX_IRINGBUF_ENTRY - 1) { \
>     iringbuf_ ## var = 0; \
>   } else { \
>     iringbuf_ ## var ++; \
>   } \
> } while (0)
> #define iringbuf_push(s) do { \
>   if (iringbuf_size == MAX_IRINGBUF_ENTRY) { \
>     strcpy(iringbuf[iringbuf_l], (s)); \
>     iringbuf_move(l); \
>     iringbuf_move(r); \
>   } else { \
>     strcpy(iringbuf[iringbuf_r], (s)); \
>     iringbuf_move(r); \
>     iringbuf_size ++; \
>   } \
> } while (0)
> #define iringbuf_pop() do { \
>   if (iringbuf_size != 0) { \
>     iringbuf_move(l); \
>     iringbuf_size --; \
>   } \
> } while (0)
> #define iringbuf_front() (iringbuf[iringbuf_l])
> ```

### 实现 mtrace

> 基本功能已实现，Kconfig 中配置选项已实现，运行时条件控制打算以后需要时实现。

### 实现 ftrace

> 我添加的命令行选项是 `-e '/path/to/elf-file'`。ftrace 打印的实现在 `nemu/src/utils/ftrace.c`，调用 ftrace 的逻辑嵌入了指令译码的部分 `inst.c`。
>
> 关于如何识别 `jal`，`jalr` 是函数调用指令还是返回，判断方法如下：
> 
> 如果是 `jal`，只有 `rd` 为 `ra` 时是函数调用一种情况。
>
> 如果是 `jal`，当 `rd` 为 `ra` 时是函数调用，`rs1` 是 `ra` 则为调用返回。

![ftrace](img/ftrace1.png)
![ftrace](img/ftrace2.png)

### 不匹配的函数调用和返回

> 尾递归优化。
>
> 在反汇编结果中，可以明显看见连续的 `ret` 指令：
>
> ```
> 80000190:	012787b3          	add	a5,a5,s2
> 80000194:	00151513          	slli	a0,a0,0x1
> 80000198:	00412483          	lw	s1,4(sp)
> 8000019c:	00012903          	lw	s2,0(sp)
> 800001a0:	00a78533          	add	a0,a5,a0
> 800001a4:	01010113          	addi	sp,sp,16
> 800001a8:	00008067          	ret
> 800001ac:	00008067          	ret
> ```
>
> 在 `recursion.c` 中，可以看见，在 `f{0, 1, 2, 3}` 中，递归调用都是整个函数中的最后一步，所以编译器进行了尾递归优化。

### 看看NEMU跑多快

> 我不确定测试是否正确，测试结果不和常理。
>
> 一开始，我的 NEMU 运行速度处于一个正常区间，运行 microbench ref 大概要 10 分钟，运行 litenes mario 大概 12 fps，am-tests video test 大概 2 fps。时钟函数改成 `clock_gettime`，mario 性能稍有提升。
>
> 根据讲义中的提示查看并更改时钟源。我的电脑处理器是 AMD Ryzen 5 4500U，linux 内核版本是
>
> ```sh
> $ uname -a
> Linux pc 6.0.18 #1 SMP PREEMPT_DYNAMIC Thu Jan 26 23:06:03 CST 2023 x86_64 x86_64 x86_64 GNU/Linux
> ```
>
> 然而，虽然是 x86 体系结构，并没有 `tsc` 时钟源。猜测可能和处理器为 AMD 而非 Intel 有关。STFW 后，发现这是由于 linux 内核启动时探测到 `tsc` 在 AMD 平台上不够稳定，默认禁用导致的。加入 linux 命令行 `tsc=reliable`，强制启用 `tsc` 时钟源，并设置为默认时钟源。
>
> 接下来，诡异的情况发生了。
>
> NEMU 的性能得到了极大提升。经过 am-tests clock 测试时钟无误后，启动 benchmark，结果如下
> 
> ```
> Running CoreMark for 1000 iterations
> 2K performance run parameters for coremark.
> CoreMark Size    : 666
> Total time (ms)  : 1
> Iterations       : 1000
> Compiler version : GCC11.4.0
> seedcrc          : 0xe9f5
> [0]crclist       : 0xe714
> [0]crcmatrix     : 0x1fd7
> [0]crcstate      : 0x8e3a
> [0]crcfinal      : 0xd340
> Finised in 1 ms.
> ==================================================
> CoreMark PASS       2921400 Marks
>                 vs. 100000 Marks (i7-7700K @ 4.20GHz)
> [src/cpu/cpu-exec.c:164 cpu_exec] nemu: HIT GOOD TRAP at pc = 0x8000244c
> [src/cpu/cpu-exec.c:132 statistic] host time spent = 12,167,461 us
> [src/cpu/cpu-exec.c:133 statistic] total guest instructions = 305,931,745
> [src/cpu/cpu-exec.c:134 statistic] simulation frequency = 25,143,433 inst/s
> 
> 
> Dhrystone Benchmark, Version C, Version 2.2
> Trying 500000 runs through Dhrystone.
> Finished in 2 ms
> ==================================================
> Dhrystone PASS         440450 Marks
>                    vs. 100000 Marks (i7-7700K @ 4.20GHz)
> [src/cpu/cpu-exec.c:164 cpu_exec] nemu: HIT GOOD TRAP at pc = 0x80000ee0
> [src/cpu/cpu-exec.c:132 statistic] host time spent = 9,976,722 us
> [src/cpu/cpu-exec.c:133 statistic] total guest instructions = 226,007,598
> [src/cpu/cpu-exec.c:134 statistic] simulation frequency = 22,653,492 inst/s
> 
>
> Empty mainargs. Use "ref" by default
> ======= Running MicroBench [input *ref*] =======
> [qsort] Quick sort: * Passed.
>   min time: 201.910 ms [2181]
> [queen] Queen placement: * Passed.
>   min time: 112.669 ms [3611]
> [bf] Brainf**k interpreter: * Passed.
>   min time: 1.524 ms [1103346]
> [fib] Fibonacci number: * Passed.
>   min time: 3.016 ms [668700]
> [sieve] Eratosthenes sieve: * Passed.
>   min time: 63.036 ms [55243]
> [15pz] A* 15-puzzle search: * Passed.
>   min time: 0.181 ms [2961325]
> [dinic] Dinic's maxflow algorithm: * Passed.
>   min time: 41.095 ms [19909]
> [lzip] Lzip compression: * Passed.
>   min time: 715.859 ms [949]
> [ssort] Suffix sort: * Passed.
>   min time: 233.391 ms [1714]
> [md5] MD5 digest: * Passed.
>   min time: 6129.492 ms [247]
> ==================================================
> MicroBench PASS        481722 Marks
>                    vs. 100000 Marks (i9-9900K @ 3.60GHz)
> Scored time: 7502.173 ms
> Total  time: 57229.434 ms
> [src/cpu/cpu-exec.c:164 cpu_exec] nemu: HIT GOOD TRAP at pc = 0x80004e24
> [src/cpu/cpu-exec.c:132 statistic] host time spent = 57,230,136 us
> [src/cpu/cpu-exec.c:133 statistic] total guest instructions = 1,866,990,200
> [src/cpu/cpu-exec.c:134 statistic] simulation frequency = 32,622,501 inst/s
> ```
>
> NEMU 的跑分达到了 481722 分？！
>
> 接下来，我启动了 am-tests video，帧率达到了 15 fps，远高于原来的 2 fps。启动了 litenes mario，但是性能反而有所降低，大概变为 9 fps 左右。启动 typing-game，流畅度大幅提升，原来和 native 相比不稳定的画面变得十分清晰流畅。
> 
> 我猜测，这可能和缓存有很大关系，typing-game，am-tests video 的核心代码始终在一个小范围内循环，因此空间局部性好。这两个小程序的条件判断分支较少，因此分支预测准确率高。而 litenes mario 程序较大（44 kB），条件判断复杂，分支预测准确率低，所以运行缓慢。
>
> 这个问题暂时没有办法解释，暂时搁置。

### 实现 dtrace

已实现，Kconfig 中打开 `DTRACE` 选项就可启用。

### 程序如何运行

具体地, 当你按下一个字母并命中的时候, 整个计算机系统(NEMU, ISA, AM, 运行时环境, 程序) 是如何协同工作, 从而让打字小游戏实现出"命中"的游戏效果?

> 按键部分：
> 
> 1. 首先是 NEMU 准备按键数据的过程，当按下一个按键，SDL 将会将这个事件加入事件队列；
> 2. 在 NEMU 的 `nemu/src/device/device.c` 中，轮询键盘事件，获得当前按键，然后通过 `send_key() -> key_enqueue()` 加入 NEMU 的按键事件队列；
> 3. 然后，如果 AM 程序读取 `AM_INPUT_KEYBRD` 寄存器，NEMU 将通过 mmio 方式，最后调用 `i8042_data_io_handler` 从按键队列中取事件放入寄存器。
> 4. 在打字游戏中，`main` 中每次循环中有一个读取按键的环节，如果按下按键，将会调用 `check_hit()`；
> 5. 每一个字符有一个速度属性，`check_hit()` 找到命中的字符后，将这个字符的速度改为负数，使得字符向上运动，并且增加命中数。
> 
> 图形部分：
> 
> 1. 首先，在打字游戏中，`render()` 负责渲染字符。如果一个字符的速度是负数（向上，说明命中了），就将它画为绿色，实现命中效果：`int col = (c->v > 0) ? WHITE : (c->v < 0 ? GREEN : RED);`；
> 2. 绘制每一个字符通过写入 `AM_GPU_FBDRAW` 实现，最后一次传入 `sync = true` 更新画面；
> 3. 然后，在 AM 中，对 `AM_GPU_FBDRAW` 的写入将转发到 `abstract-machine/am/src/platform/nemu/ioe/gpu.c:__am_gpu_fbdraw()`，由这个函数写入 NEMU 提供的 Framebuffer。如果 `sync == true`，则对 NEMU 的 `GPU SYNC` 端口（地址为 `VGACTL_ADDR + 4`）写入 1，更新画面；
> 4. 最后，在 NEMU 中，如果 `GPU SYNC` 端口被写入，这个操作会被转发给 `nemu/src/device/vga.c:vga_update_screen()` 将 Framebuffer 内容拷贝给 SDL。

### 编译与链接：`inline` 和 `static` 的作用

注：这道题的证明方法中开启了编译选项为 `-O0 -ggdb`。

Linus 曾说过：“`static inline` 意为我们需要的这个函数如果不被内联，就在当前的编译单元中生成一个 `static` 版本的函数。`extern inline` 意为我们实际上（在其他目标文件中）有一个这个函数的定义，（在库的实现文件中有一个非内联版本的相同的函数定义，人工保证这两个定义是相同的），而你（编译器）看到的接下来这个定义是内联版本的相同函数。”

如果删除 `static` 结果是没有报错。原因是 `ifetch.h` 中唯一定义的函数 `inst_fetch` 被内联。证明方法是用 `readelf` 查看任意包含了 `ifetch.h` 的 `.c` 文件对应的目标文件，如 `inst.o`

```bash
$ riscv64-linux-gnu-readelf -s inst.o
Symbol table '.symtab' contains 165 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS inst.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT   72 .text
     3: 0000000000000000   337 FUNC    LOCAL  DEFAULT   72 decode_operand
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT   76 .rodata
     5: 0000000000000151  4447 FUNC    LOCAL  DEFAULT   72 decode_exec
     6: 0000000000000000     0 SECTION LOCAL  DEFAULT   78 .debug_info
     7: 0000000000000000     0 SECTION LOCAL  DEFAULT   80 .debug_abbrev
     8: 0000000000000000     0 SECTION LOCAL  DEFAULT   81 .debug_loclists
     9: 0000000000000000     0 SECTION LOCAL  DEFAULT   85 .debug_rnglists
    10: 0000000000000000     0 SECTION LOCAL  DEFAULT   86 .debug_macro
    11: 0000000000000000     0 SECTION LOCAL  DEFAULT  230 .debug_line
    12: 0000000000000000     0 SECTION LOCAL  DEFAULT  232 .debug_str
    13: 0000000000000000     0 SECTION LOCAL  DEFAULT  233 .debug_line_str
    14: 0000000000000000     0 SECTION LOCAL  DEFAULT   88 .debug_macro
......
    85: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    1 wm4.0.425be0b7a2[...]
    86: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    2 wm4.stdcpredef.h[...]
......
   156: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND cpu
   157: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND vaddr_read
   158: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND vaddr_write
   159: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND set_nemu_state
   160: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND isa_raise_intr
   161: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND invalid_inst
   162: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND __stack_chk_fail
   163: 00000000000012b0    38 FUNC    GLOBAL DEFAULT   72 isa_exec_once
   164: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND vaddr_ifetch
```

没有出现这个函数定义，则说明函数被内联。

如果删除 `inline` 仍然不会报错，因为每一个编译单元中都定义了一个相同的 `inst_fetch` 函数。

同样地



## 选做题

### 立即数背后的故事

#### 假设我们需要将 NEMU 运行在 Motorola 68k 的机器上(把 NEMU 的源代码编译成 Motorola 68k 的机器码)

> 当需要把 NEMU 移植到 Motorola 68k 处理器的机器上时，需要在运行为小端 ISA 编译的程序时转换端序，包括指令字节顺序的转换和数据字节顺序的转换。因为为小端ISA 编译的程序目标文件中的指令和数据是按小端存放的，加载到内存时需要改变字节序。可以添加条件编译，使得 paddr_read 函数按需在读取时转换字节序，paddr_write 函数在写入时也改变字节序；

#### 假设我们需要把 Motorola 68k 作为一个新的 ISA 加入到 NEMU 中

> 当需要把 Motorola 68k 作为新的 ISA，且 NEMU 在小端机器上运行时，也需要转换。原因相同。解决方法也相同；

#### mips32 和 riscv32 应该如何解决把 C 代码中的 32 位常数直接编码到一条指令中的问题

> 寄存器的长度是足够的。寄存器的编号也是可以直接在指令中表示的。可以使用多条指令在寄存器中拼出一个完整的立即数。
>
> 具体地，在 riscv32 中，有 `lui`指令，可以加载高 20 位的立即数，而在 I, S. B, 指令中，可以存放 12 位立即数，这样就可以拼出 32 位立即数。比如，先 `lui`，然后用 `addi` 指令，加上低 12 位（有一些和立即数符号扩展相关的细节需要处理）。这两步被抽象成伪指令 `li`。如果想要生成一个 32 位的地址，也有 `auipc` 等指令可以使用。

### 为什么执行了未实现指令会出现上述报错信息

> 如果有没有实现的指令，那么指令执行阶段在 `decode_exec` 中会匹配到 `inv` 指令。这一条指令对应的执行代码最终调用了 `nemu/src/engine/interpreter/hostcall.c:invalid_inst`，然后这个函数会输出这个报错信息并设置 NEMU 状态为 `ABORT`，退出。

### mips32 的分支延迟槽

如果你是编译器开发者, 你将会如何寻找合适的指令放到延迟槽中呢?

> 从分支指令的前面取一条不会影响分支条件的非跳转指令，或者从后面取分支的两种情况都会执行的指令执行。具体实现暂时不会。我猜测，这一部分的优化可能需要把这一部分的程序的状态机以图的数据结构存储，每一条指令为一个节点，然后根据汇编语言的语义的依赖关系，拓扑排序。然后找到分支指令所在的节点，其前驱不得放入延迟槽，其后继两支中相同节点可以放入延迟槽。

### 指令名对照

你有办法在手册中找到对应的伪指令吗? 如果有的话, 为什么这个办法是有效的呢?

> 有办法。对于 RISCV 中文手册，其中已经列出了伪指令。对于英文手册，先人工提取指令的 `opcode`，然后在手册中搜索。在 Ch. 24 RV32/64G Instruction Set Listings 中可以找到实际的指令。

### 这又能怎么样呢 / 为什么要有AM? (建议二周目思考)

> 在操作系统和裸机中间再加入一个抽象层，便于运行在没有操作系统的裸机上的程序的开发，也方便了操作系统的开发。

> 对这个问题，当前，我的想法（猜测）是：PA 支持多种体系结构，有些体系结构自带 “VME”，“MPE” 等等，但是有些体系结构没有，所以需要软件实现。AM 屏蔽了这种差异，如果有硬件实现，用硬件实现，否则软件实现，提供一致的接口。

> #TODO: Check understanding after OSLab.

### 通过批处理模式运行 NEMU

> 在 `abstract-machine/scripts/platform/nemu.mk:14` 位置的 `NEMUFLAGS` 变量后增加 `-b`。启动批处理模式。

### 消失的符号

> 宏 `NR_DATA` 在预编译阶段已经被字符串替换了，自然不会出现在符号表中。`add` 函数中的局部变量 `c` 和形参 `a`，`b` 具有自动生存期，在运行时被动态分配在栈中，不会被保存在可执行文件中（为它们分配空间的操作由指令来完成）。
>
> 只有需要在二进制文件加载到内存中时占据内存空间的程序实体才算是一个符号，比如函数，全局变量，函数中的静态变量等。

### 寻找"Hello World!"

> 这是个伪命题。"Hello, world!" 不在字符串表中，而在 ".rodata" 节中。在 C 语言中，除了字符串字面量，其他字面量都是右值，而字符串字面量是左值，类型为 `const char[]`。编译器将只读数据放入 `.rodata` 只读数据区，可以保证即使程序用指针操作尝试以违反 C 语言语义的“黑魔法”修改这一变量，仍然不能成功（Segmentation fault）。

### 冗余的符号表

> 正常运行，链接报错。原因：所谓符号表，是一张列出了二进制文件中所有程序实体的表，其中的字符串（存在另外的字符串表中）是程序实体的名字。程序运行时并不需要知道有哪些程序实体（跳转指令指示了函数实体的位置，变量实体等的位置在各类指令的立即数等位置给出）。链接时需要知道程序实体及其名字，对于某一个目标文件，这样才能得知引用的外部符号的地址。

### 如何生成 native 的可执行文件
#TODO
### 奇怪的错误码
#TODO
### 这是如何实现的?
#TODO
### 测试 klib
#TODO

### 理解volatile关键字

> `volatile` 关键字的语义是这个变量可能因为某种原因被外部改变，每次读都重新从内存读取，每次写都写回内存。禁用所有相关的编译优化。编译器优化基于一些假设，比如没有改变过的变量值不会改变等。比如用 C 写一个使用内存映射 IO 的设备驱动程序，进行不断读取端口的操作，而没有使用 `volatile` 关键字，编译器进行了优化，比如上述程序，那么编译器可能会为这个变量分配一个寄存器，造成缓存不一致问题。甚至，编译器直接将后续重复的读取省略，如示例程序。

### 理解 mainargs

> 对于 `riscv32-nemu`，`mainargs` 在编译过程中是直接作为一个字符串，包含在 ELF 文件中的 `.srodata.mainargs` 节中的。过程如下：
> 1. 运行 `make ARCH=riscv32-nemu ALL=hello mainargs=I-love-PA` 时，在 `make` 的变量中增加了 `mainargs=I-love-PA`
> 2. 在 `abstract-machine/scripts/platform/nemu.mk` 中，定义了 `CFLAGS += -DMAINARGS=\"$(mainargs)\"`
> 3. 在 `abstract-machine/am/src/platform/nemu/trm.c` 中，定义了 `static const char mainargs[] = MAINARGS;`
> 
> 对于 `native`，`mainargs` 已经在 `make` 的变量中，也在 `make` 运行的程序的环境变量中。于是，在 `abstract-machine/am/src/native/platform.c` 中，直接通过获取环境变量的方式获取 `mainargs`：`const char *args = getenv("mainargs");`。

### 如何检测多个键同时被按下?

> 根据数电知识，键盘码中对于每一个按键的键码，都有对应的断码。当键盘发送按键的键码时，说明按键被按下。当收到断码时，说明一个按键被放开。可以设置一个 `bool keys[104]`，收到键码时设置对应键为 1，收到断码时设置对应键为 0。
