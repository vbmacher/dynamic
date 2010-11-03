; input : X0 - string X ended by zero, X={1,2,3,....}*
; output: XR - reversed string X

load =10
store 2

citaj:
  read 1
  load 1
  jz vypis
  store *2
  load 2
  add =1
  store 2
  jmp citaj
vypis:
  load 2
  sub =1
  store 2
  sub =9
  jz koniec
  write *2
  jmp vypis
koniec:
  halt