#!/usr/bin/sh
set -eux
./vm/as2.py ./vm/programs/${1} ./vm/out.bin
xxd -i ./vm/out.bin | sed "s/unsigned/const unsigned/" > firmware/app/vm_program.inc