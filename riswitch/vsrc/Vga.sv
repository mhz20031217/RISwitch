module Vga (
    input clock, reset,
    input vga_clk,
    input sel, we,
    input [31:0] addr,
    input [31:0] din,
    output hsync, vsync, valid,
    output [3:0] vga_r, vga_g, vga_b
);

wire [9:0] h_addr, v_addr;
wire [11:0] vga_data;

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
assign c_addr = h_addr / 9;
assign ir_addr = v_addr[3:0];
assign ic_addr = h_addr % 9;

vga_cmem cmem(
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

vga_ctrl ctrl(
    .pclk(clock),
    .reset(reset),
    .vga_data(vga_data),
    .h_addr(h_addr),
    .v_addr(v_addr),
    .hsync(hsync),
    .vsync(vsync),
    .valid(valid),
    .vga_r(vga_r),
    .vga_g(vga_g),
    .vga_b(vga_b)
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
