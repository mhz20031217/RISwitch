# PA3 实验报告

**实验进度**: 我完成了所有的必做题和大部分选做题。

## 必做题

### 实现异常响应机制

在实现 RISC-V 的异常相应机制时，花费了不少时间，主要是不清楚“不需要关心特权级切换相关的内容”后，究竟应该怎样操作 `mstatus` 寄存器中的各种标志位。

参考资料
- [1] RISC-V Vol.2
- [2] [RISC-V 异常处理流程介绍](https://tinylab.org/riscv-irq-pipeline-introduction/)
- [3] https://www.bilibili.com/video/BV1aP411R7dM/?spm_id_from=333.788.recommend_more_video.4
- [4] https://www.bilibili.com/video/BV11M4y1t7nv/?spm_id_from=333.999.0.0

### 把按键输入抽象成文件

`/dev/events` 的设计包含了各种大坑，在后续的实验中我犯了许多错误，包括但不限于

1. 忘记删除事件末尾的 `\n`，Debug 3h；
2. 由于不会正确使用 `sprintf` 和 `sscanf` 在 NDL 中手写事件解析器（后来改回了 `sscanf`）；
3. 没有考虑键值对的顺序可能不同，在 Native 上 Crash，Debug 1h；
4. ...

### 实现 `gettimeofday`

我终于解决了 NEMU 时钟的 Bug！问题在于 `AM_TIMER_UPTIME` 寄存器为 64 位，需要分两次读取，然而 NEMU 是在读取高 32 位时更新寄存器值。我的 AM 实现是先读低 32 位，再读高 32 位。

PA2 的 OJ 没有测试时钟的实现，实际上我没有解决 NEMU 在时钟部分的问题。在 YSYX 的组会上，一位同学提到他没有修改相关代码，但是结果仍然正确。助教提到坑和寄存器有 64 位有关。我这才想到问题的原因。（2023.11.06 修复了这个问题并更新了 PA2 实验报告）

### 实现更多的fixedptc API

已为 libfixedptc 库编写测试，位于 `navy-apps/tests/fixedptc-test`

## 选做题

### 什么是操作系统? (建议二周目思考)

操作系统是位于 ISA 和用户应用程序中间的抽象层，包含加载器、文件系统、进程调度、驱动程序等部分。

### 异常号的保存

#TODO

### 对比异常处理与函数调用

异常处理保存的上下文比函数调用多了函数调用没有保存的通用寄存器和各种控制状态寄存器。原因是：异常处理的目的是

### 堆和栈在哪里?

堆和栈是程序运行时创建的，自然不会放入可执行文件里面。

在 AM 中，栈实际上就是从一个指定的内存位置 `_start_pointer` 开始的。在 `abstract-machine/scripts/linker.ld` 中，指定了初始的栈顶指针在 `.bss` 段后 4KB 对齐加上 32KB 的位置。堆区从栈区后 4KB 对齐的位置开始。

```ld
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
```

### 如何识别不同格式的可执行文件?

1. 通过 Magic Number 判断是否为 ELF 文件；
2. 通过解析 Elf_Header 判断 ELF 文件运行的平台是否正确。

### 冗余的属性? 为什么要清零?

ELF 可执行文件（程序）是内存中的进程的映像，文件中 `FileSiz` 的大小不一定和内存 `MemSiz` 中的大小相同，需要初始化的部分需要在文件中保存，而没有初始化的内存区域无需保存。比如 `.bss` 节，是 C 中未初始化的全局变量的内存区域。C 标准规定，没有初始化的全局变量内存区域初始化为 0。

### 检查ELF文件的魔数  检测ELF文件的ISA类型

已完成。

### 系统调用的必要性

如果批处理序列中的每一个程序都知道下一个加载的程序是哪一个（而且也能够将下一个程序正确加载），我认为可以不需要系统调用（不需要操作系统），也可以把 AM 的 API 暴露给批处理系统中的程序。

如果规定是由操作系统来加载程序，那么系统调用是必须的。因为想要达到批处理系统的效果，一个程序结束后必须有方法告知操作系统，只有系统调用才能实现这一点。

### RISC-V系统调用号的传递

在 RISC-V ABI 中，规定 `a0` 寄存器作为第一个参数和返回值的寄存器，如果在 `a0` 中放置系统调用号，那么第一个参数将被覆盖。我猜想是为了让系统调用参数传递和普通参数传递保持一致，将系统调用号的转递放到了 `a7`。

### 实现居中的画布

已实现。

### 比较fixedpt和float

这意味着我们假定需要表示的数、运算的中间结果不会太大也不会太小，具体地，正数在 $[2^{-8}, 2^{23})$ 范围内，而且对计算精度要求不太高。

### 神奇的 fixedpt_rconst

实际上，乘法的操作数 `FIXEDPT_ONE` 的类型是 `int32_t`。

### 如何将浮点变量转换成fixedpt类型?

因为该 `float` 真值落在了 `fixedpt` 可表示的范围中，所以不用考虑 NaN，Inf 还有非规格化浮点数。

以下为 `fixedpt_fromfloat(void *p)` 的实现。

#TODO

### 神奇的LD_PRELOAD

这一题就是解析如何实现类似 `chroot ${NAVY_HOME}/fsimg` 的功能，下面以 `fopen` 的实现为例。

正常情况下，NAVY 应用程序调用 libc 的 `fopen`，然后 C 库将发起系统调用，调用 libos 中的 `_syscall_` 发起系统调用。在 native 上，如果系统调用被执行了，那么由于传递给 linux 的文件为相对于 `${NAVY_HOME}/fsimg` 不存在，文件将不能打开。`native.cpp` 的做法是在 C 库层面上劫持对 `fopen` 的调用，在程序给出的路径前加上前缀，然后转而使用 Glibc 的实现。

在 `navy-apps/libs/libos/Makefile` 中，定义了 `native.so` 的编译规则

```Makefile
build/native.so: src/native.cpp
	mkdir -p build/
	g++ -std=c++11 -O1 -fPIC -shared -o build/native.so src/native.cpp -ldl -lSDL2
```

在 `navy-apps/scripts/native.mk` 中，定义了 native 上的链接规则，设置了环境变量 `LD_PRELOAD`

```Makefile
run: app env
	@LD_PRELOAD=$(NAVY_HOME)/libs/libos/build/native.so $(APP) $(mainargs)
```

根据 [1]，“我们可以指定预先装载的一些共享库甚至是目标文件。在 `LD_PRELOAD` 里面指定的文件会在动态链接器按照固定规则搜索共享库之前装载，它比 `LD_LIBRARY_PATH` 里面所指定的目录中的共享库还要优先。无论程序是否依赖它们，`LD_PRELOAD` 里面指定的共享库或目标文件都会被装载”

根据动态链接的规则，在运行时动态链接 `fopen` 时，链接的是 `native.cpp` 中给出的 `fopen`（`native.cpp` 中写了 `extern "C"` 防止 C++ 函数名字改变）。

#### 参考文献
- [1] 8.5 环境变量 - 《程序员的自我修养》

### 实现内建的echo命令

已实现。

### 仙剑奇侠传的框架是如何工作的?

#TODO

### 仙剑奇侠传的脚本引擎

《仙剑奇侠传》是一个状态机😇。首先，开发者开发了游戏引擎（CPU），定义了各种事件和操作（ISA），实现了所有的事件处理函数（指令），然后为游戏编写脚本（程序）。

在 `navy-apps/apps/pal/repo/include/global.h` 中，定义了指令格式

```cpp
typedef struct tagSCRIPTENTRY
{
   WORD          wOperation;     // operation code
   WORD          rgwOperand[3];  // operands
} SCRIPTENTRY, *LPSCRIPTENTRY;
```

在 `navy-apps/apps/pal/repo/src/game/script.c` 中，这个巨大的 `switch` 就是指令译码（Opcode 部分）

```cpp
   switch (pScript->wOperation)
   {
   case 0x000B:
   case 0x000C:
   ...
   }
```

### 不再神秘的秘技

我没有玩过《仙剑》，也不想玩，只是负责让它跑起来。具体的细节可能需要查看 `.mkf` 中的二进制脚本才行。仔细检查了 PAL 中所有包含 `cash` 的代码，没有发现 bug。

```cpp
   case 0x001E:
      //
      // Increase or decrease cash by the specified amount
      //
      if ((SHORT)(pScript->rgwOperand[0]) < 0 &&
         gpGlobals->dwCash < (WORD)(-(SHORT)(pScript->rgwOperand[0])))
      {
         //
         // not enough cash
         //
         wScriptEntry = pScript->rgwOperand[1] - 1;
      }
      else
      {
         gpGlobals->dwCash += (SHORT)(pScript->rgwOperand[0]);
      }
      break;
```

以下是猜测

1. 第一个和第二个，似乎都是整数溢出 Bug。都是钱所剩无几，然后变为 SHORT 负数，转换过程中变为 DWORD，然后又转换成 int，就“暴增到上限了”；
2. 第三个实在不清楚；

### 实现Navy上的AM

功能已实现。
