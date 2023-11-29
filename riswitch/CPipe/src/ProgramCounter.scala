package csingle

import chisel3._
import chisel3.util._
import chisel3.experimental._

class Pc extends Module {
  val io = IO(new Bundle {
    val nextPc = Input(UInt(32.W))
    val pc     = Output(UInt(32.W))
  })

  val pc = RegInit(0.U(32.W))
  pc    := io.nextPc
  io.pc := pc
}
