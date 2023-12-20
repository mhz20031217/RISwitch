`timescale 10ns / 1ns
module onboard();

reg clock;
reg [4:0] reset;

top top(
  .CLK_INPUT(clock),
  .BTN(reset)
);

initial begin
  #1 reset = 5'b11111;
  #50 reset = 5'b00000;
end

always begin
  #1 clock = 0;
  #1 clock = 1;
end

endmodule
