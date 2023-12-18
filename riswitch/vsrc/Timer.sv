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
    timer_read({31'b0, addr[2]}, dout);
    $display("[verilog timer] read %x", dout);
  end
end
`endif
endmodule
