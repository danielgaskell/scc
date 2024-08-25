#!/bin/bash
/opt/fcc/bin/lorderz80 `ls *.o` | tsort > libc.fil
