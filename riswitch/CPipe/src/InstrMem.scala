package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

class InstrMemIO(w: Int) extends Bundle {
  val addr  = Input(UInt(w.W))
  val instr = Output(UInt(w.W))
}
