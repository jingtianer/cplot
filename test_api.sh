cmake ..
make install
echo test api
gcc -o test_api ../test_api.c -I /usr/include/cplot/ -lcplot
./test_api 2>errs.log 1>api.png

echo test tupper
gcc -o test_tupper ../test_tupper.c -I /usr/include/cplot/ -lcplot
./test_tupper 2>errs.log

echo test slop
gcc -o test_slop ../test_slop.c -I /usr/include/cplot/ -lcplot -lm
./test_slop 2>errs.log 1>slops.txt

# 10 : 14
# 100 : 14
# 1000 : 46