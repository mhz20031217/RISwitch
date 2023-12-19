#include <VSystem.h>
#include <common.hpp>
#include <cstdlib>
#include <instructions.hpp>
#include <iomanip>
#include <memory.hpp>
#include <string>
#include <unistd.h>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <getopt.h>

static const char *imem_img = IMEM_IMG;
static const char *dmem_img = DMEM_IMG;

VSystem *dut;
VerilatedVcdC *tracer;

const uint64_t max_sim_time = 100000;
const uint64_t max_test_time = 100000;
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
  if (boot_time == 0)
    boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

static void nvdl_init(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);
  tracer = new VerilatedVcdC;
  dut = new VSystem;

  dut->trace(tracer, 10);
  tracer->open("System.vcd");

  dut->clock = 0; // starting on rising edge
  sim_time = 0;
  clk_cnt = 0;
}

static void nvdl_destroy() { 
  delete tracer;
  delete dut; 
}

static void nvdl_loop_begin() {
#ifdef CLK_RT
  if (clk_cnt == 0) {
    sim_time = get_time();
    // printf("Start: %lu\n", sim_time);
  }
#endif
  dut->clock = 1;
  dut->eval();
  sim_time += 1;
  tracer->dump(sim_time);
}

static void nvdl_loop_end() {
  // Writing DataMem on falling edge
  dut->clock = 0;
  dut->eval();
#ifdef CLK_RT
  // ensure that 100000 cycles per 10ms (10^8Hz = 10MHz)
  // uncomment the printf statement to check whether the simulation is running
  // fast enough
  uint64_t now;
  if (clk_cnt == 99999) {
    while ((now = get_time()) < sim_time + 10000000)
      ;
    // printf("End  : %lu\n", now);
    clk_cnt = 0;
  } else {
    clk_cnt += 1;
  }
#endif
  sim_time += 1;
  tracer->dump(sim_time);
}

bool check_status(const std::string &name) {
  if (!dut->halt) {
    return false;
  }
  for (int i = 0; i < 5; i++) {
    nvdl_loop_begin();
    nvdl_loop_end();
  }

  if (dut->trap) {
    std::cout << "[" << name << "]\tHit GOOD trap.\n";
    return true;
  } else {
    std::cout << "[" << name << "]\tHit BAD trap.\n";
    return true;
  }
}

void run_test() {
  uint64_t test_start_time = sim_time;
  // load memory
  printf("Loading img:\n%s\n%s\n", imem_img, dmem_img);
  imem_load(imem_img);
  dmem_load(dmem_img);

  // std::cout << "Test '"<< name << "', starting at: " << test_start_time << '\n';

  while (sim_time - test_start_time < max_test_time &&
         sim_time < max_sim_time) {
    // std::cerr << "Sim time: " << sim_time << '\n';
    if (sim_time - test_start_time < 9) {
      dut->reset = 1;
    } else {
      dut->reset = 0;
    }
    nvdl_loop_begin();
    // printf("sim_time: %ld, pc: %x.\n", sim_time, dut->pc);
    nvdl_loop_end();
    if (check_status(imem_img)) {
      return;
    }
  }

  std::cout << "[" << imem_img << "]\t\tThe cpu does not terminate!\n";
}

int main(int argc, char *argv[], char *envp[]) {
  nvdl_init(argc, argv);
  std::atexit(nvdl_destroy);

  run_test();

  return 0;
}
