BITS 32
segment .text
  ; LOAD *i
  ; ram_env.r[0] = ram_env.r[ram_env.r[i]];

  mov eax, [12345678h]         ; eax = [ram_env.r[i]]
  mov eax, [eax*4 + 12345678h] ; eax = [eax*4+ram_env.r[0]]
  mov [12345678h], eax         ; ram_env[0] = eax