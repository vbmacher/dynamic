BITS 32
segment .text
  ; STORE *i
  ; ram_env.r[ram_env.r[i]] = ram_env.r[0];

  mov eax, [12345678h]         ; eax = ram_env[0]
  mov edx, [12345678h]         ; edx = [ram_env.r[i]]
  mov [edx*4 + 12345678h], eax ; [edx*4+ram_env.r[0]] = eax
  