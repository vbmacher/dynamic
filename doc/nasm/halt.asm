BITS 32
segment .text
  ;   ram_env.state += instr_count
  ;   ram_env.state = RAM_HALT;

  add dword[12345678h], 23456789h  ; ram_env.pc += s
  mov dword[12345678h], 23456789h  ; ram_env.state = RAM_HALT
  pop ebp
  ret
