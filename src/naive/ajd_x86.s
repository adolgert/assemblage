    .text
    .file "gemm_asm.s"
    .globl byhand
    .p2align 4, 0x90
byhand:
    pushq   %rbp            # Save callers frame pointer to top of stack.
    movq    %rsp, %rbp      # Set the frame (base) pointer to the current stack top.
    subq    $80, %rsp       # Make space for 80 bytes on the stack (estimated)
    movq    %rdi, -8(%rbp)  # m: int
    movq    %rsi, -16(%rbp) # n: int
    movq    %rdx, -24(%rbp) # k: int
    movq    %rcx, -32(%rbp) # A
    movq    %r8, -40(%rbp)  # B
    movq    %r9, -48(%rbp)  # C

    movq	$0, -56(%rbp)   # Loop counter i set to 0.
outer_loop:
    movq    -56(%rbp), %rax # Use of %rax is conventional.
    cmpq    -8(%rbp), %rax  # Compare with m.
    jae     exit_gemm

    movq    $0, -64(%rbp)  # Loop counter set to 0.
middle_loop:
    movq    -64(%rbp), %rax
    cmpq    -24(%rbp), %rax
    jae     outer_increment

    movq    %0, -72(%rbp)  # Loop counter set to 0.
inner_loop:
    movq    -72(%rbp), %rax
    cmpq    -16(%rbp), %rax
    jae     middle_increment



middle_increment:
    movq    -64(%rbp), %rax
    addq    $1, %rax
    movq    %rax, -64(%rbp)
    jmp     middle_loop

outer_increment:
    movq    -56(%rbp), %rax
    addq    $1, %rax        # i++
    movq    %rax, -56(%rbp)
    jmp     outer_loop

exit_gemm:
    addq    $80, %rsp       # Clean up stack space
    popq    %rbp            # Get rid of the base.
    retq
