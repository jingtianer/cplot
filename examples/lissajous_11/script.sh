h=2160 && w=3840 && a=1 && b="pi" && max=1 && for i in `seq $max`; do cplotter parametric -1 1 $h "-1*$w/$h" "1*$w/$h" $h 0 "226*pi*$i/$max" 50000 "SIN($a*t)" "COS($b*t)" > lissajous_11/test$i.png; done