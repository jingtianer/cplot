cmake ..
make install
echo test api
gcc -o test_api ../demo/test_api.c -I /usr/include/cplot/ -lcplot
./test_api

echo test polar
gcc -o test_polar ../demo/test_polar.c -I /usr/include/cplot/ -lcplot
./test_polar


echo test parametric
gcc -o test_parametric ../demo/test_parametric.c -I /usr/include/cplot/ -lcplot
./test_parametric


# echo test tupper
# gcc -o test_tupper ../demo/test_tupper.c -I /usr/include/cplot/ -lcplot
# ./test_tupper

# echo test slop
# gcc -o test_slop ../demo/test_slop.c -I /usr/include/cplot/ -lcplot -lm
# ./test_slop
