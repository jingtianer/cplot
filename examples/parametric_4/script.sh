a=1 && b=0.1 && max=180 && for i in `seq $max`; do cplotter parametric -1.25 1.25 1080 "-1.25*1920/1080" "1.25*1920/1080" 1080 0 "2*pi*$i/$max" 5000 "($a+$b)*SIN(t)-$b*SIN(($a+$b)/$b*t)" "($a+$b)*COS(t)-$b*COS(($a+$b)/$b*t)" > lissajous_4/test$i.png; done