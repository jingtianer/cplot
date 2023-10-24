set -e
gcc -I./svpng -DGREATER_CHAR=\'.\' main.c -lm -o heart
# ./heart -1 1 "500" -1 1 "500" "x*x+y*y-1" 2>errs.log
# ./heart "-pi/2" "pi/2" 500 "-3*pi" "2*pi" 500 "y^2-SIN(x+y)^2" 2>errs.log
# ./heart "-pi/2" "pi/2" 500 "-3*pi" "2*pi" 500 "y^2-SIN(x)^2" 2>errs.log
# ./heart "-2" "ACOS(1/2)-pi/4" 500 "-pi/2" "pi/2" 500 "y*y+x*x+y-SQRT(y*y+x*x)" 2>errs.log
 ./heart "-pi" "1" 500 "-2" "2" 500 "(ACOS(1-FABS(x))-pi)-y" "y-SQRT(1-(FABS(x)-1)^2)" 2>errs.log
# ./heart "-1" "pi/2" 500 "-1" "1" 500 "x*x+(y-FABS(x)^(2.0/3.0))^2-1" 2>errs.log
# ./heart "-SQRT(3)" "SQRT(3)" 500 "-2" "2" 500 "x^2-y^2-1" 2>errs.log
#./heart "-4" "4" 1500 "0" "2*pi" 1500 "y-5*EXP(-x)*SIN(6*x)" 2>errs.log
# ./heart "-0.5" "0.5" 500 "-1" "1" 500 "x^4-x^2+y^2" 2>errs.log
# ./heart "-1*pi/2" "7*pi/2" 500 "-pi*4.5" "5.5*pi" 500 "SIN(X)+SIN(Y)" 2>errs.log
# ./heart "-1*pi/2" "7*pi/2" 500 "-pi*4.5" "5.5*pi" 500 "SIN(X)*SIN(Y)" 2>errs.log
# ./heart "-1" "1" 500 "-1" "1" 500 "X^2^(1/3)+Y^2^(1/3)-1" 2>errs.log
# # gcc heart.c -lm -o heart
# # N=3
# # size=1
# # for i in `seq $N`; do
# #     ./heart "-pi" "1" "(1+pi)/20" "-2/$size" "2/$size" "4/40" "(ACOS(1-FABS($size*x))-pi)-$size*y" "$size*y-SQRT(1-(FABS($size*x)-1)^2)" 2>errs.log 1>out.$i
# #     size=$(echo "scale = 8; $size * sqrt(2)" | bc)
# #     echo $size
# # done
# # paste `for i in $(seq $N | tr ' ' '\n'| tac | tr '\n' ' '); do echo out.$i; done;for i in $(seq 2 $N); do echo out.$i; done` > out.paste
# # cat out.paste;cat out.paste;


# ./heart "-2*pi" "2*pi" 500 "-2*pi" "2*pi" 500 "SIN(x*y)" 2>errs.log
# ./heart "-2*pi" "2*pi" 500 "-2*pi" "2*pi" 500 "SIN(x/y)" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500 "SIN(x-y)" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500 "SIN(SIN(X*Y))" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500 "SIN(COS(X*Y))" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500 "SIN(LOG(X*y))" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500  "TAN(X*y)" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500 "EXP(X*y)" 2>errs.log
# ./heart "-4*pi" "4*pi" 500 "-4*pi" "4*pi" 500 "EXP(X+y)" 2>errs.log
# ./heart "-4" "4" 500 "-4" "4" 500 "FLOOR(X)-y" 2>errs.log
# ./heart "-pi" "pi" 500 "-pi" "pi" 500 "SIN(X*x+Y*y)-COS(X-Y)" 2>errs.log
# ./heart "-pi" "pi" 500 "-pi" "pi" 500 "SIN(X*x+Y*y)-COS(X*Y)" 2>errs.log
# ./heart "-2*pi" "2*pi" 500 "-2*pi" "2*pi" 500 "SIN(X*x+Y*y)-COS(X)-COS(Y)" 2>errs.log
# ./heart "-2*pi" "2*pi" 500 "-2*pi" "2*pi" 500 "SIN(X*x+Y*y)-SIN(X)-SIN(Y)" 2>errs.log
# k=2857740885
# w=8
# h=4
# ./heart "0" "$w" "$w/20" "$k*$h" "$k*$h+$h" "$h/40" "FLOOR(FLOOR(y/$h)/2^(FLOOR(y)%$h*$w+FLOOR(x)))%2-1" 2>errs.log
# ./heart "0" "$w" "$w/20" "$k*$h" "$k*$h+$h" "$h/40" "(X+y)%2-1" 2>errs.log
# ./heart "0" "$w" "$w/20" "$k*$h" "$k*$h+$h" "$h/40" "y-1.5" "3%2-y" 2>errs.log