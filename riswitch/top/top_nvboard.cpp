#include <Vtop.h>
#include <common.hpp>
#include <cstdlib>
#include <iomanip>
#include <memory.hpp>
#include <unistd.h>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <nvboard.h>

extern void nvboard_bind_all_pins(Vtop *top);

Vtop *dut;
VerilatedVcdC *tracer;

const uint64_t max_sim_time = 1000000;
const uint64_t max_test_time = 1000000;
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

static void nvdl_destroy() {
  nvboard_quit();
  delete tracer;
  delete dut; 
}

static void nvdl_init(int argc, char **argv) {
  std::atexit(nvdl_destroy);
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);
  tracer = new VerilatedVcdC;
  dut = new Vtop;

  nvboard_bind_all_pins(dut);
  nvboard_init();

#ifdef CONFIG_TRACE
  dut->trace(tracer, 10);
  tracer->open("top.vcd");
#endif

  dut->CLK_INPUT = 0; // starting on rising edge
  sim_time = 0;
  clk_cnt = 0;
}

static void nvdl_loop_begin() {
#ifdef CLK_RT
  if (clk_cnt == 0) {
    sim_time = get_time();
    // printf("Start: %lu\n", sim_time);
  }
#endif
  dut->CLK_INPUT = 1;
  dut->eval();
  sim_time += 1;
#ifdef CONFIG_TRACE
  tracer->dump(sim_time);
#endif
}

static void nvdl_loop_end() {
  nvboard_update(); // nvboard should be updated on falling edge
  dut->CLK_INPUT = 0;
  dut->eval();
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
#ifdef CONFIG_TRACE
  tracer->dump(sim_time);
#endif
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

void run_test(const std::string name) {
  uint64_t test_start_time = sim_time;
  // std::cout << "Test '"<< name << "', starting at: " << test_start_time << '\n';
  while (true) {
    if (sim_time < 10) {
      dut->BTN = 0xff;
    } else {
      dut->BTN = 0x0;
    }
    nvdl_loop_begin();
    // printf("sim_time: %ld, pc: %x.\n", sim_time, dut->pc);
    nvdl_loop_end();
  }

  std::cout << "[" << name << "]\t\tThe cpu does not terminate!\n";
}

int main(int argc, char *argv[], char *envp[]) {
  nvdl_init(argc, argv);
  // load memory
  imem_load(IMEM_IMG);
  dmem_load(DMEM_IMG);

  while (true) {
    nvdl_loop_begin();
    nvdl_loop_end();
  }

  return 0;
}
