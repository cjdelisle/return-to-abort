#!/usr/bin/env python2
import fileinput
import re
import zlib

def csum(s):
    return zlib.crc32(s) & 0xffff

def main():
    leadingUnderscore = False
    lines = []
    for line in fileinput.input():
        lines.append(line)
        m = line.split()
        if len(m) > 0 and (m[0] == 'call' or m[0] == 'callq'):
            if m[1] in [ '_printf', '___stack_chk_fail', '_execve', 'puts', '_read']:
                leadingUnderscore = True
    for line in lines:
        m = line.split()
        print line[:-1]
        if len(m) > 0 and (m[0] == 'call' or m[0] == 'callq'):
            name = m[1]
            if leadingUnderscore: name = name[1:]
            code = csum(name)
            print '\t.byte 0xeb, 0x02, ' + hex(code & 255) + ', ' + hex(code>>8) + ' # safecall(' + name + ')'

main()