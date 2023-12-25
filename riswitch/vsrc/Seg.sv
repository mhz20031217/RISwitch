`timescale 10ns/1ns
module Seg(
  input clock,
  input reset,
  input sel,
  input we,
  input [31:0] din,
  output reg [31:0] seg_content
);

always @(posedge clock) begin
  if (reset) begin
    seg_content <= 0;
  end else if (sel & we) begin
    seg_content <= din;
  end
end

endmodule
