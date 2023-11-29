package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

object BranchCond {
  // branch
  val BR_N    = 0.U(3.W)
  val BR_JAL  = 1.U(3.W)
  val BR_JALR = 2.U(3.W)
  val BR_BEQ  = 4.U(3.W)
  val BR_BNE  = 5.U(3.W)
  val BR_BLT  = 6.U(3.W)
  val BR_BGE  = 7.U(3.W)
}

class BranchCondIO extends Bundle {
  val branch    = Input(UInt(3.W))
  val less      = Input(Bool())
  val zero      = Input(Bool())
  val branchSel = Output(Bool())
  val pcSrc     = Output(Bool())
  val flushIf   = Output(Bool())
  val flushId   = Output(Bool())
}

class BranchCond extends Module {
  val io = IO(new BranchCondIO)

  import BranchCond._

  val taken =
    (io.branch === BR_JAL) ||
      (io.branch === BR_JALR) ||
      (io.branch === BR_BEQ && io.zero) ||
      (io.branch === BR_BNE && !io.zero) ||
      (io.branch === BR_BLT && io.less) ||
      (io.branch === BR_BGE && !io.less)

  io.branchSel := (io.branch === BR_JALR)
  io.pcSrc     := taken
  io.flushId   := taken
  io.flushIf   := taken
}
