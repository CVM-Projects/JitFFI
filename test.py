#!/usr/bin/env python

import sys
import os

if (sys.argv[1]):
	n = int(sys.argv[1])
else:
	n = 1

path = '.'

sources = ''
sources += '"%s/source/jitffi.cpp" ' % path
sources += '"%s/source/jitffi-sysv64.cpp" ' % path
sources += '"%s/source/jitffi-ms64.cpp" ' % path

os.system('g++ "%s/testsuite/test%d.cpp" -std=c++14 -O2 -o jitffi -I"%s/include" %s && ./jitffi' % (path, n, path, sources))
