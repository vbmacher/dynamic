BITS 32
segment .text
  ; DIV i
  ;  if (ram_env.r[ram_env.r[i]] == 0) {
  ;    ram_env.state = RAM_DIVISION_BY_ZERO;
  ;    return;
  ;  }
  ;  ram_env.r[0] /= ram_env.r[ram_env.r[i]];

  mov ebx, [12345678h]         ; ebx = ram_env.r[i]
  mov ebx, [ebx*4 + 12345678h] ; ebx = [ebx*4 + &ram_env.r[0]]
  
  cmp ebx, 0                   ; ebx == 0 ?
  jne short here
  mov dword[12345678h], 64h    ; ram_env.state = RAM_DIVISION_BY_ZERO
  jmp short ending

here:
  mov eax, [12345678h]         ; ram_env.r[0]
  cdq
  idiv ebx                     ; eax = eax / ebx
  mov [1234678h], eax          ; ram_env.r[0] = eax
ending: