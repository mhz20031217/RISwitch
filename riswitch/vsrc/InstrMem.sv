`timescale 10ns/1ns
`include "../include/config.sv"

`ifdef NVDL
import "DPI-C" function void imem_read(input int addr, output int data);
`endif

/* verilator lint_off UNUSEDPARAM */ 
module InstrMem #(
  parameter addrWidth = 32,
  parameter instrWidth = 32,
  parameter depth = 131072
) (
  input clock,
  input [addrWidth-1:0] addr,
  output [instrWidth-1:0] instr
);

`ifdef NVDL

reg [instrWidth-1:0] instrBuf;

always @(posedge clock) begin
  imem_read(addr, instrBuf);
end

assign instr = instrBuf;

`elsif VIVADO

//InstrMemGenerator instrMemInternal(
//  .addra(addr[16:2]),
//  .clka(clock),
//  .douta(instr),
//  .ena(1'b1)
//);
  
InstrMemGeneratorDistributed instrMemInternal(
  .a(addr[16:2]),
  .clk(clock),
  .qspo(instr)
);

`endif

endmodule
/* verilator lint_on UNUSEDPARAM */ 
