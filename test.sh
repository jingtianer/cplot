set -e
gcc -I./svpng ../main.c -lm -o heart
./heart
k="960939379918958884971672962127852754715004339660129306651505519271702802395266424689642842174350718121267153782770623355993237280874144307891325963941337723487857735749823926629715517173716995165232890538221612403238855866184013235585136048828693337902491454229288667081096184496091705183454067827731551705405381627380967602565625016981482083418783163849115590225610003652351370343874461848378737238198224849863465033159410054974700593138339226497249461751545728366702369745461014655997933798537483143786841806593422227898388722980000748404719"
./heart "$k" "$k+16" 100 "0" "105" 1000 "1/2-FLOOR((FLOOR(y/17)*2^(-17*FLOOR(X)-FLOOR(y)%17))%2)" 2>errs.log > png.png

w=8
h=4
k=2857740885
./heart "$k*$h" "$k*$h+$h" 300 "0" "$w" 600 "FLOOR(FLOOR(y/$h)/2^(FLOOR(x)+$w*FLOOR(y)%$h))%2-1" 2>errs.log > grid.png
./heart "-8*pi" "8*pi" 300 "-8*pi" "8*pi" 600 "y-x*SIN(x)" 2>errs.log > grid.png