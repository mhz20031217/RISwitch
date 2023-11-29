package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

object Alu {
  // aluASrc
  val A_X   = 0.U
  val A_RS1 = 0.U
  val A_PC  = 1.U

  // aluBSrc
  val B_X   = 0.U(2.W)
  val B_RS2 = 0.U(2.W)
  val B_IMM = 1.U(2.W)
  val B_4   = 2.U(2.W)

  // aluCtr
  val ALU_X    = "b0000".U(4.W)
  val ALU_ADD  = "b0000".U(4.W)
  val ALU_SUB  = "b1000".U(4.W)
  val ALU_SLL  = "b0001".U(4.W)
  val ALU_SLT  = "b0010".U(4.W)
  val ALU_SLTU = "b1010".U(4.W)
  val ALU_SRCB = "b0011".U(4.W)
  val ALU_XOR  = "b0100".U(4.W)
  val ALU_SRL  = "b0101".U(4.W)
  val ALU_SRA  = "b1101".U(4.W)
  val ALU_OR   = "b0110".U(4.W)
  val ALU_AND  = "b0111".U(4.W)
}

class AluIO(width: Int) extends Bundle {
  val aluOp = Input(UInt(4.W))
  val a     = Input(UInt(width.W))
  val b     = Input(UInt(width.W))
  val f     = Output(UInt(width.W))
  val zero  = Output(Bool())
  val less  = Output(Bool())
}

class AluCtr extends Module {
  val io = IO(new Bundle {
    val aluOp    = Input(UInt(4.W))
    val choice   = Output(UInt(3.W))
    val isArith  = Output(Bool())
    val isLeft   = Output(Bool())
    val isSigned = Output(Bool())
    val isSub    = Output(Bool())
  })
  val op = io.aluOp
  io.choice   := op(2, 0)
  io.isArith  := op(3)
  io.isLeft   := !op(2)
  io.isSigned := !op(3)
  io.isSub    := op(3) | op(1)
}

class Adder(w: Int) extends Module {
  val io = IO(new Bundle {
    val a   = Input(UInt(w.W))
    val b   = Input(UInt(w.W))
    val f   = Output(UInt(w.W))
    val cin = Input(Bool())
    val zf  = Output(Bool())
    val cf  = Output(Bool())
    val of  = Output(Bool())
    val sf  = Output(Bool())
  })
  val f = Wire(UInt((w + 1).W))
  f     := (io.a +& io.b +& io.cin)
  io.f  := f(w - 1, 0)
  io.cf := f(w) ^ io.cin
  io.zf := !io.f.orR
  io.sf := f(w - 1)
  val a = io.a
  val b = io.b
  io.of := (a(w - 1) === b(w - 1)) && (f(w - 1) =/= a(w - 1))
}

// Adapted from YSYX PPT: https://ysyx.oscc.cc/slides/2306/13.html#/chisel%E9%87%8D%E7%A3%85%E7%A6%8F%E5%88%A9
class BarrelShift(w: Int) extends Module {
  val io = IO(new Bundle {
    val in      = Input(UInt(w.W))
    val shamt   = Input(UInt(log2Up(w).W))
    val isLeft  = Input(Bool())
    val isArith = Input(Bool())
    val out     = Output(UInt(w.W))
  })
  val leftIn = Mux(io.isArith, io.in(w - 1), false.B) // 右移时从左边移入的位
  def layer(din: Seq[Bool], n: Int): Seq[Bool] = { // 描述第n级选择器如何排布
    val s = 1 << n // 需要移动的位数
    def shiftRight(i: Int) = if (i + s >= w) leftIn else din(i + s) // 描述右移时第i位输出
    def shiftLeft(i:  Int) = if (i < s) false.B else din(i - s) // 描述左移时第i位输出
    val sel = Cat(io.isLeft, io.shamt(n)) // 将移位方向和移位量作为选择器的选择信号
    din.zipWithIndex.map {
      case (b, i) => // 对于每一位输入b,
        VecInit(b, shiftRight(i), b, shiftLeft(i))(sel)
    } // 都从4种输入中选择一种作为输出
  }
  def barrelshift(din: Seq[Bool], k: Int): Seq[Bool] = // 描述有k级的桶形移位器如何排布
    if (k == 0) din // 若移位器只有0级, 则结果和输入相同
    // 否则实例化一个有k-1级的桶形移位器和第k-1级选择器, 并将后者的输出作为前者的输入
    else barrelshift(layer(din, k - 1), k - 1)
  io.out := Cat(barrelshift(io.in.asBools, log2Up(w)).reverse) // 实例化一个有log2(w)级的桶形移位器
}

class Alu(width: Int) extends Module {
  val io = IO(new AluIO(width))
  val a  = io.a
  val b  = io.b
  val f  = io.f

  val shamt = b(4, 0).asUInt

  val aluCtr = Module(new AluCtr)
  val c      = aluCtr.io
  c.aluOp := io.aluOp

  val adder = Module(new Adder(width))
  adder.io.a   := a
  adder.io.b   := b ^ Fill(width, c.isSub)
  adder.io.cin := c.isSub

  io.zero := adder.io.zf

  val less = Mux(c.isSigned, a.asSInt < b.asSInt, a < b)
  io.less := less

  val barrelshift = Module(new BarrelShift(width))
  val shiftRes    = barrelshift.io.out
  barrelshift.io.in      := a
  barrelshift.io.shamt   := shamt
  barrelshift.io.isArith := c.isArith
  barrelshift.io.isLeft  := c.isLeft

  f := MuxLookup(c.choice, 0.U(32.W))(
    Seq(
      0.U -> adder.io.f,
      1.U -> shiftRes,
      2.U -> less,
      3.U -> b,
      4.U -> (a ^ b),
      5.U -> shiftRes,
      6.U -> (a | b),
      7.U -> (a & b)
    )
  )
}
