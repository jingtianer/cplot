y="(13*COS(t)-5*COS(2*t)-2*COS(3*t)-COS(4*t))/16"
x="SIN(t)^3"
max=180 && for i in `seq $max`; do cplotter parametric -1.2 0.8 1080 "-1*1920/1080" "1*1920/1080" 1080 0 "2*pi*$i/$max" 5000 "$y" "$x" 1> parametric_7/test$i.png; done