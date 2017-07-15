#!/usr/bin/env python2
import fileinput
import re
import zlib

def csum(s):
    return zlib.crc32(s) & 0xffff

def main():
    bracketCount = 0
    lastFunc = 'NONE'
    for line in fileinput.input():
        m = line.split()
        print line[:-1]
        if len(m) > 0 and (m[0] == 'call' or m[0] == 'callq'):
            name = m[1][1:]
            code = csum(m[1][1:])
            print '\t.byte 0xeb, 0x02, ' + hex(code & 255) + ', ' + hex(code>>8) + ' # safecall(' + name + ')'

main()