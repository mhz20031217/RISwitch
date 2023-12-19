`ifdef NVDL
import "DPI-C" function void timer_read(input int is_high, output int data);
`endif

module Timer (
  `ifdef VIVADO
  input CLK_INPUT,
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

wire clk_1mhz;
clkgen #(100000000, 1000000) clk_1mhz_gen(
  .in(CLK_INPUT),
  .out(clk_1mhz)
);

always @(posedge clk_1mhz) begin
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
