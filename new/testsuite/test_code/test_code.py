from pathlib import Path
import tempfile
import subprocess
import shutil
import sys

from keystone import *

dir_jitcode_impl = Path(__file__).absolute().parent.parent.parent / 'jitcode_impl'
file_x86_64_header = dir_jitcode_impl / 'x86_64.h'
file_x86_64_source = dir_jitcode_impl / 'x86_64.c'

TestCodeTemplate = '''#include "$file_x86_64_header"
#include <assert.h>
#include <stdio.h>

#define dump(code, length, prefix) do {\\
    printf("- %s", prefix);\\
    for (int i = 0; i < length; i++) {\\
        if (i != 0) {\\
            printf(" ");\\
        }\\
        printf("%02x", code[i]);\\
    }\\
    printf("\\n");\\
} while (0)

#define compare(op_str, op, ref_length) do {\\
    printf("%s - %s", op_str, #op);\\
    uint8_t data[16] = {};\\
    int pass = 1;\\
    size_t length = op;\\
    if (length != ref_length) {\\
        pass = 0;\\
    }\\
    for (int i = 0; i < 16; i++) {\\
        if (data[i] != ref_data[i]) {\\
            pass = 0;\\
        }\\
    }\\
    if (pass) {\\
        printf(" [pass]\\n");\\
    } else {\\
        error_count++;\\
        printf(" [failed]\\n");\\
        dump(ref_data, ref_length, "excepted: ");\\
        dump(data,     length,     "given:    ");\\
    }\\
} while (0)

int main() {
    int error_count = 0;
    $test_code
    printf("%d error(s)\\n", error_count);
    return error_count;
}
'''

r64 = ('rax', 'rcx', 'rdx', 'rbx', 'rsp', 'rbp', 'rsi', 'rdi', 'r8', 'r9', 'r10', 'r11', 'r12', 'r13', 'r14', 'r15')
r32 = ('eax', 'ecx', 'edx', 'ebx', 'esp', 'ebp', 'esi', 'edi', 'r8d', 'r9d', 'r10d', 'r11d', 'r12d', 'r13d', 'r14d', 'r15d')
r16 = ('ax', 'cx', 'dx', 'bx', 'sp', 'bp', 'si', 'di', 'r8w', 'r9w', 'r10w', 'r11w', 'r12w', 'r13w', 'r14w', 'r15w')
r8 = ('al', 'cl', 'dl', 'bl', 'spl', 'bpl', 'sil', 'dil', 'r8b', 'r9b', 'r10b', 'r11b', 'r12b', 'r13b', 'r14b', 'r15b')
xmm = ('xmm0', 'xmm1', 'xmm2', 'xmm3', 'xmm4', 'xmm5', 'xmm6', 'xmm7', 'xmm8', 'xmm9', 'xmm10', 'xmm11', 'xmm12', 'xmm13', 'xmm14', 'xmm15')

all_r = r64 + r32 + r16 + r8

OneCodeTemplate = '''{ uint8_t ref_data[16] = {$ref_data}; compare("$code", $func(data, $args), $ref_length); }'''


ks = Ks(KS_ARCH_X86, KS_MODE_64)


def _get_one_code(code, func, *args) -> str:
    encoding, _ = ks.asm(code)
    ref_data = ', '.join(map(lambda c: f'0x{c:02x}', encoding))
    return OneCodeTemplate.replace('$code', code).replace('$ref_data', ref_data).replace('$ref_length', str(len(encoding))).replace('$func', func).replace('$args', ', '.join(args))


def get_r_bits(r) -> int:
    if r in xmm:
        return 128
    if r in r64:
        return 64
    elif r in r32:
        return 32
    elif r in r16:
        return 16
    elif r in r8:
        return 8


def get_jitcode_reg(r) -> str:
    bits = get_r_bits(r)
    return f'r{bits}_x86_64_{r}'


def append_mov1(one_codes):
    # Add one codes
    # %r = imm
    for r in r64:
        one_codes.append(_get_one_code(f'mov {r}, 0x123456789abcdef', 'jitcode_mov_r64_imm64_x86_64', get_jitcode_reg(r), '0x123456789abcdef'))
    for r in r32:
        one_codes.append(_get_one_code(f'mov {r}, 0x12345678', 'jitcode_mov_r32_imm32_x86_64', get_jitcode_reg(r), '0x12345678'))
    for r in r16:
        one_codes.append(_get_one_code(f'mov {r}, 0x1234', 'jitcode_mov_r16_imm16_x86_64', get_jitcode_reg(r), '0x1234'))
    for r in r8:
        one_codes.append(_get_one_code(f'mov {r}, 0x12', 'jitcode_mov_r8_imm8_x86_64', get_jitcode_reg(r), '0x12'))

def append_mov2(one_codes):
    # %r1 = %r2
    for r2 in all_r:
        for r1 in all_r:
            if get_r_bits(r1) == get_r_bits(r2):
                one_codes.append(_get_one_code(f'mov {r1}, {r2}', 'jitcode_mov_r1_r2_x86_64', get_jitcode_reg(r1), get_jitcode_reg(r2)))

def append_mov3(one_codes):
    # *%r1 = %r2
    for r2 in all_r:
        for r1 in all_r:
            if get_r_bits(r1) in (64, 32):
                one_codes.append(_get_one_code(f'mov [{r1}], {r2}', 'jitcode_mov_pr1_r2_x86_64', get_jitcode_reg(r1), get_jitcode_reg(r2)))

def append_mov4(one_codes):
    # %r1 = *%r2
    for r2 in all_r:
        for r1 in all_r:
            if get_r_bits(r2) in (64, 32):
                one_codes.append(_get_one_code(f'mov {r1}, [{r2}]', 'jitcode_mov_r1_pr2_x86_64', get_jitcode_reg(r1), get_jitcode_reg(r2)))

def append_mov5(one_codes):
    # %r1 = %r2, one is xmm
    for r2 in r64 + r32:
        for r1 in xmm:
            one_codes.append(_get_one_code(f'mov{"q" if r2 in r64 else "d"} {r1}, {r2}', 'jitcode_mov_r1_r2_x86_64', get_jitcode_reg(r1), get_jitcode_reg(r2)))
    for r2 in xmm:
        for r1 in r64 + r32:
            one_codes.append(_get_one_code(f'mov{"q" if r1 in r64 else "d"} {r1}, {r2}', 'jitcode_mov_r1_r2_x86_64', get_jitcode_reg(r1), get_jitcode_reg(r2)))


def do_test(one_codes):
    test_code = '\n'.join(one_codes)
    with tempfile.NamedTemporaryFile(suffix='.c', delete=True) as f:
        test_code = TestCodeTemplate.replace('$file_x86_64_header', str(file_x86_64_header)).replace('$test_code', test_code)
        f.write(test_code.encode())
        f.flush()
        print('Compiling...')
        sys.stdout.flush()
        subprocess.check_call([shutil.which('cc'), '-g', f.name, '-o', f.name + '.out', file_x86_64_source])
        print('Testing...')
        sys.stdout.flush()
        sp = subprocess.run([f.name + '.out'], stdout=subprocess.PIPE)
        if sp.returncode != 0:
            print(sp.stdout.decode())
            raise RuntimeError


def gen(f):
    one_codes = []
    f(one_codes)
    return one_codes

do_test(gen(append_mov1))
do_test(gen(append_mov2))
do_test(gen(append_mov3))
do_test(gen(append_mov4))
do_test(gen(append_mov5))
