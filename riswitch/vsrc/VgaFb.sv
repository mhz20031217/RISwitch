`timescale 10ns/1ns

module Conventer2b (
  input [1:0] color_2b,
  output [3:0] vga_color
);

assign vga_color =
  (color_2b == 2'b00) ? 4'd0 :
  (color_2b == 2'b01) ? 4'd5 :
  (color_2b == 2'b10) ? 4'd10 :
  (color_2b == 2'b11) ? 4'd15 :
  4'b0;

endmodule

module VgaFb (
  input vga_clk,
  input clock, reset,
  input sel, we,
  input [9:0] h_addr, v_addr,
  input [31:0] addr,
  input [31:0] din,
  `ifdef NVDL
  output reg [11:0] vga_data
  `elsif VIVADO
  output [11:0] vga_data
  `endif
);

wire [18:0] addr_r = {h_addr, v_addr[8:0]};

wire [5:0] r_buf;
wire [3:0] r_4b, g_4b, b_4b;

Conventer2b vga_r(
  .color_2b(r_buf[5:4]),
  .vga_color(r_4b)
);

Conventer2b vga_g(
  .color_2b(r_buf[3:2]),
  .vga_color(g_4b)
);

Conventer2b vga_b(
  .color_2b(r_buf[1:0]),
  .vga_color(b_4b)
);

assign vga_data = {r_4b, g_4b, b_4b};

`ifdef NVDL
reg [5:0] mem [327679:0];


always @(posedge clock) begin
  if (sel & we) begin
    mem[addr[18:0]] <= din[5:0];
  end
end

assign r_buf = mem[addr_r];

`elsif VIVADO

FramebufferGenerator mem(
  .addra(addr[18:0]),
  .clka(clock),
  .dina(din[5:0]),
  .ena(sel),
  .wea(we),
  .addrb(addr_r),
  .clkb(vga_clk),
  .doutb(r_buf),
  .enb(1'b1)
);
`endif
    
endmodule
