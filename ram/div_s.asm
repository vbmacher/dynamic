BITS 32
segment .text
  ; DIV =s
  ;   if (!s) {
  ;     ram_env.state = RAM_DIVISION_BY_ZERO;
  ;     return;
  ;   }
  ;   ram_env.r[0] /= s;

  mov ebx, 12345678h        ; ebx = s  
  cmp ebx, 0                ; ebx == 0 ?
  jne short here
  mov dword[12345678h], 64h ; ram_env.state = RAM_DIVISION_BY_ZERO
  jmp short ending
  
here:
  mov eax, [12345678h]      ; ram_env.r[0]
  cdq
  idiv ebx
  mov [1234678h], eax


;  mov ebx, 12345678h        ; ebx = s
  ;cmp ebx, 0                ; ebx == 0 ?
  ;jne short here
  ;mov dword[12345678h], 64h ; ram_env.state = RAM_DIVISION_BY_ZERO
  ;jmp short ending
;here:
  ;xor edx, edx              ; edx = 0
  ;mov eax, dword[12345678h] ; eax = ram_env.r[0]
  ;idiv ebx                  ; eax = edx:eax / s
  ;mov [12345678h], eax      ; ram_env.r[0] = eax
ending: