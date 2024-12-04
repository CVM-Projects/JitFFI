from keystone import *

ks = Ks(KS_ARCH_ARM64, KS_MODE_LITTLE_ENDIAN)

for dst in range(31):
    for src in range(31):
        assembly_code = f"str w{src}, [x{dst}]"
        encoding, count = ks.asm(assembly_code)
        code = int.from_bytes(bytes(encoding), byteorder='little', signed=False)
        print(assembly_code, '\n', hex(code), hex(0xf9000000 | (src & 0x1f) | ((dst & 0x1f) << 5)), hex(code & 0x1f), hex(((code >> 5) & 0x1f)))

