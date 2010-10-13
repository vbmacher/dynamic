BITS 32
segment .text
  ; SUB =i
  ;   ram_env.r[0] -= s;

  sub dword[12345678h], 23456789h  ; ram_env[0] -= s
  