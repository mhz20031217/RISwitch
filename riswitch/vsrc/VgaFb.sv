`timescale 10ns/1ns
module VgaFb (
  input vga_clk,
  input clock, reset,
  input sel, we,
  input [9:0] h_addr, v_addr,
  input [31:0] addr,
  input [31:0] din,
  `ifdef NVDL
  output reg [11:0] vga_data
  `elsif VIVADO
  output [11:0] vga_data
  `endif
);

wire [18:0] addr_r = {h_addr, v_addr[8:0]};

`ifdef NVDL
reg [15:0] mem [327679:0];


always @(posedge clock) begin
  if (sel & we) begin
    mem[addr[19:1]] <= din;
  end
end

assign vga_data = mem[addr_r];

`elsif VIVADO
FramebufferGenerator mem(
  .addra(addr[19:1]),
  .clka(clock),
  .dina(din[11:0]),
  .ena(sel),
  .wea(we),
  .addrb(addr_r),
  .clkb(vga_clk),
  .doutb(vga_data),
  .enb(1'b1)
);
`endif
    
endmodule
