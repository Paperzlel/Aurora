cp scripts/$1.diff toolchain/$1-$2/$1.diff
cd toolchain/$1-$2
patch -p1 -N < $1.diff
cd ../../
