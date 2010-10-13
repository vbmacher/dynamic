BITS 32
segment .text
  ; STORE i
  ;   ram_env.r[i] = ram_env.r[0];

  mov eax, [12345678h]  ; eax = ram_env.r[0]
  mov [12345678h], eax  ; ram_env[i] = eax
  