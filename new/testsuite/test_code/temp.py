from keystone import *

ks = Ks(KS_ARCH_ARM64, KS_MODE_LITTLE_ENDIAN)

# for dst in range(31):
    # for src in range(31):

def f(assembly_code):
    encoding, count = ks.asm(assembly_code)
    code = int.from_bytes(bytes(encoding), byteorder='little', signed=False)
    print(assembly_code, '\n', hex(code), f'0b{code:032b}')
    return code

for dst in range(31):
    for src in range(31):
        s0 = f(f'fmov x{dst}, d{src}')
        s1 = f(f'fmov d{dst}, x{src}')
        s2 = f(f'fmov d{dst}, d{src}')
        t0 = (0x9e660000 | src << 5 | dst)
        t1 = (0x9e670000 | src << 5 | dst)
        t2 = (0x1e604000 | src << 5 | dst)
        assert s0 == t0 and s1 == t1 and s2 == t2

a = f(f'fmov d0, d0')
b = f(f'fmov d0, d1')

print(hex(b - a))
#
# a = f(f'fmov x0, d0')
# b = f(f'fmov x1, d0')
#
# print(hex(b - a))

#
# a = f(f'fmov d0, #0')
# a = f(f'fmov d0, #0.0')
# b = f(f'fmov d0, #1')
# b = f(f'fmov d0, #1.0')
# b = f(f'fmov d0, #2')
# b = f(f'fmov d0, #3')
# print(hex(b - a))
