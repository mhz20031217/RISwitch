module top (
  input CLK_INPUT,
  input [15:0] SW,
  input [4:0] BTN,
  output [15:0] LED,
  `ifdef NVDL
    output [7:0] SEG0, SEG1, SEG2, SEG3, SEG4, SEG5, SEG6, SEG7,
  `elsif VIVADO
    output [7:0] AN,
    output CA, CB, CC, CD, CE, CF, CG, DP,
  `endif
  input PS2_CLK, PS2_DAT,
  output [3:0] VGA_R, VGA_G, VGA_B,
  `ifdef NVDL
  output VGA_VALID_N,
  `endif
  output VGA_HS, VGA_VS

);

/********************
*   CLK (10MHz)    *
********************/

wire CLK; // RT: 10MHz, PERF: maxinum frequency provided by platform

`ifdef CLK_RT
`ifdef NVDL
assign CLK = CLK_INPUT;
`elsif VIVADO
clkgen #(100000000, 10000000) clk_10khz_gen(.in(CLK_INPUT), .out(CLK));
`endif
`elsif CLK_PERF
assign CLK = CLK_INPUT;
`endif

/***********************************
*   SEG_CONTENT, SEG_DP, SEG_EN    *
***********************************/

wire [31:0] SEG_CONTENT;
wire [7:0] SEG_DP, SEG_EN;

wire [3:0] SEG_CONTENT_BUF [0:7];

genvar i;
generate
for (i = 0; i < 8; i = i + 1) begin
    assign SEG_CONTENT_BUF[i] = SEG_CONTENT[4*i+3:4*i];
end
endgenerate

/*
    +--0--+
    5     1
    +--6--+
    4     2
    +--3--+ 7
*/

`ifdef NVDL

wire [7:0] SEG_DISPLAY_BUF [0:7];

assign SEG0 = ~SEG_DISPLAY_BUF[0];
assign SEG1 = ~SEG_DISPLAY_BUF[1];
assign SEG2 = ~SEG_DISPLAY_BUF[2];
assign SEG3 = ~SEG_DISPLAY_BUF[3];
assign SEG4 = ~SEG_DISPLAY_BUF[4];
assign SEG5 = ~SEG_DISPLAY_BUF[5];
assign SEG6 = ~SEG_DISPLAY_BUF[6];
assign SEG7 = ~SEG_DISPLAY_BUF[7];

generate
for (i = 0; i < 8; i = i + 1) begin
    assign SEG_DISPLAY_BUF[i][7] = SEG_EN[i] & SEG_DP[i];
    led7seg bin_to_raw(
        .in(SEG_CONTENT_BUF[i]), 
        .out(SEG_DISPLAY_BUF[i][6:0]), 
        .en(SEG_EN[i])
    );
end
endgenerate

`elsif VIVADO

wire [7:0] SEG_DISPLAY_BUF;

reg [2:0] select;
assign AN = ~(8'b1 << select);

initial begin
    select = 0;
end


wire clk_1khz;
clkgen #(100000000, 1000) clk_1khz_gen(.in(CLK_INPUT), .out(clk_1khz));

always @(posedge clk_1khz) begin
    select <= select + 1;
end

led7seg bin_to_raw(
    .in(SEG_CONTENT_BUF[select]),
    .out(SEG_DISPLAY_BUF[6:0]),
    .en(SEG_EN[select])
);

assign SEG_DISPLAY_BUF[7] = SEG_DP[select];

assign {DP, CG, CF, CE, CD, CC, CB, CA}
    = ~SEG_DISPLAY_BUF;

`endif

/********************************************
* VGA_VALID, VGA_DATA, VGA_HADDR, VGA_VADDR *
********************************************/


wire [11:0] VGA_DATA;
wire [9:0] VGA_HADDR, VGA_VADDR;

vga_ctrl vga_ctrl(
  .pclk(CLK),
  .reset(1'b0),
  .vga_data(VGA_DATA),
  .h_addr(VGA_HADDR),
  .v_addr(VGA_VADDR),
  .hsync(VGA_HS),
  .vsync(VGA_VS),
  .valid(VGA_VALID_N),
  .vga_r(VGA_R),
  .vga_g(VGA_G),
  .vga_b(VGA_B)
);

`ifndef NVDL
wire VGA_VALID_N;
`endif

/* USERSPACE BEGIN */

wire [31:0] imemaddr, dmemaddr;
wire [31:0] imemdataout, dmemdatain, dmemdataout;
wire [2:0] dmemop;
wire imemclk, dmemrdclk, dmemwrclk;
wire dmemwe;
wire halt, trap;

Cpu cpu(
  .clock(BTN[4]), 
  .reset(BTN[3]),
  .imemaddr(imemaddr),
  .imemdataout(imemdataout),
  .imemclk(imemclk),
  .dmemaddr(dmemaddr),
  .dmemdataout(dmemdataout),
  .dmemdatain(dmemdatain),
  .dmemrdclk(dmemrdclk),
  .dmemwrclk(dmemwrclk),
  .dmemop(dmemop),
  .dmemwe(dmemwe),
  .dbgdata(SEG_CONTENT[31:0]),
  .halt(halt),
  .trap(trap)
);

reg state;

initial begin
  state = 0;
end

always @(posedge CLK) begin
  if (halt & trap) begin
    state <= 1;
  end else begin
    state <= state;
  end
end

assign LED[0] = state;

InstrMem imem(
  .address(imemaddr[17:2]),
  .clock(imemclk),
  .q(imemdataout),
  .data(32'b0),
  .wren(1'b0)
);

DataMem dmem(
  .addr(dmemaddr),
  .dataout(dmemdataout),
  .datain(dmemdatain),
  .rdclk(dmemrdclk),
  .wrclk(dmemwrclk),
  .memop(dmemop),
  .we(dmemwe)
);

assign SEG_EN = 8'b11111111;


/* USERSPACE END */

endmodule

