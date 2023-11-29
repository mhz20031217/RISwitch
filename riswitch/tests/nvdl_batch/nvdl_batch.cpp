#include "VCpu.h"
#include "VCpu___024root.h"
#include "VCpu__Syms.h"
#include <iomanip>
#include <verilated.h>
#include <common.hpp>
#include <memory.hpp>
#include <unistd.h>
#include <verilated_vcd_c.h>

VCpu *dut;
VerilatedVcdC *tracer;

const uint64_t max_sim_time = 100000;
uint64_t sim_time;
uint32_t clk_cnt;

static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t ns = now.tv_sec * 1000000000 + now.tv_nsec;
  return ns;
}

static uint64_t get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

static void nvdl_init(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);
  tracer = new VerilatedVcdC;
  dut = new VCpu;

  dut->trace(tracer, 10);
  tracer->open("Cpu.vcd");

  dut->clock = 0; // starting on rising edge
  sim_time = 0;
  clk_cnt = 0;

  // load memory
  imem_load("../tests/cpu_pipebatch/rv32ui-p-addi.hex");
  dmem_load("../tests/cpu_pipebatch/rv32ui-p-addi_d.hex");
  
  dut->eval();
  tracer->dump(sim_time);
}

static void nvdl_destroy() {
  delete dut;
}

static void nvdl_loop_begin() {
#ifdef CLK_RT
  if (clk_cnt == 0) {
    sim_time = get_time();
    // printf("Start: %lu\n", sim_time);
  }
#endif
  // read InstrMem and DataMem on rising edge
  paddr_t iaddr = dut->imemaddr;
  paddr_t daddr = dut->dmemaddr;
  vluint8_t dop = dut->dmemop;
  dut->clock = 1;
  dut->eval();
  dut->imemdataout = imem_read(iaddr);
  dut->dmemdataout = dmem_read(daddr, dop);
  sim_time += 1;
  tracer->dump(sim_time);
}

static void nvdl_loop_end() {
  // Writing DataMem on falling edge
  paddr_t daddr = dut->dmemaddr;
  vluint8_t dop = dut->dmemop;
  word_t din = dut->dmemdatain;
  vluint8_t dwe = dut->dmemwe;
  dut->clock = 0;
  dut->eval();
  if (dwe) {
    dmem_write(daddr, dop, din);
  }
#ifdef CLK_RT
  // ensure that 100000 cycles per 10ms (10^8Hz = 10MHz)
  // uncomment the printf statement to check whether the simulation is running fast enough
  uint64_t now;
  if (clk_cnt == 99999) {
    while ((now = get_time()) < sim_time + 10000000);
    // printf("End  : %lu\n", now);
    clk_cnt = 0;
  } else {
    clk_cnt += 1;
  }
#endif
  sim_time += 1;
  tracer->dump(sim_time);
}

void check_status() {
  if (!dut->halt) {
    return;
  }
  for (int i = 0; i < 5; i ++) {
    nvdl_loop_begin();
    nvdl_loop_end();
  } 
  
  if (dut->trap) {
    std::cout << "Hit GOOD trap.\n"; 
    exit(EXIT_SUCCESS);
  } else {
    std::cout << "Hit BAD trap.\n";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[], char *envp[]) {
  nvdl_init(argc, argv);

  while (true) {
    // std::cerr << "Sim time: " << sim_time << '\n';
    if (sim_time < 9) {
      dut->reset = 1;
    } else {
      dut->reset = 0;
    }
    nvdl_loop_begin();
    nvdl_loop_end();
    check_status();
  }

  nvdl_destroy();
  return 0;
}

