y="COS(t)*(EXP(COS(t))-2*COS(4*t)-SIN(t/12)^5)"
x="SIN(t)*(EXP(COS(t))-2*COS(4*t)-SIN(t/12)^5)"
max=180 && for i in `seq $max`; do cplotter parametric -2.5 3.5 1080 "-3.5*1920/1080" "3.5*1920/1080" 1080 0 "6*pi*$i/$max" 5000 "$y" "$x" 1> parametric_8/test$i.png; done