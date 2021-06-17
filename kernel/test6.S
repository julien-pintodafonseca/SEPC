.text
.globl __proc6_1
__proc6_1:
    movl $3,%eax
    ret
.globl __proc6_2
__proc6_2:
    movl 4(%esp),%eax
    pushl %eax
    popl %eax
    ret
.previous