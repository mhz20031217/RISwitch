`include "../include/config.sv"

`ifdef NVDL
import "DPI-C" function void dmem_read(input int addr, output int data);
import "DPI-C" function void dmem_write(input int addr, input int data, input byte wmask);
`endif

/* verilator lint_off UNUSEDPARAM */ 
module DataMem #(
  parameter addrWidth = 32,
  parameter dataWidth = 32,
  parameter depth = 131072
) (
  input [addrWidth-1:0] addr,
  input [dataWidth-1:0] din,
  output [dataWidth-1:0] dout,
  input [2:0] memOp,
  input clkRd, clkWr,
  input we
);

localparam M_LB = 3'd0;
localparam M_LH = 3'd1;
localparam M_LW = 3'd2;
localparam M_LBU = 3'd4;
localparam M_LHU = 3'd5;
localparam M_SB = M_LB;
localparam M_SH = M_LH;
localparam M_SW = M_LW;

wire [1:0] offset = addr[1:0];

`ifdef NVDL
reg [dataWidth-1:0] rdBuf;
`endif

`ifdef VIVADO
wire [dataWidth-1:0] rdBuf;
`endif

wire [dataWidth-1:0] validRd;
wire extendBit;

assign extendBit = (memOp == M_LBU || memOp == M_LHU) ? 0 : 1;
assign validRd =
  (offset == 2'd0) ? rdBuf :
  (offset == 2'd1) ? {8'b0, rdBuf[31:8]} :
  (offset == 2'd2) ? {16'b0, rdBuf[31:16]} :
  (offset == 2'd3) ? {24'b0, rdBuf[31:24]} :
  32'd0;

assign dout = 
  (memOp == M_LB || memOp == M_LBU) ? {{24{extendBit&validRd[7]}}, validRd[7:0]} :
  (memOp == M_LH || memOp == M_LHU) ? {{16{extendBit&validRd[15]}}, validRd[15:0]} :
  validRd;

wire [dataWidth-1:0] wrBuf, validWr;
wire [3:0] wmask;

assign validWr =
  (offset == 2'd0) ? din :
  (offset == 2'd1) ? {din[23:0], 8'b0} :
  (offset == 2'd2) ? {din[15:0], 16'b0} :
  (offset == 2'd3) ? {din[7:0], 24'b0} :
  32'b0;
assign wrBuf = validWr;

assign wmask =
  (memOp == M_SB) ? (4'b0001 << offset) :
  (memOp == M_SH) ? (4'b0011 << offset) :
  4'b1111;
  
`ifdef NVDL

always @(posedge clkRd) begin
  dmem_read(addr, rdBuf);
end

always @(posedge clkWr) begin
  if (we) begin
    dmem_write(addr, wrBuf, {4'b0, wmask});
  end
end
`endif

`ifdef VIVADO

DataMemGenerator dataMemInternal(
  .addra(addr[16:2]),
  .clka(clkWr),
  .dina(wrBuf),
  .ena(we),
  .wea(wmask),
  .addrb(addr[16:2]),
  .clkb(clkRd),
  .doutb(dout),
  .enb(1'b1)
);

`endif

endmodule
/* verilator lint_on UNUSEDPARAM */
