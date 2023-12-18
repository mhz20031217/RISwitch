module Mmu(
  input [31:0] addr,
  output [31:0] dout,

  output sel_dmem, sel_seg, sel_kbd, sel_timer, sel_cmem, sel_vga, sel_led, sel_serial,
  input [31:0] dout_dmem, dout_kbd, dout_timer, dout_sw
);

wire [3:0] id = addr[23:20];
wire sel_sw;

assign sel_dmem = (id == 4'h1);
assign sel_seg = (id == 4'h2);
assign sel_kbd = (id == 4'h5);
assign sel_timer = (id == 4'h3);
assign sel_cmem = (id == 4'h4);
assign sel_vga = (id == 4'h8);
assign sel_sw = (id == 4'h6);
assign sel_led = (id == 4'h7);
assign sel_serial = (id == 4'hf);

assign dout =
  (sel_dmem) ? dout_dmem :
  (sel_kbd) ? dout_kbd :
  (sel_timer) ? dout_timer :
  (sel_sw) ? dout_sw :
  32'b0;

endmodule
