#!/usr/bin/env python3

from sys import argv

bin = argv[1]
img_hex = argv[2]

assert bin != ''
assert img_hex != ''

fp_img = open(img_hex, 'w')

with open(bin, 'rb') as fp:
    while True:
        bytes = fp.read(4)
        if not bytes:
            break
        fp_img.write(bytes[::-1].hex() + ' ')
    fp.close()

fp_img.close()
