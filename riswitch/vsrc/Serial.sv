import "DPI-C" function void serial_write(input int ch);

module Serial (
    input clock,
    input reset,
    input sel,
    input we,
    input [31:0] din
);

always @(posedge clock) begin
  if (sel & we) begin
    serial_write(din);
  end
end

endmodule
