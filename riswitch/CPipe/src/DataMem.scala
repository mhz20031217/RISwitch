package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

object DataMem {
  // memOp
  val M_X   = 0.U(3.W)
  val M_LB  = 0.U(3.W)
  val M_LH  = 1.U(3.W)
  val M_LW  = 2.U(3.W)
  val M_LBU = 4.U(3.W)
  val M_LHU = 5.U(3.W)
  val M_SB  = M_LB
  val M_SH  = M_LH
  val M_SW  = M_LW
}

class DataMemIO(w: Int) extends Bundle {
  val addr  = Input(UInt(w.W))
  val din   = Input(UInt(w.W))
  val dout  = Output(UInt(w.W))
  val memOp = Input(UInt(3.W))
  val memWe = Input(Bool())
  val memRe = Input(Bool())
}
