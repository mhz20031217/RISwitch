package cpipe

import chisel3._
import chisel3.util._
import chisel3.experimental._

class IfIdPipelineRegister(w: Int) extends Bundle {
  val pc    = UInt(w.W)
  val instr = UInt(32.W)
}

class IdExPipelineRegister(w: Int) extends Bundle {
  val pc      = UInt(32.W)
  val imm     = UInt(w.W)
  val rs1     = UInt(5.W)
  val rs2     = UInt(5.W)
  val rd      = UInt(5.W)
  val rs1Data = UInt(w.W)
  val rs2Data = UInt(w.W)
  val c       = new ContrSignals
}

class ExMemPipelineRegister(w: Int) extends Bundle {
  val aluF    = UInt(w.W)
  val rs2Data = UInt(w.W)
  val rd      = UInt(5.W)
  val c       = new ContrSignals
}

class MemWbPipelineRegister(w: Int) extends Bundle {
  val memToReg = Bool()
  val regWe    = Bool()
  val rd       = UInt(5.W)
  val aluF     = UInt(w.W)
  val memOut   = UInt(w.W)
}

class CoreIO(w: Int) extends Bundle {
  val imem = Flipped(new InstrMemIO(w))
  val dmem = Flipped(new DataMemIO(w))
  val pc   = Output(UInt(w.W))
  val halt = Output(Bool())
  val trap = Output(Bool())
}

class Core(w: Int) extends Module {
  val io          = IO(new CoreIO(w))
  val contrGen    = Module(new ContrGen)
  val immGen      = Module(new ImmGen(w))
  val alu         = Module(new Alu(w))
  val branchCond  = Module(new BranchCond)
  val forwardUnit = Module(new ForwardUnit)

  val resetVector = 0x080000000L.U(w.W)
  val fd_reg_init = Wire(new IfIdPipelineRegister(w))
  fd_reg_init.pc    := resetVector
  fd_reg_init.instr := 0.U(32.W)
  val de_reg_init = Wire(new IdExPipelineRegister(w))
  de_reg_init    := 0.U.asTypeOf(new IdExPipelineRegister(w))
  de_reg_init.pc := resetVector

  // Pipeline Register
  val fd_reg = RegInit(new IfIdPipelineRegister(w), fd_reg_init)
  val de_reg = RegInit(new IdExPipelineRegister(w), de_reg_init)
  val em_reg = RegInit(new ExMemPipelineRegister(w), 0.U.asTypeOf(new ExMemPipelineRegister(w)))
  val mw_reg = RegInit(new MemWbPipelineRegister(w), 0.U.asTypeOf(new MemWbPipelineRegister(w)))

  val halt = !em_reg.c.valid
  val trap = RegInit(0.B)

  // Instruction Fetch
  val branchTarget = Wire(UInt(w.W))
  val pc           = RegInit(resetVector)
  io.pc := pc
  when(!forwardUnit.io.stallIf) {
    pc := Mux(branchCond.io.pcSrc, branchTarget, pc + 4.U(w.W))
  }

  io.imem.addr := pc

  when(!forwardUnit.io.stallId) {
    when(branchCond.io.flushIf) {
      fd_reg.pc    := resetVector
      fd_reg.instr := 0.U(32.W)
    }.otherwise {
      fd_reg.instr := io.imem.instr
      fd_reg.pc    := pc
    }
  }

  // Instruction Decode
  val busW       = Wire(UInt(w.W))
  val id_rd      = fd_reg.instr(11, 7)
  val id_rs1     = fd_reg.instr(19, 15)
  val id_rs2     = fd_reg.instr(24, 20)
  val id_rs1Data = Wire(UInt(w.W))
  val id_rs2Data = Wire(UInt(w.W))

  // forward declaration for ex_rs1Data and ex_rs2Data
  val ex_rs1Data = Wire(UInt(w.W))
  val ex_rs2Data = Wire(UInt(w.W))

  contrGen.io.i := fd_reg.instr

  immGen.io.instr := fd_reg.instr
  immGen.io.extOp := contrGen.io.extOp

  val regfile = Wire(new RegfileIO(w))
  regfile.rw   := mw_reg.rd
  regfile.ra   := id_rs1
  regfile.rb   := id_rs2
  regfile.busW := busW
  regfile.we   := mw_reg.regWe
  withClock((!clock.asBool).asClock) {
    val regFile = Module(new Regfile(w))
    regfile <> regFile.io
  }

  id_rs1Data := Mux(forwardUnit.io.id_rs1_sel, ex_rs1Data, regfile.busA)
  id_rs2Data := Mux(forwardUnit.io.id_rs2_sel, ex_rs2Data, regfile.busB)

  import ForwardUnit._

  when(!forwardUnit.io.stallEx) {
    when(branchCond.io.flushId) {
      de_reg    := 0.U.asTypeOf(de_reg)
      de_reg.pc := resetVector
    }.otherwise {
      de_reg.pc      := fd_reg.pc
      de_reg.c       := contrGen.io.c
      de_reg.imm     := immGen.io.imm
      de_reg.rd      := id_rd
      de_reg.rs1     := id_rs1
      de_reg.rs2     := id_rs2
      de_reg.rs1Data := id_rs1Data
      de_reg.rs2Data := id_rs2Data
    }
  }.elsewhen(forwardUnit.io.stallEx) {
    when(forwardUnit.io.id_rs1_sel === FD_EX) {
      de_reg.rs1Data := id_rs1Data
    }
    when(forwardUnit.io.id_rs2_sel === FD_EX) {
      de_reg.rs2Data := id_rs2Data
    }
  }

  // Execute
  ex_rs1Data := MuxLookup(forwardUnit.io.ex_rs1_sel, de_reg.rs1Data)(
    Seq(
      FD_RSDATA -> de_reg.rs1Data,
      FD_MEM -> Mux(em_reg.rd === 0.U(5.W), 0.U(w.W), em_reg.aluF),
      FD_WB -> Mux(mw_reg.rd === 0.U(5.W), 0.U(w.W), busW)
    )
  )
  ex_rs2Data := MuxLookup(forwardUnit.io.ex_rs2_sel, de_reg.rs2Data)(
    Seq(
      FD_RSDATA -> de_reg.rs2Data,
      FD_MEM -> Mux(em_reg.rd === 0.U(5.W), 0.U(w.W), em_reg.aluF),
      FD_WB -> Mux(mw_reg.rd === 0.U(5.W), 0.U(w.W), busW)
    )
  )

  import Alu._
  val ex_c = de_reg.c
  alu.io.a := MuxLookup(ex_c.aluASrc, ex_rs1Data)(
    Seq(
      A_RS1 -> ex_rs1Data,
      A_PC -> de_reg.pc
    )
  )
  alu.io.b := MuxLookup(ex_c.aluBSrc, ex_rs2Data)(
    Seq(
      B_IMM -> de_reg.imm,
      B_RS2 -> ex_rs2Data,
      B_4 -> 4.U(w.W)
    )
  )
  alu.io.aluOp := ex_c.aluOp

  branchCond.io.branch := ex_c.branch
  branchCond.io.less   := alu.io.less
  branchCond.io.zero   := alu.io.zero

  branchTarget :=
    de_reg.imm +
      Mux(branchCond.io.branchSel, ex_rs1Data, de_reg.pc)

  when(forwardUnit.io.flushEx) {
    em_reg := 0.U.asTypeOf(em_reg)
  }.otherwise {
    em_reg.rd      := de_reg.rd
    em_reg.rs2Data := ex_rs2Data
    em_reg.c       := de_reg.c
    em_reg.aluF    := alu.io.f
  }

  // Memory
  io.dmem.addr  := em_reg.aluF
  io.dmem.din   := em_reg.rs2Data
  io.dmem.memOp := em_reg.c.memOp
  io.dmem.memWe := em_reg.c.memWe

  val fu = forwardUnit.io
  fu.ex_rd        := de_reg.rd
  fu.mem_rd       := em_reg.rd
  fu.wb_rd        := mw_reg.rd
  fu.ex_rs1       := de_reg.rs1
  fu.ex_rs2       := de_reg.rs2
  fu.id_rs1       := id_rs1
  fu.id_rs2       := id_rs2
  fu.mem_regWe    := em_reg.c.regWe
  fu.wb_regWe     := mw_reg.regWe
  fu.mem_memToReg := em_reg.c.memToReg

  mw_reg.rd       := em_reg.rd
  mw_reg.memToReg := em_reg.c.memToReg
  mw_reg.regWe    := em_reg.c.regWe
  mw_reg.aluF     := em_reg.aluF
  mw_reg.memOut   := io.dmem.dout

  // Write Back
  busW := Mux(mw_reg.memToReg, mw_reg.memOut, mw_reg.aluF)

  // halt conditiion
  io.halt := halt

  when(halt && busW === 0x0c0ffeeL.U(32.W)) {
    trap := 1.B
  }
  io.trap := trap
}
