BITS 32
segment .text
  ; JZ i
  ;   if (ram_env.r[0] == 0)
  ;     ram_env.pc = s

  add dword[12345678h], 23456789h ; ram_env.pc += instr_count
  cmp dword[12345678h],0          ; (ram_env.r[0] == 0)
  jnz exit
  mov dword[12345678h], 23456789h ; ram_env.pc = s
  mov eax, 12345678h
  jmp eax                         ; jmp addr(s)
exit:
  