`timescale 10ns/1ns
module vga_cmem (
  input clk,
  input [4:0] r_addr, // [0, 29)
  input [6:0] c_addr, // [0, 69)
  output [7:0] ascii,
  output [2:0] fg_color,
  output [2:0] bg_color,
  input we,
  input [4:0] wr_addr,
  input [6:0] wc_addr,
  input [7:0] w_ascii,
  input [2:0] w_fg_color,
  input [2:0] w_bg_color
);

(* ram_style = "block" *) reg [15:0] mem [2239:0];

wire [11:0] index, w_index;
assign index = {c_addr, r_addr};
assign w_index = {wc_addr, wr_addr};

wire [15:0] mem_inbuf, mem_outbuf;
assign mem_outbuf = mem[index];

assign {bg_color, fg_color, ascii} = mem_outbuf[13:0];
assign mem_inbuf = {2'b0, w_bg_color, w_fg_color, w_ascii};

always @(posedge clk) begin
  if (we) begin
    mem[w_index] <= mem_inbuf;
  end
end

endmodule
