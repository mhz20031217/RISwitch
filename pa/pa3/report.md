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

## 选做题

### 什么是操作系统? (建议二周目思考)

操作系统是位于 ISA 和用户应用程序中间的抽象层，包含加载器、文件系统、进程调度、驱动程序等部分。

### 异常号的保存

#TODO

### 对比异常处理与函数调用

异常处理保存的上下文比函数调用多了函数调用没有保存的通用寄存器和各种控制状态寄存器。原因是：异常处理的目的是
