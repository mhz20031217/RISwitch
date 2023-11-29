package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

class RegfileIO(xlen: Int) extends Bundle {
  val ra   = Input(UInt(5.W))
  val rb   = Input(UInt(5.W))
  val rw   = Input(UInt(5.W))
  val we   = Input(Bool())
  val busA = Output(UInt(xlen.W))
  val busB = Output(UInt(xlen.W))
  val busW = Input(UInt(xlen.W))
}

class Regfile(xlen: Int) extends Module {
  val io = IO(new RegfileIO(xlen))

  val mem = Mem(32, UInt(xlen.W))

  io.busA := Mux(io.ra.orR, mem(io.ra), 0.U(xlen.W))
  io.busB := Mux(io.rb.orR, mem(io.rb), 0.U(xlen.W))

  when(io.we & io.rw.orR) {
    mem(io.rw) := io.busW
  }
}
