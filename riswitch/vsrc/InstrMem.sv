`include "config.sv"

import "DPI-C" function void imem_read(input int addr, output int data);

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

reg [instrWidth-1:0] instrBuf;
always @(posedge clock) begin
  imem_read(addr, instrBuf);
end

assign instr = instrBuf;

endmodule
/* verilator lint_on UNUSEDPARAM */ 
