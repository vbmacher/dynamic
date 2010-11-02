BITS 32
segment .text
  ; JMP i
  ;   ram_env.pc = s;

  mov dword[12345678h], 23456789h  ; ram_env.pc = s
  mov eax, 12345678h
  jmp eax ; jmp addr(s)
  