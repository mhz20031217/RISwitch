package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

object ForwardUnit {
  val FD_RSDATA = "b00".U(2.W)
  val FD_MEM    = "b01".U(2.W)
  val FD_WB     = "b10".U(2.W)
  val FD_BUS    = 0.U(1.W)
  val FD_EX     = 1.U(1.W)
}

class ForwardUnitIO extends Bundle {
  val ex_rd        = Input(UInt(5.W))
  val mem_rd       = Input(UInt(5.W))
  val wb_rd        = Input(UInt(5.W))
  val mem_memToReg = Input(Bool())
  val mem_regWe    = Input(Bool())
  val wb_regWe     = Input(Bool())
  val ex_rs1       = Input(UInt(5.W))
  val ex_rs2       = Input(UInt(5.W))
  val id_rs1       = Input(UInt(5.W))
  val id_rs2       = Input(UInt(5.W))
  val stallIf      = Output(Bool())
  val stallId      = Output(Bool())
  val stallEx      = Output(Bool())
  val flushEx      = Output(Bool())
  val ex_rs1_sel   = Output(UInt(2.W))
  val ex_rs2_sel   = Output(UInt(2.W))
  val id_rs1_sel   = Output(Bool())
  val id_rs2_sel   = Output(Bool())
}

class ForwardUnit extends Module {
  val io = IO(new ForwardUnitIO)
  import ForwardUnit._

  io.ex_rs1_sel := Mux(
    io.mem_rd === io.ex_rs1 && io.mem_regWe,
    FD_MEM,
    Mux(io.wb_rd === io.ex_rs1 && io.wb_regWe, FD_WB, FD_RSDATA)
  )
  io.ex_rs2_sel := Mux(
    io.mem_rd === io.ex_rs2 && io.mem_regWe,
    FD_MEM,
    Mux(io.wb_rd === io.ex_rs2 && io.wb_regWe, FD_WB, FD_RSDATA)
  )
  io.id_rs1_sel := Mux(io.ex_rs1_sel =/= FD_RSDATA && io.flushEx, FD_EX, FD_BUS)
  io.id_rs2_sel := Mux(io.ex_rs2_sel =/= FD_RSDATA && io.flushEx, FD_EX, FD_BUS)

  val load_use = (io.mem_memToReg && (io.ex_rs1 === io.mem_rd || io.ex_rs2 === io.mem_rd))
  io.stallIf := load_use
  io.stallId := load_use
  io.stallEx := load_use
  io.flushEx := load_use
}
