# set -e
# gcc -I./svpng -I. ../main.c -lm -o heart
gcc -o test ../test_api.c -I /usr/include/cplot/ -lcplot
./test 2>errs.log 1>api.png
# gcc -I./svpng -I. -Dtupper ../main.c -lm -o tupper
# ./heart
# ./tupper

# w=8
# h=4
# k=2857740885
# ./heart "$k*$h" "$k*$h+$h" 300 "0" "$w" 600 "FLOOR(FLOOR(y/$h)/2^(FLOOR(x)+$w*FLOOR(y)%$h))%2<1" 2>errs.log > grid.png
# ./heart "-8*pi" "8*pi" 300 "-8*pi" "8*pi" 600 "y<x*SIN(x)" 2>errs.log > out.png

# ./cplot "-pi" "pi" 300 "-pi" "pi" 300 "1+TAN(X^2)*TAN(Y^2)=0.5*TAN(X^2)" 2>errs.log 1>out.png
