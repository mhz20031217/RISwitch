`timescale 10ns/1ns
module VgaCmem (
    input vga_clk,
    input clock, reset,
    input sel, we,
    input [9:0] h_addr, v_addr,
    input [31:0] addr,
    input [31:0] din,
    output [11:0] vga_data
);

wire [2:0] fg_color, bg_color, w_fg_color, w_bg_color;
wire [7:0] ascii, w_ascii;
wire vga_bit;

wire [4:0] r_addr, wr_addr;
wire [6:0] c_addr, wc_addr;
wire [3:0] ir_addr;
wire [3:0] ic_addr;

assign w_ascii = din[7:0];
assign w_fg_color = din[10:8];
assign w_bg_color = din[13:11];

assign wr_addr = addr[5:1];
assign wc_addr = addr[12:6];

assign r_addr = v_addr[8:4];
assign ir_addr = v_addr[3:0];

assign c_addr = h_addr / 9;
assign ic_addr = h_addr % 9;

//wire [63:0] intermediate;
//assign intermediate = h_addr * 477218588; // 2^32 / 9
//assign c_addr = intermediate[63:32]; // intermediate / 2^32
//assign ic_addr = h_addr - (c_addr*9);

vga_cmem cmem(
    .vga_clk(vga_clk),
    .clk(clock),
    .r_addr(r_addr),
    .c_addr(c_addr),
    .ascii(ascii),
    .fg_color(fg_color),
    .bg_color(bg_color),
    .we(sel & we),
    .wr_addr(wr_addr),
    .wc_addr(wc_addr),
    .w_ascii(w_ascii),
    .w_fg_color(w_fg_color),
    .w_bg_color(w_bg_color)
);

vga_bitmap vga_bitmap(
    .ascii(ascii),
    .ir_addr(ir_addr),
    .ic_addr(ic_addr),
    .vga_bit(vga_bit)
);

vga_render render(
    .vga_bit(vga_bit),
    .fg_color(fg_color),
    .bg_color(bg_color),
    .vga_data(vga_data)
);
    
endmodule
