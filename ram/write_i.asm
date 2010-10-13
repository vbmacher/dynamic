BITS 32
segment .text
  ; WRITE i
  ;   if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
  ;     ram_env.state = RAM_OUTPUT_FULL;
  ;     return;
  ;   }
  ;   ram_env.output[ram_env.p_output++] = ram_env.r[i];

  mov eax, [12345678h]       ; eax = ram_env.p_output
  cmp eax, byte 100         ; RAM_OUTPUT_SIZE
  jl short here
  mov dword[12345678h], 100 ; ram_env.state = RAM_OUTPUT_FULL
  jmp short ending
 here:
  mov edx, [12345678h]       ; edx = ram_env.r[i]
  add eax, 12345678h       ; eax += &ram_env.output
  mov [eax], edx             ; [eax] = edx
  inc dword[12345678h]       ; ram_env.p_output++
ending:
	ret