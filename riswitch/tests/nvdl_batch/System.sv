module System(
  input clock,
  input reset,
  output [addrWidth-1:0] pc,
  output halt, trap
);

localparam addrWidth = 32;
localparam dataWidth = 32;
localparam instrWidth = 32;

wire [addrWidth-1:0] imemaddr, dmemaddr;
wire [instrWidth-1:0] imemdataout;
wire [dataWidth-1:0] dmemdatain, dmemdataout;
wire imemclk, dmemrdclk, dmemwrclk;
wire [2:0] dmemop;
wire dmemwe, dmemre;

Cpu cpu(
  .clock(clock),
  .reset(reset),
  .imemaddr(imemaddr), 
  .imemdataout(imemdataout), 
  .imemclk(imemclk), 
  .dmemaddr(dmemaddr), 
  .dmemdataout(dmemdataout), 
  .dmemdatain(dmemdatain), 
  .dmemrdclk(dmemrdclk), 
  .dmemwrclk(dmemwrclk), 
  .dmemop(dmemop), 
  .dmemwe(dmemwe),
  .dmemre(dmemre),
  .dbgdata(pc)
);

InstrMem instrMem(
  .clock(imemclk),
  .addr(imemaddr),
  .instr(imemdataout)
);

DataMem dataMem(
  .addr(dmemaddr),
  .din(dmemdatain),
  .dout(dmemdataout),
  .memOp(dmemop),
  .clkRd(dmemrdclk),
  .clkWr(dmemwrclk),
  .we(dmemwe)
);

endmodule
