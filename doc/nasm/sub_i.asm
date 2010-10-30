BITS 32
segment .text
  ; SUB i
  ;   ram_env.r[0] -= ram_env.r[i];

  mov edx, [12345678h]       ; edx = ram_env.r[i]
  sub [12345678h], edx       ; ram_env[0] -= edx
  