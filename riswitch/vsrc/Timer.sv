`ifdef NVDL
import "DPI-C" function void timer_read(input int is_high, output int data);
`endif

module Timer (
  input clock,
  input reset,
  input sel,
  input [31:0] addr,
  output reg [31:0] dout
);

`ifdef NVDL
always @(posedge clock) begin
  if (sel) begin
    $display("[verilog timer] read");
    timer_read({31'b0, addr[2]}, dout);
  end
end
`endif
endmodule
