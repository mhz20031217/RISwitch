sync_csr:
  csrw mepc, a0;
  csrw mstatus, a1;
  csrw mcause, a2;
  csrw mtvec, a3;