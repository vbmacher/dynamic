BITS 32
segment .text
  ; LOAD =i
  ;   ram_env.r[0] = s;

  mov dword[12345678h], 23456789h  ; ram_env.r[0] = s