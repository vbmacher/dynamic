BITS 32
segment .text
  ; READ i
  ; ram_env.r[i] = ram_env.input[ram_env.p_input++];

  mov eax, [dword 0]       ;
  add eax, [dword 0]
  movsx eax, byte [eax]
  mov dword[dword 0], eax
  inc dword[dword 0]
