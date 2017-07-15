#!/usr/bin/env python2
import fileinput
import re
import zlib

dQuoteRe = re.compile(r'[^"]*"')
sQuoteRe = re.compile(r"'[^']*'")
commentRe = re.compile(r"#.*$")
oBracketRe = re.compile(r"{")
cBracketRe = re.compile(r"}")

returnRe = re.compile(r"([^a-zA-Z_])SAFE_RETURN\(([^)]*)\)")

# TODO: This is a poc but to make it work you'd need to support functions like this:
# int
# main
# (...
functionRe = re.compile(r"([a-zA-Z0-9_]+) *\(")

def csum(s):
    return hex(zlib.crc32(s) & 0xffff)

def main():
    bracketCount = 0
    lastFunc = 'NONE'
    def returnReplace(matchobj):
        return matchobj.group(1) + 'returnCheck(' + csum(lastFunc) + ');return(' + matchobj.group(2) + ')'
    for line in fileinput.input():
        cleaned = commentRe.sub("", line)
        cleaned = dQuoteRe.sub("", cleaned)
        cleaned = sQuoteRe.sub("", cleaned)
        bracketCount += len(oBracketRe.findall(cleaned))
        bracketCount -= len(cBracketRe.findall(cleaned))
        if bracketCount == 0:
            m = functionRe.search(cleaned)
            if m:
                lastFunc = m.group(1)
                #print "# begin function %s" % m.group(1)
        line = returnRe.sub(returnReplace, line)
        print line[:-1] ## + ' # ' + str(bracketCount)

main()