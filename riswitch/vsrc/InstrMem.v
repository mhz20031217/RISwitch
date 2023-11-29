module InstrMem(
	address,
	clock,
	data,
	wren,
	q);

	input	[15:0]  address;
	input	  clock;
	input	[31:0]  data;
	input	  wren;
	output reg	[31:0]  q;
	
	(* ram_style = "distributed" *) reg [31:0] ram [65535:0];
	always@(posedge clock)
		q <= ram[address];
		
initial begin
  $readmemh("../tests/cpu_pipebatch/rv32ui-p-addi.hex", ram);
end
		

endmodule
