# RISwitch 计算机系统和配套集成开发环境

[演示](https://www.bilibili.com/video/BV1xt4y1d78A)

[演示 - NJU Box](https://box.nju.edu.cn/f/5c9b2507110844d7b7f3/)

完整报告见 [riswitch/doc/report.md](riswitch/doc/report.md)。

## 技术指标

1. ISA: rv32i;
2. CPU 主频: 50MHz, 5 段流水线;
3. InstrMem: 128KB;
4. DataMem: 128KB;
5. VGA 彩色模式: 640x480, 64 色;
6. VGA 字符模式: 70x30, ANSI 8 色;
7. 设备工作模式: 未实现中断，轮询机制;

## 项目分工

陈佳皓：移植 AM 软件到 `ARCH=riscv32-switch`，如 NTerm，Typing-game，OSLab 等，实现“运行诞生于未来的程序”；

茆弘之：实现五段流水线 CPU CPipe，起草 RISwitch 规约，全系统仿真框架搭建，实现“不停计算的机器”；

宋承柏：实现 DataMem，支持长度不超过 4 字节键码的键盘，字符终端，VGA，七段数码管，开关，LED 等外设，实现“来自外部的声音”；

## 项目特色

本项目的初始目标是在 FPGA 上用 Nanos-lite 启动马里奥，所以以 RISwitch (RISC-V + Switch) 为名，后续因为板上空间不够容纳 LiteNES（虚拟上板，修改内存容量后还是能跑的），时间不够大改 CPU 架构实现 CSR 和特权指令以及加总线而放弃。

本项目有着十分完备的基础设施，能够通过虚拟上板的方式弥补没有实现串口调试，物理上板综合时间长的问题。值得一提的是，不同于在 `ARCH=native` 或 `ARCH=riscv32-nemu` 上调试 AM 程序后再在板上验证的方式，本项目的虚拟上板同时能够仿真 RTL，在验证软件的同时测试硬件功能正确。因为保证了虚拟设备在时序等方面和硬件完全一致，实现了“只要虚拟上板能跑，物理上板只可能有时序问题，逻辑一定正确”的效果。
