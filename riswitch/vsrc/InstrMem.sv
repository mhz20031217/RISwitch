`include "../include/config.sv"

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

`ifdef NVDL
reg [instrWidth-1:0] instrBuf;

always @(posedge clock) begin
  imem_read(addr, instrBuf);
end

assign instr = instrBuf;

`elsif VIVADO

reg [instrWidth-1:0] mem [depth-1:0];

always @(posedge clock) begin
  instrBuf <= mem[addr[17:2]];
end

initial begin
  $readmemh(`IMEM_IMG, mem);
end

`endif

endmodule
/* verilator lint_on UNUSEDPARAM */ 
