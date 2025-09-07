/* The issue is that your macro parameters dest and src are C variables, so the compiler loads them from memory each time.
	#NO_APP
	movl	%eax, -52(%rbp)
	movl	-52(%rbp), %eax
	movl	-56(%rbp), %ecx
	#APP
	imull	%ecx, %eax
*/
#define MULTIPLY(dest, src) \
    __asm__ volatile( \
        "imul %[rsrc], %[rdest]\n" \
        : [rdest] "+r"(dest) \
        : [rsrc] "r" (src) \
    );
