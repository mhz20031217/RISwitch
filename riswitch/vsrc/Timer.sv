`timescale 10ns/1ns
`ifdef NVDL
import "DPI-C" function void timer_read(input int is_high, output int data);
`endif

module Timer (
  `ifdef VIVADO
  input CLK_1MHz,
  `endif
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
  end
end
`elsif VIVADO
reg [63:0] uptime;

always @(posedge CLK_1MHz) begin
  if (reset) begin
    uptime <= 64'b0;
  end else begin
    uptime <= uptime + 1;
  end
end

always @(posedge clock) begin
  if (sel) begin
    if (addr[2]) begin
      dout <= uptime[63:32];
    end else begin
      dout <= uptime[31:0];
    end
  end
end
`endif
endmodule
