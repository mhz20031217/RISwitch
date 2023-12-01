# RISwitch 计算机系统规约

作为 FPGA 和 上面 C 语言程序之间的抽象层，大部分规约定义和实现细节可以通过阅读 AM ARCH=riscv32-switch 了解。

## Freestanding 裸机运行时环境

指令集为 RV32I，无 fence.i 扩展；

### 地址空间 与 设备规约

所有非对齐访存行为为 UB。

以下所有范围为左闭右开。

1. 指令存储器: ROM, 只支持 4 字节对齐访存，大小 128KB，地址空间 0x80000000 ~ 0x80020000；
2. 数据存储器: RAM, 支持所有 RV32I 访存类型，大小 128KB，地址空间 0x80100000 ~ 0x80120000；
3. MMIO 区域

    MMIO 区域只支持 4 字节对齐访存，由 AM 软件实现保证这一点。

    1. 字符输出串口（7 段数码管控制形式 1）: 大小 4B，地址空间 0x80200000 ~ 0x80200004，只写，行为为当向这个设备寄存器写入后，7 段数码管会保存并一直 32 位无符号数的 16 进制表示，直到该端口被写入新内容；
    2. 键盘按键输入寄存器：大小 4B，地址空间 0x80500000 ~ 0x80500004，只读，内容为待读取的 1 个完整的 PS/2 通码或断码（意味着需要维护一个缓冲区，每次读取出队，如果缓冲区溢出可以 reset），不支持超过 32 位的键盘码（比如 <kbd>Pause/Break</kbd>, <kbd>PrtScn</kbd>）。比如：快速按下 A 键，按下 B 键，松开 A 键，松开 B 键，这个寄存器的 32 位值在四次读取的时候为 0x0000001C，0x00000032，0x0000F01C，0x0000F032；
    3. 时钟输入寄存器：大小 8B，地址空间 0x80300000 ~ 0x80300008，只读，内容为 64 位无符号数，代表从开机起经过的纳秒数。由于需要分两次读取，规定软件通过 CPU 读取整个寄存器时必须先读取低 32 位，再读取高 32 位。时钟设备保证以这样的行为访问时钟寄存器，高地址读取结果是读取低地址时的高 32 位；
    4. 字符显存缓冲区：大小 32 * 70 * 2B，地址空间 0x80400000 ~ 0x80401180，只写，对应一个 30 行x70 列的屏幕（为了简化地址计算，行数向上对齐到 32），列优先存储。每个 2B 中，高字节为颜色（只有低 6 位有效，其中，前景色为 [2:0]，背景色为 [5:3]，分别支持 8 种颜色，颜色编码详见 #TODO）；
    5. VGA 显存缓冲区（暂不实现）：大小 512x640 * 2B = 655360B，地址空间 0x80800000 ~ 0x808A0000，只写，对应一个 480 行x 640 列的屏幕（行数向上对齐到 512）。
    6. 显示模式控制寄存器：大小 4B，地址空间 0x80500000 ~ 0x80500004，只写，最低位为 0，选择字符渲染器结果输出，最低位为 1，选择显存输出；
    7. 开关将寄存器：大小 4B，地址空间 0x80600000 ~ 0x80600004，只读，低 2 字节有效，对应开关 `SW[15:0]`；
    8. LED 寄存器：大小 4B，地址空间 0x80700000 ~ 0x80700004，只写，低 2 字节有效，对应 `LED[15:0]`，行为为：写入后保持写入内容的状态，直到下一次写入；

### 程序的内存映像

本节规定如何加载一个程序镜像到存储器，以及程序如何开始运行和结束。

1. `.text` 节

    `.text` 节单独成段，将被加载到指令存储器，从 0x80000000 开始，其中 `entry`（第一条指令）位于 0x80000000 位置。

2. `.data`, `.rodata`, `.bss` 节

    `.data`, `.rodata`, `.bss` 和其他节将组成一段，被加载到 0x80100000 位置。

3. 程序如何开始

    从 `entry` 开始逐条指令运行，程序自行设置栈、堆等常见的存储区域；

4. 程序如何结束

    程序永不结束，如果执行了未实现的指令，行为为 UB；

## AM 抽象计算机 和 AM 上的 C 语言运行时环境

AM 本身是一台“抽象计算机”，但可以起到 RISwitch 计算机系统的“驱动程序”的作用，提供基本的 C 语言运行时。

### 实现细节

1. 栈在哪里？

    栈底在数据存储器中，`.bss` 节后面 4KB 对齐的位置再加上 32KB。（意味着局部数组不能过大）；

2. 堆在哪里？

    堆在数据存储器中，从栈底开始向上增长；

3. 程序如何开始

    `start.S` 的 `_start` 函数作为 `entry`，根据链接器导出的符号设置栈指针，然后调用 AM 应用程序的 `main` 函数；

    函数签名 `void main(const char *)`，该函数不能返回，否则行为为 UB；

4. 程序如何结束

    程序通过调用 AM 的 `halt(int code)` 函数结束，将返回值设置为 `code`；

### 提供的寄存器

AbstractMachine 作为计算机，提供一些寄存器可以读取和写入。这些寄存器就是 C 语言结构体。

由于 FPGA 上的设备较为特殊，和 AM 实现的寄存器不尽相同，因此更改了 AM 寄存器的定义，放在 `$AM_HOME/am/include/amdev-switch.h` 中。

#### 定义

```c 
/* @member present: whether timer module is present
 * @member has_rtc: whether real time clock is present 
 */
AM_DEVREG( 4, TIMER_CONFIG, RD, bool present, has_rtc);
AM_DEVREG( 5, TIMER_RTC,    RD, int year, month, day, hour, minute, second);
/* @member us: microseconds passed since bootup
 */
AM_DEVREG( 6, TIMER_UPTIME, RD, uint64_t us);
/* @member present: whether keyboard is present
 */
AM_DEVREG( 7, INPUT_CONFIG, RD, bool present);
/* @member keydown: whether one of the keys is pressed
 * @member keycode: AM Keycode for the pressed key
 */
AM_DEVREG( 8, INPUT_KEYBRD, RD, bool keydown; int keycode);
/* @member value: next value to show on 7-seg display
 */
AM_DEVREG(25, SEG,          WR, uint32_t value);
/* @member value: current value of switches
 */
AM_DEVREG(26, SWITCH,       RD, uint16_t value);
/* @member value: next value to show on LED
 */
AM_DEVREG(27, LED,          WR, uint16_t value);
/* @member present: whether char-based memory is present 
 * @member width: the width of cmem (in chars)
 * @member heigth: the height of cmem (in chars)
 */
AM_DEVREG(28, CMEM_CONFIG,  RD, bool present; int width, height);
/* Put a char ASCII on (x, y) (starting from 0) with fg_color FG and bg_color BG. */
AM_DEVREG(29, CMEM_PUTCH,   WR, int x, y; char ascii; uint8_t fg, bg);
```

#### 使用示例

1. 读取当前按键

```c
/* fceux-am/src/drivers/sdl/input.c */
static void KeyboardCommands ()
{
	// get the keyboard input
  int keycode;
  do {
#define KEYDOWN_MASK 0x8000
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    assert(ev.keycode < 256);
    keycode = ev.keycode;
    g_keyState[keycode] = ev.keydown;
    if (keycode == AM_KEY_ESCAPE || keycode == AM_KEY_Q) halt(0);
  } while (keycode != AM_KEY_NONE);

	// Toggle throttling
	NoWaiting &= ~1;
}
```

2. 等待一段时间

```c
void delay(uint32_t us) {
  uint64_t st = io_read(AM_TIMER_UPTIME).us;
  while (io_read(AM_TIMER_UPTIME).us - st < us);
}
```

3. 字符显存写入

```c
int xPos, yPos;
char ch;
unsigned char fg, bg;
io_write(AM_CMEM_PUTCH, .x = xPos, .y = yPos, .ascii = ch, .fg = fg, .bg = bg);
```

### 可使用的库函数

见 `$AM_HOME/klib/include/klib.h`；
