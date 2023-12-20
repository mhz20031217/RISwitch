`timescale 10ns/1ns
module Led (
  input clock,
  input reset,
  input sel,
  input we,
  input [31:0] din,
  output reg [15:0] led_out
);

always @(posedge clock) begin
  if (reset) begin
    led_out <= 0;
  end else if (sel & we) begin
    led_out <= din[15:0];
  end else begin
    led_out <= led_out;
  end
end

endmodule
