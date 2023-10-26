## A plot tool Based on C

### dependency
- [svpng](https://github.com/miloyip/svpng)
- cmake 3.10
- gcc

### build
```sh
git clone https://github.com/jingtianer/cplot
cd cplot
mkdir cmake-build-debug
cmake .. 
make
make install
```
### Usage

#### USE_PNG
```
cplot "y1" "y2" "height" "x1" "x2" "width" "<expressions...>" 2>errs.log 1>out.png
```

#### bool operator
```sh
# and: seperate expressions by ','
"expression1,expression2,..."
cplot "-2" "2" 300 "-2" "2" 300 "x<1,x>-1,y<1,y>-1" 2>errs.log 1>out.png
# or: 
"expression1" "expression2"
cplot "-2" "2" 300 "-2" "2" 300 "x>1" "x<-1" "y>1" "y<-1" 2>errs.log 1>out.png
```

### expression format
- rules
  - no blank characters
  - `pi` represents `circumference ratio, pi`, `e` represents `Euler's number, e`
  - all name of unary functions are upper case
  - must contains exactly one comparision operator
    - grater than: `>`
    - smaller than: `<` 
    - equals `=` 
    - not smaller than: `>=` 
    - not greater than: `<=` 
    - not equals: `<>`
  - `x` or `X` represents for x-Axis
  - `y` or `Y` represents for y-Axis
  - numbers
    - same rule with [strtold](https://en.cppreference.com/w/c/string/byte/strtof)
  - multiplier can not be ignored

### function list
> must be upper case

|function|explaination|
|-|-|
|acos|arccos|
|asin|arcsin|
|atan|arctan|
|cos|cosine|
|cosh|hyperbolic cosine|
|sin|sine|
|sinh|hyperbolic sine|
|tan|tangent|
|tanh|hyperbolic tangent|
|exp|Exponential, Base e|
|log|log|
|floor|floor|
|sqrt|square root function|
|fabs|absolute|
|ceil|ceil|

## operators
|operators|explaination|
|-|-|
|%|mod|
|*|multiply|
|+|plus|
|-|minus|
|/|div|
|^|pow|

### configuration

all configuration defined in`config.h`

