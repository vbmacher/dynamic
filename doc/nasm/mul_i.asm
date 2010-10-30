BITS 32
segment .text
  ; MUL i
  ;   ram_env.r[0] *= ram_env.r[i];

  mov eax, [12345678h]   ; edx = ram_env.r[i]
  imul dword[12345678h]  ; eax *= ram_env.r[0]
  mov [12345678h], eax   ; ram_env.r[0] = eax
  