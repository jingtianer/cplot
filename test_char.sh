set -e
gcc -DUSE_CHAR -DLOG_LEVEL=7 -I./svpng main.c -lm -o heart
 ./heart -1 1 "1/20" -1 1 "1/20" "x*x+y*y-1" 2>errs.log
# ./heart "-pi/2" "pi/2" "1/20" "-3*pi" "2*pi" "1/20" "y^2-SIN(x+y)^2" 2>errs.log
# ./heart "-pi/2" "pi/2" "1/20" "-3*pi" "2*pi" "1/20" "y^2-SIN(x)^2" 2>errs.log
# ./heart "-2" "ACOS(1/2)-pi/4" "1/20" "-pi/2" "pi/2" "1/20" "y*y+x*x+y-SQRT(y*y+x*x)" 2>errs.log
# ./heart "-pi" "1" "1/20" "-2" "2" "1/20" "(ACOS(1-FABS(x))-pi)-y" "y-SQRT(1-(FABS(x)-1)^2)" 2>errs.log
# ./heart "-1" "pi/2" "1/20" "-1" "1" "1/20" "x*x+(y-FABS(x)^(2.0/3.0))^2-1" 2>errs.log
# ./heart "-SQRT(3)" "SQRT(3)" "1/20" "-2" "2" "1/20" "x^2-y^2-1" 2>errs.log
#./heart "-4" "4" "1/20" "0" "2*pi" "1/20" "y-5*EXP(-x)*SIN(6*x)" 2>errs.log
# ./heart "-0.5" "0.5" "1/20" "-1" "1" "1/20" "x^4-x^2+y^2" 2>errs.log
# ./heart "-1*pi/2" "7*pi/2" "4*pi/40" "-pi*4.5" "5.5*pi" "10*pi/80" "SIN(X)+SIN(Y)" 2>errs.log
# ./heart "-1*pi/2" "7*pi/2" "1/20" "-pi*4.5" "5.5*pi" "1/20" "SIN(X)*SIN(Y)" 2>errs.log
# ./heart "-1" "1" "1/20" "-1" "1" "1/20" "X^2^(1/3)+Y^2^(1/3)-1" 2>errs.log
#  N=3
#  size=1
#  for i in `seq $N`; do
#      ./heart "-pi" "1" "(1+pi)/20" "-2/$size" "2/$size" "4/40" "(ACOS(1-FABS($size*x))-pi)-$size*y" "$size*y-SQRT(1-(FABS($size*x)-1)^2)" 2>errs.log 1>out.$i
#      size=$(echo "scale = 8; $size * sqrt(2)" | bc)
#      echo $size
#  done
#  paste `for i in $(seq $N | tr ' ' '\n'| tac | tr '\n' ' '); do echo out.$i; done;for i in $(seq 2 $N); do echo out.$i; done` > out.paste
#  cat out.paste;cat out.paste;


# ./heart "-2*pi" "2*pi" "1/20" "-2*pi" "2*pi" "1/20" "SIN(x*y)" 2>errs.log
# ./heart "-2*pi" "2*pi" "1/20" "-2*pi" "2*pi" "1/20" "SIN(x/y)" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20" "SIN(x-y)" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20" "SIN(SIN(X*Y))" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20" "SIN(COS(X*Y))" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20" "SIN(LOG(X*y))" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20"  "TAN(X*y)" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20" "EXP(X*y)" 2>errs.log
# ./heart "-4*pi" "4*pi" "1/20" "-4*pi" "4*pi" "1/20" "EXP(X+y)" 2>errs.log
# ./heart "-4" "4" "1/20" "-4" "4" "1/20" "FLOOR(X)-y" 2>errs.log
# ./heart "-pi" "pi" "1/20" "-pi" "pi" "1/20" "SIN(X*x+Y*y)-COS(X-Y)" 2>errs.log
# ./heart "-pi" "pi" "1/20" "-pi" "pi" "1/20" "SIN(X*x+Y*y)-COS(X*Y)" 2>errs.log
# ./heart "-2*pi" "2*pi" "1/20" "-2*pi" "2*pi" "1/20" "SIN(X*x+Y*y)-COS(X)-COS(Y)" 2>errs.log
# ./heart "-2*pi" "2*pi" "1/20" "-2*pi" "2*pi" "1/20" "SIN(X*x+Y*y)-SIN(X)-SIN(Y)" 2>errs.log
# k=2857740885
# w=8
# h=4
# ./heart "0" "$w" "$w/20" "$k*$h" "$k*$h+$h" "$h/40" "FLOOR(FLOOR(y/$h)/2^(FLOOR(y)%$h*$w+FLOOR(x)))%2-1" 2>errs.log
# ./heart "0" "$w" "$w/20" "$k*$h" "$k*$h+$h" "$h/40" "(X+y)%2-1" 2>errs.log
# ./heart "0" "$w" "$w/20" "$k*$h" "$k*$h+$h" "$h/40" "y-1.5" "3%2-y" 2>errs.log