package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

object Control {
  val Y = true.B
  val N = false.B

  import Instructions._
  import Alu._
  import BranchCond._
  import DataMem._
  import ImmGen._

  // format: off
  //                                      memToReg memOp                valid
  //            aluASrc aluBSrc aluCtr regWe | memWe |   extOp  branch    |
  val default = // |      |       |       |  |  |    |      |      |      |
             List(A_X  , B_X  , ALU_X   , N, N, N, M_X  , EOP_X, BR_N,    N)
  val map = Array(
    LUI   -> List(A_X  , B_IMM, ALU_SRCB, Y, N, N, M_X  , EOP_U, BR_N,    Y),
    AUIPC -> List(A_PC , B_IMM, ALU_ADD , Y, N, N, M_X  , EOP_U, BR_N,    Y),
    JAL   -> List(A_PC , B_4  , ALU_ADD , Y, N, N, M_X  , EOP_J, BR_JAL,  Y),
    JALR  -> List(A_PC , B_4  , ALU_ADD , Y, N, N, M_X  , EOP_I, BR_JALR, Y),
    BEQ   -> List(A_RS1, B_RS2, ALU_SLT , N, N, N, M_X  , EOP_B, BR_BEQ,  Y),
    BNE   -> List(A_RS1, B_RS2, ALU_SLT , N, N, N, M_X  , EOP_B, BR_BNE,  Y),
    BLT   -> List(A_RS1, B_RS2, ALU_SLT , N, N, N, M_X  , EOP_B, BR_BLT,  Y),
    BGE   -> List(A_RS1, B_RS2, ALU_SLT , N, N, N, M_X  , EOP_B, BR_BGE,  Y),
    BLTU  -> List(A_RS1, B_RS2, ALU_SLTU, N, N, N, M_X  , EOP_B, BR_BLT,  Y),
    BGEU  -> List(A_RS1, B_RS2, ALU_SLTU, N, N, N, M_X  , EOP_B, BR_BGE,  Y),
    LB    -> List(A_RS1, B_IMM, ALU_ADD , Y, Y, N, M_LB , EOP_I, BR_N,    Y),
    LH    -> List(A_RS1, B_IMM, ALU_ADD , Y, Y, N, M_LH , EOP_I, BR_N,    Y),
    LW    -> List(A_RS1, B_IMM, ALU_ADD , Y, Y, N, M_LW , EOP_I, BR_N,    Y),
    LBU   -> List(A_RS1, B_IMM, ALU_ADD , Y, Y, N, M_LBU, EOP_I, BR_N,    Y),
    LHU   -> List(A_RS1, B_IMM, ALU_ADD , Y, Y, N, M_LHU, EOP_I, BR_N,    Y),
    SB    -> List(A_RS1, B_IMM, ALU_ADD , N, N, Y, M_SB , EOP_S, BR_N,    Y),
    SH    -> List(A_RS1, B_IMM, ALU_ADD , N, N, Y, M_SH , EOP_S, BR_N,    Y),
    SW    -> List(A_RS1, B_IMM, ALU_ADD , N, N, Y, M_SW , EOP_S, BR_N,    Y),
    ADDI  -> List(A_RS1, B_IMM, ALU_ADD , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    SLTI  -> List(A_RS1, B_IMM, ALU_SLT , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    SLTIU -> List(A_RS1, B_IMM, ALU_SLTU, Y, N, N, M_X  , EOP_I, BR_N,    Y),
    XORI  -> List(A_RS1, B_IMM, ALU_XOR , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    ORI   -> List(A_RS1, B_IMM, ALU_OR  , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    ANDI  -> List(A_RS1, B_IMM, ALU_AND , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    SLLI  -> List(A_RS1, B_IMM, ALU_SLL , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    SRLI  -> List(A_RS1, B_IMM, ALU_SRL , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    SRAI  -> List(A_RS1, B_IMM, ALU_SRA , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    ADD   -> List(A_RS1, B_RS2, ALU_ADD , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    SUB   -> List(A_RS1, B_RS2, ALU_SUB , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    SLT   -> List(A_RS1, B_RS2, ALU_SLT , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    SLTU  -> List(A_RS1, B_RS2, ALU_SLTU, Y, N, N, M_X  , EOP_X, BR_N,    Y),
    XOR   -> List(A_RS1, B_RS2, ALU_XOR , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    OR    -> List(A_RS1, B_RS2, ALU_OR  , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    AND   -> List(A_RS1, B_RS2, ALU_AND , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    SLL   -> List(A_RS1, B_RS2, ALU_SLL , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    SRL   -> List(A_RS1, B_RS2, ALU_SRL , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    SRA   -> List(A_RS1, B_RS2, ALU_SRA , Y, N, N, M_X  , EOP_X, BR_N,    Y),
    NOP   -> List(A_RS1, B_IMM, ALU_ADD , Y, N, N, M_X  , EOP_I, BR_N,    Y),
    ZERO  -> List(A_RS1, B_IMM, ALU_ADD , Y, N, N, M_X  , EOP_I, BR_N,    Y),
  )
  // format: on
}

class ContrSignals extends Bundle {
  val aluASrc  = Output(Bool())
  val aluBSrc  = Output(UInt(2.W))
  val aluOp    = Output(UInt(4.W))
  val regWe    = Output(Bool())
  val memToReg = Output(Bool())
  val memWe    = Output(Bool())
  val memOp    = Output(UInt(3.W))
  val branch   = Output(UInt(3.W))
  val valid    = Output(Bool())
}

class ContrGen extends Module {
  val io = IO(new Bundle {
    val i     = Input(UInt(32.W))
    val c     = Output(new ContrSignals)
    val extOp = Output(UInt(3.W))
  })
  val contr = ListLookup(io.i, Control.default, Control.map)

  io.c.aluASrc  := contr(0)
  io.c.aluBSrc  := contr(1)
  io.c.aluOp    := contr(2)
  io.c.regWe    := contr(3)
  io.c.memToReg := contr(4)
  io.c.memWe    := contr(5)
  io.c.memOp    := contr(6)
  io.extOp      := contr(7)
  io.c.branch   := contr(8)
  io.c.valid    := contr(9)
}
