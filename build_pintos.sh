call='pfs'
exCallPath='../../examples/pfs'
reader='pfs_reader'
readerPath='../../examples/pfs_reader'
writer='pfs_writer'
writerPath='../../examples/pfs_writer'
cd src/examples/
make
cd ../threads/
make
cd ../userprog/
make
cd build/
pintos --qemu -- -f -q
pintos --qemu -p '../../examples/pfs_writer' -a 'pfs_writer' -- -q
pintos --qemu -p $readerPath -a $reader -- -q
pintos --qemu -p $exCallPath -a $call -- -q
#pintos --qemu -p ../../examples/lab4test1 -a \'lab4test1 -s 1233' -- -q

#pintos --qemu -p $printfPath -a $printf -- -q
#pintos --qemu --gdb -- run $call
#pintos --qemu -- run $call
# Max arguments
#pintos --qemu -- run 'lab4test1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
pintos --qemu -- run $call
pintos --qemu -- cat messages
#ddd --debugger pintos-gdb kernel.o
#pintos --qemu -- ls
clear
# make SIMULATOR=--qemu check

# Projects/repositories/tddb68-vt20-margu424-axega544/pintos/src/userprog/build/userprog/syscall.o
