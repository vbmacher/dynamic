BITS 32
segment .text
  ; ADD =i
  ;   ram_env.r[0] += s;

  add dword[12345678h], 23456789h  ; ram_env[0] += s
  