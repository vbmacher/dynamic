load =1
loop5:
store 1

load =1
loop4:
store 2

load =127
loop3:
store 3

load =127

loop2:
store 4

load =127

loop1:
store 5
load =127

loop0:
store 6

;load =101
;div =3
;div =3
;div =3
;div =3
;div =3
;div =3
;div =3
;div =3

;load =2
;mul =3
;mul =3
;mul =3
;mul =3
;div =7
;div =7
;div =7

load 6
sub =1
jgtz loop0

load 5
sub =1
jgtz loop1

load 4
sub =1
jgtz loop2

load 3
sub =1
jgtz loop3

load 2
sub =1
jgtz loop4

load 1
sub =1
jgtz loop5

write =2
halt