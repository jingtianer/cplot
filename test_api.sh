gcc -o test ../test_api.c -I /usr/include/cplot/ -lcplot
./test 2>errs.log 1>api.png
