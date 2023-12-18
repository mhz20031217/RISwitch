package cpipe

import chisel3._
import chisel3.util._
import chisel3.util.experimental._
import chisel3.experimental._

class Cpu(w: Int) extends Module {
  val io = IO(new Bundle {
    val imemaddr    = Output(UInt(w.W))
    val imemdataout = Input(UInt(w.W))
    val imemclk     = Output(Clock())
    val dmemaddr    = Output(UInt(w.W))
    val dmemdataout = Input(UInt(w.W))
    val dmemdatain  = Output(UInt(w.W))
    val dmemrdclk   = Output(Clock())
    val dmemwrclk   = Output(Clock())
    val dmemop      = Output(UInt(3.W))
    val dmemwe      = Output(Bool())
    val dbgdata     = Output(UInt(w.W))
  })
  forceName(io.imemaddr, "imemaddr")
  forceName(io.imemdataout, "imemdataout")
  forceName(io.imemclk, "imemclk")
  forceName(io.dmemaddr, "dmemaddr")
  forceName(io.dmemdataout, "dmemdataout")
  forceName(io.dmemdatain, "dmemdatain")
  forceName(io.dmemrdclk, "dmemrdclk")
  forceName(io.dmemwrclk, "dmemwrclk")
  forceName(io.dbgdata, "dbgdata")
  forceName(io.dmemop, "dmemop")
  forceName(io.dmemwe, "dmemwe")

  withClock((!clock.asBool).asClock) {
    val core = Module(new Core(w))
    val c    = core.io

    io.imemaddr   := c.imem.addr
    c.imem.instr  := io.imemdataout
    io.dmemaddr   := c.dmem.addr
    c.dmem.dout   := io.dmemdataout
    io.dmemdatain := c.dmem.din
    io.dmemop     := c.dmem.memOp
    io.dmemwe     := c.dmem.memWe
    io.dbgdata    := c.pc
  }

  io.imemclk   := clock
  io.dmemrdclk := clock
  io.dmemwrclk := (!clock.asBool).asClock
}
