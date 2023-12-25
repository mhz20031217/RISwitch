#!/usr/bin/env python3

from sys import argv

bin = argv[1]
img_hex = argv[2]
img_coe = argv[3]

assert bin != ''
assert img_hex != ''
assert img_coe != ''

fp_img = open(img_hex, 'w')
fp_img.write('@00000000\n')

fp_coe = open(img_coe, 'w')
fp_coe.write("""memory_initialization_radix=16;
memory_initialization_vector=\n""")

with open(bin, 'rb') as fp:
    while True:
        bytes = fp.read(4)
        if not bytes:
            break
        fp_img.write(bytes[::-1].hex() + '\n')
        fp_coe.write(bytes[::-1].hex() + ',')
    fp_coe.write(';\n')
    fp.close()

fp_img.close()
fp_coe.close()
