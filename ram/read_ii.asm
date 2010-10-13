BITS 32
segment .text
  ; READ *i
  ; ram_env.r[ram_env.r[i]] = ram_env.input[ram_env.p_input++];

  mov eax, [12345678h]  ; eax = &ram_env.p_input
  add eax, [12345678h]  ; eax += ram_env.input
  movsx eax, byte [eax] ; eax = [eax]
  mov edx, [12345678h]  ; edx = ram_env.r[i]
  mov [edx*4 + 12345678h], eax ; [edx*4+ram_env.r[0]] = eax
  inc dword[12345678h]  ; ram_env.p_input++
