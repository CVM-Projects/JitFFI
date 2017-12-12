# X86-64 Register Usage

## Preserved across function calls

Y : Yes

N : No

number : passing

x : returning

Register|SysV64|MS64
--------|------|----
%rax|N(x)|N(x)
%rcx|N(4)|N(1)
%rdx|N(3 x)|N(2)
%rbx|Y|Y
%rsp|Y|Y
%rbp|Y|Y
%rsi|N(2)|Y
%rdi|N(1)|Y
%r8 |N(5)|N(3)
%r9 |N(6)|N(4)
%r10|N|N
%r11|N|N
%r12|Y|Y
%r13|Y|Y
%r14|Y|Y
%r15|Y|Y
