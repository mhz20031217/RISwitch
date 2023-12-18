`include "config.sv"

import "DPI-C" function void dmem_read(input int addr, output int data);
import "DPI-C" function void dmem_write(input int addr, input int data, input byte wmask);

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

`ifdef NVDL
localparam M_LB = 3'd0;
localparam M_LH = 3'd1;
localparam M_LW = 3'd2;
localparam M_LBU = 3'd4;
localparam M_LHU = 3'd5;
localparam M_SB = M_LB;
localparam M_SH = M_LH;
localparam M_SW = M_LW;

wire [1:0] offset = addr[1:0];

reg [dataWidth-1:0] rdBuf, validRd;
wire extendBit;

always @(posedge clkRd) begin
  dmem_read(addr, rdBuf);
end

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
always @(posedge clkWr) begin
  if (we) begin
    dmem_write(addr, wrBuf, {4'b0, wmask});
  end
end

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

`elsif VIVADO
reg [3:0] ea;
reg [31:0] tempout;
wire [31:0] tempin;
reg [31:0] ram [32767:0];
reg [7:0] bt = 8'b0;
reg [15:0] wd = 16'b0;
reg [31:0] din_r;
wire [31:0] cur;

initial begin
  $readmemh(DMEM_IMG, ram);
end

assign cur = ram[addr[31:2]];

always @(*) begin

        case(addr[1:0])
        2'b00:  begin
                    bt = cur[7:0];
                    wd = cur[15:0];
                    din_r = din;
                    case(memOp)
                        3'b000: begin ea[0] = 1; ea[1] = 0; ea[2] = 0; ea[3] = 0; end
                        3'b001: begin ea[0] = 1; ea[1] = 1; ea[2] = 0; ea[3] = 0; end
                        3'b010: begin ea[0] = 1; ea[1] = 1; ea[2] = 1; ea[3] = 1; end
                    endcase
                end
        2'b01:  begin   
                    bt = cur[15:8];
                    wd = 0;
                    din_r = din << 8;
                    case(memOp)
                        3'b000: begin ea[0] = 0; ea[1] = 1; ea[2] = 0; ea[3] = 0; end
                        3'b001: begin ea[0] = 0; ea[1] = 0; ea[2] = 0; ea[3] = 0; end
                        3'b010: begin ea[0] = 0; ea[1] = 0; ea[2] = 0; ea[3] = 0; end
                    endcase
                end
        2'b10:  begin
                    bt = cur[23:16];
                    wd = cur[31:16];
                    din_r = din << 16;
                    case(memOp)
                        3'b000: begin ea[0] = 0; ea[1] = 0; ea[2] = 1; ea[3] = 0; end
                        3'b001: begin ea[0] = 0; ea[1] = 0; ea[2] = 1; ea[3] = 1; end
                        3'b010: begin ea[0] = 0; ea[1] = 0; ea[2] = 0; ea[3] = 0; end
                    endcase
                end
        2'b11:  begin
                    bt = cur[31:24];
                    wd = 0; 
                    din_r = din << 24;
                    case(memOp)
                        3'b000: begin ea[0] = 0; ea[1] = 0; ea[2] = 0; ea[3] = 1; end
                        3'b001: begin ea[0] = 0; ea[1] = 0; ea[2] = 0; ea[3] = 0; end
                        3'b010: begin ea[0] = 0; ea[1] = 0; ea[2] = 0; ea[3] = 0; end
                    endcase
                end
        endcase
end

always @(posedge clkRd)
begin
    tempout <= cur;
    case(memOp)
        3'b000: dout <= {{24{bt[7]}}, bt};
        3'b001: dout <= {{16{wd[15]}}, wd};
        3'b010: dout <= cur;
        3'b100: dout <= {24'b0, bt};
        3'b101: dout <= {16'b0, wd};
    endcase
end

assign tempin[7:0] = (ea[0])? din_r[7:0] : tempout[7:0];
assign tempin[15:8] = (ea[1])? din_r[15:8] : tempout[15:8];
assign tempin[23:16] = (ea[2])? din_r[23:16] : tempout[23:16];
assign tempin[31:24] = (ea[3])? din_r[31:24] : tempout[31:24];

always @(posedge clkWr) begin
    if(we) begin
        ram[addr[31:2]] <= tempin;
    end
end
`endif

endmodule
/* verilator lint_on UNUSEDPARAM */
