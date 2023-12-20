`timescale 10ns/1ns
module VgaFb (
  input clock, reset,
  input sel, we,
  input [9:0] h_addr, v_addr,
  input [31:0] addr,
  input [31:0] din,
  output reg [11:0] vga_data
);

`ifdef NVDL
reg [15:0] mem [327679:0];

wire [18:0] addr_r = {h_addr, v_addr[8:0]};

always @(posedge clock) begin
  if (sel & we) begin
    mem[addr[19:1]] <= din;
  end
end

assign vga_data = mem[addr_r];

`elsif VIVADO

`endif
    
endmodule
