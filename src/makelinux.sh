#!/bin/bash
echo cpp2
gcc cpp.c -DLINUXBUILD -o ../bin/cpp2
echo as
gcc as.c -DLINUXBUILD -o ../bin/as
echo cc
gcc cc.c -DLINUXBUILD -o ../bin/cc
echo cc0
gcc cc0.c -DLINUXBUILD -o ../bin/cc0
echo cc1
gcc cc1.c -DLINUXBUILD -o ../bin/cc1
echo cc2
gcc cc2.c -DLINUXBUILD -o ../bin/cc2
echo ld
gcc ld.c -DLINUXBUILD -o ../bin/ld
echo reloc
gcc reloc.c -DLINUXBUILD -o ../bin/reloc
echo copt
gcc copt.c -DLINUXBUILD -o ../bin/copt
echo gfx2sgx
gcc gfx2sgx.c -lm -DLINUXBUILD -o ../bin/gfx2sgx
echo sortmap
gcc sortmap.c -DLINUXBUILD -o ../bin/sortmap
