; This program computes Mandelbrot set

; input: (x,y) pairs ended by 'd' (ascii 100)

; 35 positions of X (64 - 99 in ascii)
; 20 positions of Y (64 - 84 in ascii)

;registers:
;  0: acc
;  1: input read
;  2: input scaled X
;  3: input scaled Y
;  4: current computed X
;  5: current computed Y
;  6: iteration
;  7: max_iteration
;  8: tmp, x*x
;  9: tmp, y*y
; 10: tmp, xtemp

; read X
read 1

; test for input end
load 1
sub =100
jz exit

; scale X, assume its allright
load 1
sub =64
store 2

; read Y, assume its there and that its allright
read 1
sub =64
store 3

load =0
store 4
store 5
store 6

load =255 ; max_iteration
store 7

compute:
  load 4 ; x*x
  mul 4
  store 8
  
  load 5
  mul 5  ; y*y
  store 9
  
  add 8
  sub =255 ; origin 2*2, my opinion at least 20*20 (400)
  jgtz endloop
  
  load 6
  sub 7
  jgtz endloop
  jz endloop
  
  ;while ( x*x + y*y <= (2*2)  AND  iteration < max_iteration )

  load 8
  sub 9
  add 2
  
  store 10
  
  load =2
  mul 4
  mul 5
  add 3
  store 5
  
  load 10
  store 4
  
  load 6
  add =1
  store 6
  
  jmp compute

endloop:

  write 2
  write 3

  load 6
  sub 7
  jz black
  load 6
  add =65
  store 6
  write 6
  jmp exit
black:
  write 65

exit:
  halt

