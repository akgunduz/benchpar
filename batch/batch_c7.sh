#!/bin/bash
Client/benchtool_c7 -a m -m 2 -o spr -r 10 1
Client/benchtool_c7 -a m -m 2 -o spr -r 5 2
Client/benchtool_c7 -a m -m 2 -o spr -r 3 3
Client/benchtool_c7 -a c -m 1 -o spr -r 100 1
Client/benchtool_c7 -a c -m 1 -o spr -r 100 2
Client/benchtool_c7 -a c -m 1 -o spr -r 100 3
Client/benchtool_c7 -a c -m 1 -o spr -r 100 4
Client/benchtool_c7 -a s -m 0 -o spr -r 10000 1
Client/benchtool_c7 -a s -m 0 -o spr -r 10000 2
Client/benchtool_c7 -a s -m 0 -o spr -r 10000 3