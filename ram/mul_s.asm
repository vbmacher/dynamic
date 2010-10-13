BITS 32
segment .text
  ; MUL =i
  ;   ram_env.r[0] *= s;

  imul eax, dword[12345678h], 23456789h  ; eax = ram_env.r[0] * s
  mov [12345678h], eax
  