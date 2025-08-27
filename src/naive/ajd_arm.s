    .section    __TEXT,__text,regular,pure_instructions
    .global _naive
    .p2align    2
_naive:
    sub sp, sp, #80
    str x0, [sp, #72]   ; m
	str	x1, [sp, #64]   ; n
	str	x2, [sp, #56]   ; p
	str	x3, [sp, #48]   ; A
	str	x4, [sp, #40]   ; B
	str	x5, [sp, #32]   ; C
    b outer_loop_init

outer_loop_init:
    str xzr, [sp, #24]  ; i=0
    b outer_loop_start

outer_loop_start:
    ldr x8, [sp, #24]   ; i
    ldr x9, [sp, #72]   ; m
    subs x8, x8, x9     ; i-m
    b.hs onexit         ; higher-same >= 0
    b middle_init

middle_init:
    str xzr, [sp, #16]  ; j
    b middle_loop_start

middle_loop_start:
    ldr x8, [sp, #16]   ; j
    ldr x9, [sp, #56]   ; p
    subs x8, x8, x9     ; j - p
    b.hs outer_loop_incr ; higher-same >= 0
    b inner_init

inner_init:
    str xzr, [sp, #8]      ; k=0

inner_loop_start:
    ldr x8, [sp, #8]    ; k
    ldr x9, [sp, #64]   ; n
    subs x8, x8, x9     ; k - p
    b.hs middle_loop_incr ; higher-same >= 0
    b mult_accumulate

; c[i * p + j] += a[i * n + k] * b[k * p + j];
mult_accumulate:
; Start with a[i * n + k]
	ldr	x8, [sp, #48]
	ldr	x9, [sp, #24]
	ldr	x10, [sp, #64]
	mul	x9, x9, x10
	ldr	x10, [sp, #8]
	add	x9, x9, x10
	ldr	d0, [x8, x9, lsl #3]    ; shift left by 3 to get *8 for double

; Then b[k * p + j]
	ldr	x8, [sp, #40]   ; B
	ldr	x9, [sp, #8]    ; k
	ldr	x10, [sp, #56]  ; p
	mul	x9, x9, x10
	ldr	x10, [sp, #16]  ; j
	add	x9, x9, x10
	ldr	d1, [x8, x9, lsl #3]

; It's += so get c
	ldr	x8, [sp, #32]   ; C
	ldr	x9, [sp, #24]   ; i
	ldr	x10, [sp, #56]  ; p
	mul	x9, x9, x10
	ldr	x10, [sp, #16]  ; j
	add	x9, x9, x10
	ldr	d2, [x8, x9, lsl #3]

    fmadd   d0, d0, d1, d2
    str d0, [x8, x9, lsl #3]
    b inner_loop_incr

inner_loop_incr:
    ldr x8, [sp, #8]    ; k++
    add x8, x8, #1
    str x8, [sp, #8]
    b inner_loop_start

middle_loop_incr:
    ldr x8, [sp, #16]   ; j++
    add x8, x8, #1
    str x8, [sp, #16]
    b middle_loop_start

outer_loop_incr:
    ldr x8, [sp, #24]   ; i++
    add x8, x8, #1
    str x8, [sp, #24]
    b outer_loop_start

onexit:
    add sp, sp, #80
    ret
