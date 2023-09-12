---
title: PA0 实验报告
author: 221830012 茆弘之
date: 2023 年 09 月 10 日
---

**实验进度**: 我完成了所有内容（必做题、选做题）；

## 必做题

### Write a "Hello World" program under GNU/Linux, Write a Makefile to compile the "Hello World" program, Learn to use GDB

详见 `pa/pa0/hello.c`, `pa/pa0/Makefile`；

### 在github上添加ssh key

已添加；

### 读后感 / 实验心得

实际上，从我 7 月 2 号参加 ysyx 时，PA0 相当于已经开始了。这两个月，我终于有时间学习程序设计的一些基础设施（命令行工具、编译工具链、软件开发相关工具），配置开发环境并加深对计算机系统的理解（RTFM，RTFSC）了。

#### 通过 STFW, RTFM, RTFSC 独立解决问题  

想要解决一个问题，我们需要知道解决问题的方法。

我们可以从外界直接获得这个方法，询问他人、上网搜索、阅读手册属于这个范畴。我们尝试解决的问题常常关于一个特定的产品，这个产品是由他人生产制造的，而生产者一般已经将这个事物的性质研究透彻、记录在了文档中。产品的使用者在使用过程中也可能记录下使用方法。从外界直接获得方法节省了我们自己研究事物性质的时间。

我们也可以自己研究这个产品来得出解决问题的方法，但是这需要不少时间。

在计算机科学领域，程序及其源代码就是产品，手册就是产品说明书，网上别人写的相关文章就是使用方法的简介。可以看出，信息的流向是： $\mathrm{src}\Rightarrow \mathrm{manual} \Rightarrow \mathrm{blog} \Rightarrow \mathrm{user experience}$。从左到右，解决方案更加简单易懂，但是信息的质量逐级递减，所包含的细节不断丢失。因此，在解决问题时，STFW, RTFM, RTFSC 是有顺序的。

首先应该尝试走捷径，STFW，了解问题的大概面貌；如果问题没有解决，或者解决方案不正确，说明走捷径失败，应该 RTFM；最后，如果问题还是没有解决，我们应该 RTFSC。有句话说得好“哪有什么岁月静好，不过是有人在为你负重前行。”我们不能一味等待别人为我们提供解决方案。作为计算机科学专业的学生，在还没有记录的专业问题上，应该主动探索，提出自己的解决方案，在社区的文档中记录下来，为后人提供方便。

还记得在高中阶段，作业答案没有过程，唯有自己购买的习题册能够将问题的解法深入剖析（STFW）。对于各种题型，我们可以自己从头推算，自己总结出模型（RTFSC），也可以等着老师讲解，或者问同学，但是，老师要面对超过 50 个学生，同学也常常没有时间耐心地为别人解答问题。到了计算机科学领域也是这样，我们最好独立解决问题，这样既锻炼了自己，也方便了他人。

实际上，掌握正确阅读手册的方法后，看似费时费力的 RTFM 反而常常是最快的、最稳定的解决方案。在计算机科学领域，手册常常扮演行为规范的角色。而计算机体系结构正是建立在一层层由各种规范定义的接口所建立的抽象上的。

#### 好的提问

如果问题过于复杂，自己实在解决不了，我们需要掌握提问的艺术，准确描述问题，让别人乐意为我们解决问题。即使是 ChatGPT，准确的 Prompt 往往能产生优质的回答。

## 选做题

### Why executing the "poweroff" command requires superuser privilege?

答：如果普通用户有执行 `poweroff` 程序的权限。可以设想以下情形：黑客利用漏洞获取了一个由 GNU/Linux 驱动的工业设备的普通用户权限，并执行了 `poweroff` 命令，可能导致设备损坏，甚至人员伤亡；

### Things behind scrolling

答：在原始的 tty 中，内容确实不能滚动，但是在一些终端模拟器（如 gnome-terminal, konsole）中，内容是可以滚动的（按<kbd>Shift</kbd><kbd>PgUp</kbd> / <kbd>Shift</kbd><kbd>PgDn</kbd>）。关于 tmux 如何使得终端内容可以滚动，以及如何实现这个功能：在 Unix 中，每一个程序有一个 Controlling Terminal，这个程序的 `stdin`, `stdout` 和 `stderr` 如果没有重定向，是从 `/dev/tty` 输入输出。输出的内容是字符流，tmux 充当了里面运行程序的 tty，收集了程序输出的文本，然后负责显示、换行等。实现滚动很简单，只要实现一个 buffer 记录一定长度的历史文本流就可以了。如果这个问题侧重点是怎样实现滚动的效果，用 C++ 可以定义 `TextBuffer` 类和 `ViewPoint` 类，其中 `TextBuffer` 是一个按行的环形缓冲区，而 `ViewPoint` 负责根据当前位置、Pane 的长宽取 `TextBuffer` 中的数据，负责断行和显示；

参考资料：

- [1] [tmux/tty.c](https://github.com/tmux/tmux/blob/master/tty.c)
- [2] [What is the purpose of the controlling terminal?](https://unix.stackexchange.com/questions/404555/what-is-the-purpose-of-the-controlling-terminal)

### What happened? (about `make` execution)

答：关于 `make` 本身，当执行 `make` 时，`make` 读取 `Makefile` 并解析，取得一些变量、规则等，解析它们的依赖关系，然后执行用户指定的规则。关于 `make` 具体如何执行规则，这个过程可以用 `strace make` 一探究竟，关键点是 `make` 将会查看一些文件的存在情况和创建时间。`pa/pa0/strace.out` 是已经完成过编译后 `strace make` 的输出。

由 `make` 的手册可知，执行一个规则的一行命令前，先进行一次变量展开，然后创建一个 shell 进程执行这行命令。所以，`make` 执行的过程就是自动执行编译或其他各种命令的过程。
