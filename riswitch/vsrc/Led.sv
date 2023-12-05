module Led (
  input clock,
  input reset,
  input sel,
  input we,
  input [31:0] din,
  output reg [15:0] led
);

always @(posedge clock) begin
  if (reset) begin
    led <= 0;
  end else if (sel & we) begin
    led <= din[15:0];
  end
end
  
endmodule
