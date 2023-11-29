package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

object ImmGen {
  // extOp
  val EOP_X = 0.U(3.W)
  val EOP_I = 0.U(3.W)
  val EOP_U = 1.U(3.W)
  val EOP_S = 2.U(3.W)
  val EOP_B = 3.U(3.W)
  val EOP_J = 4.U(3.W)
}

class ImmGenIO(xlen: Int) extends Bundle {
  val instr = Input(UInt(xlen.W))
  val extOp = Input(UInt(3.W))
  val imm   = Output(UInt(xlen.W))
}

class ImmGen(xlen: Int) extends Module {
  import ImmGen._
  val io = IO(new ImmGenIO(xlen))
  val i  = io.instr

  val imm = Wire(UInt(32.W))
  imm := MuxLookup(io.extOp, 0.U(xlen.W))(
    Seq(
      EOP_I -> Cat(Fill(20, i(31)), i(31, 20)),
      EOP_U -> Cat(i(31, 12), 0.U(12.W)),
      EOP_S -> Cat(Fill(20, i(31)), i(31, 25), i(11, 7)),
      EOP_B -> Cat(Fill(20, i(31)), i(7), i(30, 25), i(11, 8), 0.U(1.W)),
      EOP_J -> Cat(Fill(12, i(31)), i(19, 12), i(20), i(30, 21), 0.U(1.W))
    )
  )
  io.imm := imm.asSInt.asUInt
}
