#!/usr/bin/env python

import os
import argparse

parser = argparse.ArgumentParser(description='Compile & Test.')
parser.add_argument('-c', '--compile', action='store_true', default=False)
parser.add_argument('-d', '--compile-shared', action='store_true', default=False)
parser.add_argument('-t', '--test', nargs=1, type=int)
parser.add_argument('-w', '--no-warnings', action='store_true', default=False)
args = parser.parse_args()

if ((args.compile or args.compile_shared) and args.test):
    print('Error.')
    exit(0)

path = '.'

source_data = [ 'jitfunc', 'jitffi-def', 'jitffi-sysv64', 'jitffi-ms64' ]

sources = ''
for name in source_data:
    sources += '"%s/source/%s.cpp" ' % (path, name)

include = '"%s/include"' % path

command = 'g++ -std=c++14 -O2'
if (args.no_warnings):
    command += ' -w'

if (args.test):
    testsource = '"%s/testsuite/test%d.cpp"' % (path, args.test[0])
    os.system('%s -I%s %s %s -o test && ./test' % (command, include, sources, testsource))
elif (args.compile):
    os.system('%s -I%s %s -c' % (command, include, sources))
    files = ''
    for name in source_data:
        files += '%s.o ' % name
    os.system('ar -r libjitffi.a %s' % files)
elif (args.compile_shared):
    os.system('%s -I%s %s -shared -fPIC -o libjitffi.so' % (command, include, sources))
