from keystone import *

ks = Ks(KS_ARCH_ARM64, KS_MODE_LITTLE_ENDIAN)

for i in range(32):
    assembly_code = f"mov w{i}, #0x1234"
    encoding, count = ks.asm(assembly_code)
    code = int.from_bytes(bytes(encoding), byteorder='little', signed=False)
    print(assembly_code, '\n', hex(code), hex(code >> 5 >> 16), bin(code >> 5 >> 16), bin(code))

