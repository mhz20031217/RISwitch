import chisel3.stage._

object Elaborate extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new cpipe.Cpu(32), args)
}
