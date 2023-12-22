`timescale 10ns/1ns
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
*   CLK             *
********************/

wire CLK_100MHz, CLK_80MHz, CLK_50MHz, CLK_25MHz, CLK_10MHz;
`ifdef NVDL
assign CLK_100MHz = CLK_INPUT;
assign CLK_80MHz = CLK_INPUT;
assign CLK_50MHz = CLK_INPUT;
assign CLK_25MHz = CLK_INPUT;
assign CLK_10MHz = CLK_INPUT;
`elsif VIVADO
ClockGenerator clkgen(
  .reset(1'b0),
  .clk_in1(CLK_INPUT),
  .clk_100mhz(CLK_100MHz),
  .clk_80mhz(CLK_80MHz),
  .clk_50mhz(CLK_50MHz),
  .clk_25mhz(CLK_25MHz),
  .clk_10mhz(CLK_10MHz),
  .locked()
);
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

always @(posedge CLK_10MHz) begin
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

/***************
*     VGA      *
***************/

`ifndef NVDL
wire VGA_VALID_N;
`endif

wire [9:0] VGA_HADDR, VGA_VADDR;
wire [11:0] VGA_DATA;
wire VGA_CLK;
`ifdef NVDL
assign VGA_CLK = CLK_INPUT;
`elsif VIVADO
assign VGA_CLK = CLK_25MHz;
`endif

vga_ctrl ctrl(
  .pclk(VGA_CLK),
  .reset(BTN[4]),
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

/* USERSPACE BEGIN */

wire clock = CLK_50MHz;
wire reset = BTN[4];

localparam addrWidth = 32;
localparam dataWidth = 32;
localparam instrWidth = 32;

wire [addrWidth-1:0] imemaddr, dmemaddr;
wire [instrWidth-1:0] imemdataout;
wire [dataWidth-1:0] dmemdatain, dmemdataout;
wire imemclk, dmemrdclk, dmemwrclk;
wire [2:0] dmemop;
wire dmemwe, dmemre;

/* verilator lint_off UNUSEDSIGNAL */
wire [31:0] dontcare;
/* verilator lint_on UNUSEDSIGNAL */

Cpu cpu(
  .clock(clock),
  .reset(reset),
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
  .dmemre(dmemre),
  .dbgdata(dontcare)
);

InstrMem instrMem(
  .clock(imemclk),
  .addr(imemaddr),
  .instr(imemdataout)
);

wire sel_dmem, sel_seg, sel_kbd, sel_timer, sel_cmem, sel_fb, sel_vgamode, sel_led, sel_serial;
wire [31:0] dout_timer, dout_sw, dout_dmem, dout_kbd;

assign dout_sw = {16'b0, SW};

Keyboard mykbd(
        .clk(clock),
        .clrn(~reset),
        .ps2_clk(PS2_CLK),
        .ps2_data(PS2_DAT),
        .en(sel_kbd & dmemre),
        .cur_key(dout_kbd)
);

Mmu mmu(
  .addr(dmemaddr),
  .dout(dmemdataout),
  .sel_dmem(sel_dmem),
  .sel_timer(sel_timer),
  .sel_seg(sel_seg),
  .sel_kbd(sel_kbd),
  .sel_cmem(sel_cmem),
  .sel_fb(sel_fb),
  .sel_led(sel_led),
  .sel_serial(sel_serial),
  .sel_vgamode(sel_vgamode),
  .dout_timer(dout_timer),
  .dout_sw(dout_sw),
  .dout_kbd(dout_kbd),
  .dout_dmem(dout_dmem)
);

DataMem dataMem(
  .addr(dmemaddr),
  .din(dmemdatain),
  .dout(dout_dmem),
  .memOp(dmemop),
  .clkRd(dmemrdclk),
  .clkWr(dmemwrclk),
  .we(dmemwe & sel_dmem)
);

Led led(
  .clock(dmemwrclk),
  .reset(reset),
  .sel(sel_led),
  .we(dmemwe),
  .din(dmemdatain),
  .led_out(LED)
);

Seg seg(
  .clock(dmemwrclk),
  .reset(reset),
  .sel(sel_seg),
  .we(dmemwe),
  .din(dmemdatain),
  .seg_content(SEG_CONTENT)
);

assign SEG_EN = 8'b11111111;
assign SEG_DP = 8'b00000000;

wire [11:0] vga_cmem_data, vga_fb_data;
reg vga_mode;
always @(posedge dmemwrclk) begin
  if (BTN[3]) begin
    vga_mode <= ~vga_mode;
  end else if (dmemwe & sel_vgamode) begin
    vga_mode <= dmemdatain[0];
  end
end
assign VGA_DATA = (vga_mode) ? vga_cmem_data : vga_fb_data;

VgaCmem vcmem(
  .vga_clk(VGA_CLK),
  .clock(dmemwrclk),
  .reset(reset),
  .sel(sel_cmem),
  .we(dmemwe),
  .din(dmemdatain),
  .addr(dmemaddr),
  .h_addr(VGA_HADDR),
  .v_addr(VGA_VADDR),
  .vga_data(vga_cmem_data)
);

VgaFb vfb(
  .vga_clk(VGA_CLK),
  .clock(dmemwrclk),
  .reset(reset),
  .sel(sel_fb),
  .we(dmemwe),
  .din(dmemdatain),
  .addr(dmemaddr),
  .h_addr(VGA_HADDR),
  .v_addr(VGA_VADDR),
  .vga_data(vga_fb_data)
);

`ifdef VIVADO
wire CLK_1MHz;
clkgen #(10000000, 1000000) clk_1mhz_gen(
  .in(CLK_10MHz),
  .out(CLK_1MHz)
);
`endif

Timer timer(
  `ifdef VIVADO
  .CLK_1MHz(CLK_1MHz),
  `endif
  .clock(dmemrdclk),
  .reset(reset),
  .sel(sel_timer & dmemre),
  .addr(dmemaddr),
  .dout(dout_timer)
);

`ifdef NVDL
Serial serial(
  .clock(dmemrdclk),
  .reset(reset),
  .sel(sel_serial),
  .we(dmemwe),
  .din(dmemdatain)
);
`endif

/* USERSPACE END */

endmodule
