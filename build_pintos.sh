call='lab1test2'
exCallPath='../../examples/lab1test2'
cd src/examples/
make
cd ../threads/
make
cd ../userprog/
make
cd build/
pintos --qemu -- -f -q
pintos --qemu -p $exCallPath -a $call -- -q
#pintos --qemu --gdb -- run $call
pintos --qemu -- run $call
#ddd --debugger pintos-gdb kernel.o
#pintos --qemu -- ls
clear
# make SIMULATOR=--qemu check

# Projects/repositories/tddb68-vt20-margu424-axega544/pintos/src/userprog/build/userprog/syscall.o
