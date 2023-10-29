cmake ..
make install
echo test api
gcc -o test ../test_api.c -I /usr/include/cplot/ -lcplot
./test 2>errs.log 1>api.png
echo test slop
gcc -o test ../test_slop.c -I /usr/include/cplot/ -lcplot -lm
echo test slop
./test 2>errs.log 1>slops.txt

# 10 : 14
# 100 : 14
# 1000 : 46