void fun() {
    extern unsigned char _end;
    volatile unsigned char *p = &_end;
    
    *p = 0;
    while (*p != 0xff) {
        *p = 0x33;
        *p = 0x34;
        *p = 0x86;
    }
}
