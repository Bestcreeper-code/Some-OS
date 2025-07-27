	.file	"memory.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.file 0 "C:/Users/odayl/Desktop/Codetests/asm" "src/memory.c"
.lcomm free_regions,2048,32
	.globl	free_region_count
	.bss
	.align 4
free_region_count:
	.space 4
	.section .rdata,"dr"
	.align 8
.LC0:
	.ascii "mmap->len: %u | free_regions[%d].length: %u\12\0"
	.text
	.globl	parse_memory_map
	.def	parse_memory_map;	.scl	2;	.type	32;	.endef
	.seh_proc	parse_memory_map
parse_memory_map:
.LFB2:
	.file 1 "src/memory.c"
	.loc 1 11 50
	.cfi_startproc
	pushq	%rbp
	.seh_pushreg	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	.loc 1 12 23
	movl	$0, free_region_count(%rip)
	.loc 1 17 18
	movq	16(%rbp), %rax
	movl	(%rax), %eax
	.loc 1 17 26
	andl	$64, %eax
	.loc 1 17 8
	testl	%eax, %eax
	je	.L7
	.loc 1 21 33
	movq	16(%rbp), %rax
	movl	48(%rax), %edx
	.loc 1 21 54
	movq	16(%rbp), %rax
	movl	44(%rax), %eax
	.loc 1 21 45
	addl	%edx, %eax
	.loc 1 21 15
	movl	%eax, %eax
	movq	%rax, -16(%rbp)
	.loc 1 22 68
	movq	16(%rbp), %rax
	movl	48(%rax), %eax
	movl	%eax, %eax
	.loc 1 22 29
	movq	%rax, -8(%rbp)
	.loc 1 25 11
	jmp	.L4
.L6:
	.loc 1 27 17
	movq	-8(%rbp), %rax
	movl	20(%rax), %eax
	.loc 1 27 12
	cmpl	$1, %eax
	jne	.L5
	.loc 1 28 35
	movl	free_region_count(%rip), %eax
	.loc 1 28 16
	cmpl	$127, %eax
	jg	.L5
	.loc 1 30 48
	movl	free_region_count(%rip), %edx
	.loc 1 30 65
	movq	-8(%rbp), %rax
	movq	4(%rax), %rax
	.loc 1 30 59
	movslq	%edx, %rdx
	movq	%rdx, %rcx
	salq	$4, %rcx
	leaq	free_regions(%rip), %rdx
	movq	%rax, (%rcx,%rdx)
	.loc 1 31 48
	movl	free_region_count(%rip), %edx
	.loc 1 31 62
	movq	-8(%rbp), %rax
	movq	12(%rax), %rax
	.loc 1 31 56
	movslq	%edx, %rdx
	movq	%rdx, %rcx
	salq	$4, %rcx
	leaq	8+free_regions(%rip), %rdx
	movq	%rax, (%rcx,%rdx)
	.loc 1 32 17
	movl	free_region_count(%rip), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rcx
	movl	free_region_count(%rip), %edx
	movq	-8(%rbp), %rax
	movq	12(%rax), %rax
	movq	%rcx, %r9
	movl	%edx, %r8d
	movq	%rax, %rdx
	leaq	.LC0(%rip), %rax
	movq	%rax, %rcx
	call	printf
	.loc 1 34 34
	movl	free_region_count(%rip), %eax
	addl	$1, %eax
	movl	%eax, free_region_count(%rip)
.L5:
	.loc 1 39 64
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edx
	.loc 1 39 42
	movq	-8(%rbp), %rax
	.loc 1 39 58
	addq	%rdx, %rax
	.loc 1 39 71
	addq	$4, %rax
	.loc 1 39 14
	movq	%rax, -8(%rbp)
.L4:
	.loc 1 25 12
	movq	-8(%rbp), %rax
	.loc 1 25 28
	cmpq	-16(%rbp), %rax
	jb	.L6
	jmp	.L1
.L7:
	.loc 1 18 9
	nop
.L1:
	.loc 1 41 1
	addq	$48, %rsp
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.seh_endproc
	.globl	FirstRegionOfSizeOrMore
	.def	FirstRegionOfSizeOrMore;	.scl	2;	.type	32;	.endef
	.seh_proc	FirstRegionOfSizeOrMore
FirstRegionOfSizeOrMore:
.LFB3:
	.loc 1 43 53
	.cfi_startproc
	pushq	%rbp
	.seh_pushreg	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushq	%rdi
	.seh_pushreg	%rdi
	.cfi_def_cfa_offset 24
	.cfi_offset 5, -24
	subq	$216, %rsp
	.seh_stackalloc	216
	.cfi_def_cfa_offset 240
	leaq	208(%rsp), %rbp
	.seh_setframe	%rbp, 208
	.cfi_def_cfa 6, 32
	.seh_endprologue
	movq	%rcx, 32(%rbp)
	.loc 1 44 14
	movq	32(%rbp), %rax
	addq	$8, %rax
	movq	%rax, -40(%rbp)
.LBB2:
	.loc 1 46 14
	movl	$0, -4(%rbp)
	.loc 1 46 5
	jmp	.L9
.L24:
.LBB3:
	.loc 1 47 28
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 47 12
	testq	%rax, %rax
	je	.L25
	.loc 1 49 18
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	movq	%rax, -48(%rbp)
	.loc 1 50 18
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	movq	%rax, -16(%rbp)
	.loc 1 52 14
	leaq	-176(%rbp), %rdx
	movl	$0, %eax
	movl	$16, %ecx
	movq	%rdx, %rdi
	rep stosq
	.loc 1 53 19
	movl	-4(%rbp), %eax
	cltq
	movb	$1, -176(%rbp,%rax)
.L17:
	.loc 1 57 21
	movb	$0, -17(%rbp)
.LBB4:
	.loc 1 58 22
	movl	$0, -24(%rbp)
	.loc 1 58 13
	jmp	.L12
.L16:
	.loc 1 59 27
	movl	-24(%rbp), %eax
	cltq
	movzbl	-176(%rbp,%rax), %eax
	.loc 1 59 20
	testb	%al, %al
	jne	.L26
	.loc 1 59 49 discriminator 2
	movl	-24(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 59 31 discriminator 2
	testq	%rax, %rax
	je	.L26
	.loc 1 61 36
	movl	-24(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rdx
	.loc 1 61 59
	movq	-48(%rbp), %rcx
	movq	-16(%rbp), %rax
	addq	%rcx, %rax
	.loc 1 61 20
	cmpq	%rax, %rdx
	jne	.L15
	.loc 1 62 48
	movl	-24(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 62 30
	addq	%rax, -16(%rbp)
	.loc 1 63 31
	movl	-24(%rbp), %eax
	cltq
	movb	$1, -176(%rbp,%rax)
	.loc 1 64 29
	movb	$1, -17(%rbp)
	jmp	.L15
.L26:
	.loc 1 59 63
	nop
.L15:
	.loc 1 58 53 discriminator 2
	addl	$1, -24(%rbp)
.L12:
	.loc 1 58 31 discriminator 1
	movl	free_region_count(%rip), %eax
	cmpl	%eax, -24(%rbp)
	jl	.L16
.LBE4:
	.loc 1 67 18
	cmpb	$0, -17(%rbp)
	jne	.L17
	.loc 1 69 12
	movq	-16(%rbp), %rax
	cmpq	-40(%rbp), %rax
	jb	.L23
.LBB5:
	.loc 1 70 22
	movl	$0, -28(%rbp)
	.loc 1 70 13
	jmp	.L19
.L21:
	.loc 1 71 27
	movl	-28(%rbp), %eax
	cltq
	movzbl	-176(%rbp,%rax), %eax
	.loc 1 71 20
	testb	%al, %al
	je	.L20
	.loc 1 71 31 discriminator 1
	movl	-28(%rbp), %eax
	cmpl	-4(%rbp), %eax
	je	.L20
	.loc 1 72 28
	movl	-28(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	addq	%rdx, %rax
	.loc 1 72 21
	movl	$16, %r8d
	movl	$0, %edx
	movq	%rax, %rcx
	call	memset
.L20:
	.loc 1 70 53 discriminator 2
	addl	$1, -28(%rbp)
.L19:
	.loc 1 70 31 discriminator 1
	movl	free_region_count(%rip), %eax
	cmpl	%eax, -28(%rbp)
	jl	.L21
.LBE5:
	.loc 1 76 36
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	8+free_regions(%rip), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, (%rcx,%rdx)
	.loc 1 77 20
	movl	-4(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	addq	%rdx, %rax
	jmp	.L22
.L25:
	.loc 1 47 42
	nop
.L23:
.LBE3:
	.loc 1 46 45 discriminator 2
	addl	$1, -4(%rbp)
.L9:
	.loc 1 46 23 discriminator 1
	movl	free_region_count(%rip), %eax
	cmpl	%eax, -4(%rbp)
	jl	.L24
.LBE2:
	.loc 1 81 12
	movl	$0, %eax
.L22:
	.loc 1 82 1
	addq	$216, %rsp
	popq	%rdi
	.cfi_restore 5
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, -200
	ret
	.cfi_endproc
.LFE3:
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC1:
	.ascii "Couldn't find enough memory for %u bytes\12\0"
.LC2:
	.ascii "Allocated %u bytes at %p\12\0"
	.text
	.globl	malloc
	.def	malloc;	.scl	2;	.type	32;	.endef
	.seh_proc	malloc
malloc:
.LFB4:
	.loc 1 84 28
	.cfi_startproc
	pushq	%rbp
	.seh_pushreg	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	.loc 1 85 29
	movq	16(%rbp), %rax
	movq	%rax, %rcx
	call	FirstRegionOfSizeOrMore
	movq	%rax, -8(%rbp)
	.loc 1 86 8
	cmpq	$0, -8(%rbp)
	je	.L28
	.loc 1 86 33 discriminator 1
	movq	-8(%rbp), %rax
	movq	8(%rax), %rdx
	.loc 1 86 50 discriminator 1
	movq	16(%rbp), %rax
	addq	$8, %rax
	.loc 1 86 24 discriminator 1
	cmpq	%rax, %rdx
	jb	.L28
	.loc 1 86 78 discriminator 2
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 86 69 discriminator 2
	testq	%rax, %rax
	jne	.L29
.L28:
	.loc 1 87 9
	movq	16(%rbp), %rax
	movq	%rax, %rdx
	leaq	.LC1(%rip), %rax
	movq	%rax, %rcx
	call	printf
	.loc 1 88 16
	movl	$0, %eax
	jmp	.L30
.L29:
	.loc 1 91 51
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 91 15
	movq	%rax, -16(%rbp)
	.loc 1 92 13
	movq	-16(%rbp), %rax
	movq	16(%rbp), %rdx
	movq	%rdx, (%rax)
	.loc 1 93 11
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 93 20
	subq	16(%rbp), %rax
	leaq	-8(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 8(%rax)
	.loc 1 94 11
	movq	-8(%rbp), %rax
	movq	(%rax), %rdx
	.loc 1 94 23
	movq	16(%rbp), %rax
	addq	%rdx, %rax
	leaq	8(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	.loc 1 96 75
	movq	-16(%rbp), %rax
	leaq	8(%rax), %rdx
	.loc 1 96 5
	movq	16(%rbp), %rax
	addq	$8, %rax
	movq	%rdx, %r8
	movq	%rax, %rdx
	leaq	.LC2(%rip), %rax
	movq	%rax, %rcx
	call	printf
	.loc 1 97 12
	movq	-16(%rbp), %rax
	addq	$8, %rax
.L30:
	.loc 1 98 1
	addq	$48, %rsp
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.seh_endproc
	.globl	free
	.def	free;	.scl	2;	.type	32;	.endef
	.seh_proc	free
free:
.LFB5:
	.loc 1 100 26
	.cfi_startproc
	pushq	%rbp
	.seh_pushreg	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	.cfi_def_cfa_register 6
	subq	$80, %rsp
	.seh_stackalloc	80
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	.loc 1 101 15
	movq	16(%rbp), %rax
	movq	%rax, -40(%rbp)
	.loc 1 102 45
	movq	-40(%rbp), %rax
	subq	$8, %rax
	.loc 1 102 15
	movq	%rax, -48(%rbp)
	.loc 1 103 14
	movq	-48(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, -56(%rbp)
	.loc 1 106 9
	movl	$-1, -4(%rbp)
.LBB6:
	.loc 1 107 14
	movl	$0, -8(%rbp)
	.loc 1 107 5
	jmp	.L32
.L35:
	.loc 1 108 28
	movl	-8(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 108 12
	testq	%rax, %rax
	jne	.L33
	.loc 1 108 62 discriminator 1
	movl	-8(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 108 44 discriminator 1
	testq	%rax, %rax
	jne	.L33
	.loc 1 109 39
	movl	-8(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	free_regions(%rip), %rdx
	movq	-40(%rbp), %rax
	movq	%rax, (%rcx,%rdx)
	.loc 1 110 36
	movl	-8(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	8+free_regions(%rip), %rdx
	movq	-56(%rbp), %rax
	movq	%rax, (%rcx,%rdx)
	.loc 1 111 28
	movl	-8(%rbp), %eax
	movl	%eax, -4(%rbp)
	.loc 1 112 13
	jmp	.L34
.L33:
	.loc 1 107 44 discriminator 2
	addl	$1, -8(%rbp)
.L32:
	.loc 1 107 23 discriminator 1
	cmpl	$127, -8(%rbp)
	jle	.L35
.L34:
.LBE6:
	.loc 1 116 8
	cmpl	$-1, -4(%rbp)
	je	.L53
.LBB7:
	.loc 1 121 14
	movl	$0, -12(%rbp)
	.loc 1 121 5
	jmp	.L38
.L42:
.LBB8:
	.loc 1 122 18
	movl	-12(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -16(%rbp)
	.loc 1 122 9
	jmp	.L39
.L41:
	.loc 1 123 32
	movl	-16(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 123 16
	testq	%rax, %rax
	je	.L40
	.loc 1 123 66 discriminator 1
	movl	-12(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 123 48 discriminator 1
	testq	%rax, %rax
	je	.L40
	.loc 1 124 32
	movl	-16(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rdx
	.loc 1 124 60
	movl	-12(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	free_regions(%rip), %rax
	movq	(%rcx,%rax), %rax
	.loc 1 123 82 discriminator 2
	cmpq	%rax, %rdx
	jnb	.L40
.LBB9:
	.loc 1 126 26
	movl	-12(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	movq	%rax, -72(%rbp)
	.loc 1 127 26
	movl	-12(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	movq	%rax, -80(%rbp)
	.loc 1 129 60
	movl	-16(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 129 43
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	movq	%rdx, %rcx
	salq	$4, %rcx
	leaq	free_regions(%rip), %rdx
	movq	%rax, (%rcx,%rdx)
	.loc 1 130 57
	movl	-16(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 130 40
	movl	-12(%rbp), %edx
	movslq	%edx, %rdx
	movq	%rdx, %rcx
	salq	$4, %rcx
	leaq	8+free_regions(%rip), %rdx
	movq	%rax, (%rcx,%rdx)
	.loc 1 132 43
	movl	-16(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	free_regions(%rip), %rdx
	movq	-72(%rbp), %rax
	movq	%rax, (%rcx,%rdx)
	.loc 1 133 40
	movl	-16(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	8+free_regions(%rip), %rdx
	movq	-80(%rbp), %rax
	movq	%rax, (%rcx,%rdx)
.L40:
.LBE9:
	.loc 1 122 52 discriminator 2
	addl	$1, -16(%rbp)
.L39:
	.loc 1 122 31 discriminator 1
	cmpl	$127, -16(%rbp)
	jle	.L41
.LBE8:
	.loc 1 121 48 discriminator 2
	addl	$1, -12(%rbp)
.L38:
	.loc 1 121 23 discriminator 1
	cmpl	$126, -12(%rbp)
	jle	.L42
.LBE7:
.LBB10:
	.loc 1 139 14
	movl	$0, -20(%rbp)
	.loc 1 139 5
	jmp	.L43
.L52:
	.loc 1 140 28
	movl	-20(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 140 12
	testq	%rax, %rax
	je	.L54
.LBB11:
	.loc 1 141 18
	movl	-20(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -24(%rbp)
	.loc 1 141 9
	jmp	.L46
.L51:
.LBB12:
	.loc 1 142 32
	movl	-24(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 142 16
	testq	%rax, %rax
	je	.L55
	.loc 1 144 45
	movl	-20(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rdx
	.loc 1 144 73
	movl	-20(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	8+free_regions(%rip), %rax
	movq	(%rcx,%rax), %rax
	.loc 1 144 22
	addq	%rdx, %rax
	movq	%rax, -64(%rbp)
	.loc 1 145 41
	movl	-24(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	movq	(%rdx,%rax), %rax
	.loc 1 145 16
	cmpq	%rax, -64(%rbp)
	jne	.L48
	.loc 1 146 32
	movl	-20(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	(%rdx,%rax), %rdx
	.loc 1 146 58
	movl	-24(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rcx
	leaq	8+free_regions(%rip), %rax
	movq	(%rcx,%rax), %rax
	.loc 1 146 40
	leaq	(%rdx,%rax), %rcx
	movl	-20(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	8+free_regions(%rip), %rax
	movq	%rcx, (%rdx,%rax)
.LBB13:
	.loc 1 148 26
	movl	-24(%rbp), %eax
	movl	%eax, -28(%rbp)
	.loc 1 148 17
	jmp	.L49
.L50:
	.loc 1 149 54
	movl	-28(%rbp), %eax
	leal	1(%rax), %edx
	.loc 1 149 37
	movl	-28(%rbp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %r8
	leaq	free_regions(%rip), %rcx
	movslq	%edx, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	leaq	free_regions(%rip), %rax
	leaq	(%rdx,%rax), %rdx
	movq	(%rdx), %rax
	movq	8(%rdx), %rdx
	movq	%rax, (%r8,%rcx)
	movq	%rdx, 8(%r8,%rcx)
	.loc 1 148 60 discriminator 3
	addl	$1, -28(%rbp)
.L49:
	.loc 1 148 35 discriminator 1
	cmpl	$126, -28(%rbp)
	jle	.L50
.LBE13:
	.loc 1 151 62
	movq	$0, 2032+free_regions(%rip)
	.loc 1 152 59
	movq	$0, 2040+free_regions(%rip)
	.loc 1 154 18
	subl	$1, -24(%rbp)
	jmp	.L48
.L55:
	.loc 1 142 49
	nop
.L48:
.LBE12:
	.loc 1 141 52 discriminator 2
	addl	$1, -24(%rbp)
.L46:
	.loc 1 141 31 discriminator 1
	cmpl	$127, -24(%rbp)
	jle	.L51
	jmp	.L45
.L54:
.LBE11:
	.loc 1 140 45
	nop
.L45:
	.loc 1 139 48 discriminator 2
	addl	$1, -20(%rbp)
.L43:
	.loc 1 139 23 discriminator 1
	cmpl	$126, -20(%rbp)
	jle	.L52
	jmp	.L31
.L53:
.LBE10:
	.loc 1 117 9
	nop
.L31:
	.loc 1 158 1
	addq	$80, %rsp
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.seh_endproc
.Letext0:
	.file 2 "C:/Users/odayl/AppData/Local/w64devkit/x86_64-w64-mingw32/include/corecrt.h"
	.file 3 "C:/Users/odayl/AppData/Local/w64devkit/x86_64-w64-mingw32/include/stdint.h"
	.file 4 "src/headers/multiboot_info.h"
	.file 5 "src/headers/memory.h"
	.file 6 "src/headers/string.h"
	.file 7 "src/headers/io.h"
	.section	.debug_info,"dr"
.Ldebug_info0:
	.long	0x81c
	.word	0x5
	.byte	0x1
	.byte	0x8
	.secrel32	.Ldebug_abbrev0
	.uleb128 0xf
	.ascii "GNU C17 14.2.0 -mtune=generic -march=x86-64 -g -O0\0"
	.byte	0x1d
	.secrel32	.LASF0
	.secrel32	.LASF1
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.secrel32	.Ldebug_line0
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.ascii "char\0"
	.uleb128 0x10
	.long	0x5d
	.uleb128 0x5
	.ascii "size_t\0"
	.byte	0x2
	.byte	0x23
	.byte	0x2c
	.long	0x79
	.uleb128 0x3
	.byte	0x8
	.byte	0x7
	.ascii "long long unsigned int\0"
	.uleb128 0x3
	.byte	0x8
	.byte	0x5
	.ascii "long long int\0"
	.uleb128 0x5
	.ascii "uintptr_t\0"
	.byte	0x2
	.byte	0x4b
	.byte	0x2c
	.long	0x79
	.uleb128 0x3
	.byte	0x2
	.byte	0x7
	.ascii "short unsigned int\0"
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii "int\0"
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii "long int\0"
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.ascii "unsigned int\0"
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.ascii "long unsigned int\0"
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.ascii "unsigned char\0"
	.uleb128 0x3
	.byte	0x10
	.byte	0x4
	.ascii "long double\0"
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.ascii "signed char\0"
	.uleb128 0x3
	.byte	0x2
	.byte	0x5
	.ascii "short int\0"
	.uleb128 0x5
	.ascii "uint32_t\0"
	.byte	0x3
	.byte	0x28
	.byte	0x14
	.long	0xdf
	.uleb128 0x5
	.ascii "uint64_t\0"
	.byte	0x3
	.byte	0x2a
	.byte	0x30
	.long	0x79
	.uleb128 0x9
	.byte	0x18
	.byte	0x4
	.byte	0x8
	.long	0x19e
	.uleb128 0x1
	.ascii "size\0"
	.byte	0x4
	.byte	0x9
	.long	0x140
	.byte	0
	.uleb128 0x1
	.ascii "addr\0"
	.byte	0x4
	.byte	0xa
	.long	0x151
	.byte	0x4
	.uleb128 0x1
	.ascii "len\0"
	.byte	0x4
	.byte	0xb
	.long	0x151
	.byte	0xc
	.uleb128 0x1
	.ascii "type\0"
	.byte	0x4
	.byte	0xc
	.long	0x140
	.byte	0x14
	.byte	0
	.uleb128 0x5
	.ascii "multiboot_mmap_entry_t\0"
	.byte	0x4
	.byte	0xd
	.byte	0x1b
	.long	0x162
	.uleb128 0x9
	.byte	0x60
	.byte	0x4
	.byte	0x17
	.long	0x372
	.uleb128 0x1
	.ascii "flags\0"
	.byte	0x4
	.byte	0x18
	.long	0x140
	.byte	0
	.uleb128 0x1
	.ascii "mem_lower\0"
	.byte	0x4
	.byte	0x19
	.long	0x140
	.byte	0x4
	.uleb128 0x1
	.ascii "mem_upper\0"
	.byte	0x4
	.byte	0x1a
	.long	0x140
	.byte	0x8
	.uleb128 0x1
	.ascii "boot_device\0"
	.byte	0x4
	.byte	0x1b
	.long	0x140
	.byte	0xc
	.uleb128 0x1
	.ascii "cmdline\0"
	.byte	0x4
	.byte	0x1c
	.long	0x140
	.byte	0x10
	.uleb128 0x1
	.ascii "mods_count\0"
	.byte	0x4
	.byte	0x1d
	.long	0x140
	.byte	0x14
	.uleb128 0x1
	.ascii "mods_addr\0"
	.byte	0x4
	.byte	0x1e
	.long	0x140
	.byte	0x18
	.uleb128 0x1
	.ascii "irrelevant\0"
	.byte	0x4
	.byte	0x20
	.long	0x372
	.byte	0x1c
	.uleb128 0x1
	.ascii "mmap_length\0"
	.byte	0x4
	.byte	0x22
	.long	0x140
	.byte	0x2c
	.uleb128 0x1
	.ascii "mmap_addr\0"
	.byte	0x4
	.byte	0x23
	.long	0x140
	.byte	0x30
	.uleb128 0x1
	.ascii "drives_length\0"
	.byte	0x4
	.byte	0x24
	.long	0x140
	.byte	0x34
	.uleb128 0x1
	.ascii "drives_addr\0"
	.byte	0x4
	.byte	0x25
	.long	0x140
	.byte	0x38
	.uleb128 0x1
	.ascii "config_table\0"
	.byte	0x4
	.byte	0x26
	.long	0x140
	.byte	0x3c
	.uleb128 0x1
	.ascii "boot_loader_name\0"
	.byte	0x4
	.byte	0x27
	.long	0x140
	.byte	0x40
	.uleb128 0x1
	.ascii "apm_table\0"
	.byte	0x4
	.byte	0x28
	.long	0x140
	.byte	0x44
	.uleb128 0x1
	.ascii "vbe_control_info\0"
	.byte	0x4
	.byte	0x29
	.long	0x140
	.byte	0x48
	.uleb128 0x1
	.ascii "vbe_mode_info\0"
	.byte	0x4
	.byte	0x2a
	.long	0x140
	.byte	0x4c
	.uleb128 0x1
	.ascii "vbe_mode\0"
	.byte	0x4
	.byte	0x2b
	.long	0x140
	.byte	0x50
	.uleb128 0x1
	.ascii "vbe_interface_seg\0"
	.byte	0x4
	.byte	0x2c
	.long	0x140
	.byte	0x54
	.uleb128 0x1
	.ascii "vbe_interface_off\0"
	.byte	0x4
	.byte	0x2d
	.long	0x140
	.byte	0x58
	.uleb128 0x1
	.ascii "vbe_interface_len\0"
	.byte	0x4
	.byte	0x2e
	.long	0x140
	.byte	0x5c
	.byte	0
	.uleb128 0xa
	.long	0x140
	.long	0x382
	.uleb128 0xb
	.long	0x79
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.ascii "multiboot_info_t\0"
	.byte	0x4
	.byte	0x2f
	.byte	0x1f
	.long	0x1bd
	.uleb128 0x9
	.byte	0x10
	.byte	0x5
	.byte	0xa
	.long	0x3c5
	.uleb128 0x1
	.ascii "base_addr\0"
	.byte	0x5
	.byte	0xb
	.long	0x151
	.byte	0
	.uleb128 0x1
	.ascii "length\0"
	.byte	0x5
	.byte	0xc
	.long	0x151
	.byte	0x8
	.byte	0
	.uleb128 0x5
	.ascii "free_region_t\0"
	.byte	0x5
	.byte	0xd
	.byte	0x1b
	.long	0x39b
	.uleb128 0xa
	.long	0x3c5
	.long	0x3eb
	.uleb128 0xb
	.long	0x79
	.byte	0x7f
	.byte	0
	.uleb128 0x2
	.ascii "free_regions\0"
	.byte	0x5
	.byte	0x11
	.byte	0x16
	.long	0x3db
	.uleb128 0x9
	.byte	0x3
	.quad	free_regions
	.uleb128 0x3
	.byte	0x1
	.byte	0x2
	.ascii "_Bool\0"
	.uleb128 0x11
	.ascii "free_region_count\0"
	.byte	0x1
	.byte	0x9
	.byte	0x5
	.long	0xcc
	.uleb128 0x9
	.byte	0x3
	.quad	free_region_count
	.uleb128 0xd
	.ascii "memset\0"
	.byte	0x6
	.byte	0x11
	.byte	0x7
	.long	0x45a
	.long	0x45a
	.uleb128 0x7
	.long	0x45a
	.uleb128 0x7
	.long	0xcc
	.uleb128 0x7
	.long	0x79
	.byte	0
	.uleb128 0x12
	.byte	0x8
	.uleb128 0xd
	.ascii "printf\0"
	.byte	0x7
	.byte	0x28
	.byte	0x5
	.long	0xcc
	.long	0x476
	.uleb128 0x7
	.long	0x476
	.uleb128 0x13
	.byte	0
	.uleb128 0x6
	.long	0x65
	.uleb128 0x14
	.ascii "free\0"
	.byte	0x1
	.byte	0x64
	.byte	0x6
	.quad	.LFB5
	.quad	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.long	0x621
	.uleb128 0x8
	.ascii "_Memory\0"
	.byte	0x64
	.byte	0x11
	.long	0x45a
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x2
	.ascii "address\0"
	.byte	0x1
	.byte	0x65
	.byte	0xf
	.long	0xa4
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x2
	.ascii "sizeptr\0"
	.byte	0x1
	.byte	0x66
	.byte	0xf
	.long	0x621
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x2
	.ascii "size\0"
	.byte	0x1
	.byte	0x67
	.byte	0xe
	.long	0x151
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0x2
	.ascii "inserted_index\0"
	.byte	0x1
	.byte	0x6a
	.byte	0x9
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0xc
	.quad	.LBB6
	.quad	.LBE6-.LBB6
	.long	0x520
	.uleb128 0x2
	.ascii "i\0"
	.byte	0x1
	.byte	0x6b
	.byte	0xe
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0xc
	.quad	.LBB7
	.quad	.LBE7-.LBB7
	.long	0x59f
	.uleb128 0x2
	.ascii "i\0"
	.byte	0x1
	.byte	0x79
	.byte	0xe
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x4
	.quad	.LBB8
	.quad	.LBE8-.LBB8
	.uleb128 0x2
	.ascii "j\0"
	.byte	0x1
	.byte	0x7a
	.byte	0x12
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x4
	.quad	.LBB9
	.quad	.LBE9-.LBB9
	.uleb128 0x2
	.ascii "temp_addr\0"
	.byte	0x1
	.byte	0x7e
	.byte	0x1a
	.long	0x151
	.uleb128 0x3
	.byte	0x91
	.sleb128 -88
	.uleb128 0x2
	.ascii "temp_len\0"
	.byte	0x1
	.byte	0x7f
	.byte	0x1a
	.long	0x151
	.uleb128 0x3
	.byte	0x91
	.sleb128 -96
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x4
	.quad	.LBB10
	.quad	.LBE10-.LBB10
	.uleb128 0x2
	.ascii "i\0"
	.byte	0x1
	.byte	0x8b
	.byte	0xe
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x4
	.quad	.LBB11
	.quad	.LBE11-.LBB11
	.uleb128 0x2
	.ascii "j\0"
	.byte	0x1
	.byte	0x8d
	.byte	0x12
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x4
	.quad	.LBB12
	.quad	.LBE12-.LBB12
	.uleb128 0x2
	.ascii "end_i\0"
	.byte	0x1
	.byte	0x90
	.byte	0x16
	.long	0x151
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0x4
	.quad	.LBB13
	.quad	.LBE13-.LBB13
	.uleb128 0x2
	.ascii "k\0"
	.byte	0x1
	.byte	0x94
	.byte	0x1a
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -44
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x6
	.long	0x151
	.uleb128 0xe
	.ascii "malloc\0"
	.byte	0x54
	.byte	0x7
	.long	0x45a
	.quad	.LFB4
	.quad	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x67d
	.uleb128 0x8
	.ascii "_Size\0"
	.byte	0x54
	.byte	0x15
	.long	0x6a
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x2
	.ascii "region\0"
	.byte	0x1
	.byte	0x55
	.byte	0x14
	.long	0x67d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x2
	.ascii "data\0"
	.byte	0x1
	.byte	0x5b
	.byte	0xf
	.long	0x621
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x6
	.long	0x3c5
	.uleb128 0xe
	.ascii "FirstRegionOfSizeOrMore\0"
	.byte	0x2b
	.byte	0x10
	.long	0x67d
	.quad	.LFB3
	.quad	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.long	0x7a3
	.uleb128 0x8
	.ascii "size\0"
	.byte	0x2b
	.byte	0x2f
	.long	0x6a
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x2
	.ascii "requestedSize\0"
	.byte	0x1
	.byte	0x2c
	.byte	0xe
	.long	0x151
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0x4
	.quad	.LBB2
	.quad	.LBE2-.LBB2
	.uleb128 0x2
	.ascii "i\0"
	.byte	0x1
	.byte	0x2e
	.byte	0xe
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x4
	.quad	.LBB3
	.quad	.LBE3-.LBB3
	.uleb128 0x2
	.ascii "currBase\0"
	.byte	0x1
	.byte	0x31
	.byte	0x12
	.long	0x151
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0x2
	.ascii "currSize\0"
	.byte	0x1
	.byte	0x32
	.byte	0x12
	.long	0x151
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x2
	.ascii "merged\0"
	.byte	0x1
	.byte	0x34
	.byte	0xe
	.long	0x7a3
	.uleb128 0x3
	.byte	0x91
	.sleb128 -208
	.uleb128 0x2
	.ascii "changed\0"
	.byte	0x1
	.byte	0x37
	.byte	0xe
	.long	0x40a
	.uleb128 0x2
	.byte	0x91
	.sleb128 -49
	.uleb128 0xc
	.quad	.LBB4
	.quad	.LBE4-.LBB4
	.long	0x781
	.uleb128 0x2
	.ascii "j\0"
	.byte	0x1
	.byte	0x3a
	.byte	0x16
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.byte	0
	.uleb128 0x4
	.quad	.LBB5
	.quad	.LBE5-.LBB5
	.uleb128 0x2
	.ascii "j\0"
	.byte	0x1
	.byte	0x46
	.byte	0x16
	.long	0xcc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -60
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xa
	.long	0x40a
	.long	0x7b3
	.uleb128 0xb
	.long	0x79
	.byte	0x7f
	.byte	0
	.uleb128 0x15
	.ascii "parse_memory_map\0"
	.byte	0x1
	.byte	0xb
	.byte	0x6
	.quad	.LFB2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x815
	.uleb128 0x8
	.ascii "mb_info\0"
	.byte	0xb
	.byte	0x29
	.long	0x815
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x2
	.ascii "mmap_end\0"
	.byte	0x1
	.byte	0x15
	.byte	0xf
	.long	0xa4
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x2
	.ascii "mmap\0"
	.byte	0x1
	.byte	0x16
	.byte	0x1d
	.long	0x81a
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x6
	.long	0x382
	.uleb128 0x6
	.long	0x19e
	.byte	0
	.section	.debug_abbrev,"dr"
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 14
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0x21
	.sleb128 8
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 9
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0x8
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x1f
	.uleb128 0x1b
	.uleb128 0x1f
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"dr"
	.long	0x2c
	.word	0x2
	.secrel32	.Ldebug_info0
	.byte	0x8
	.byte	0
	.word	0
	.word	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"dr"
.Ldebug_line0:
	.section	.debug_str,"dr"
	.section	.debug_line_str,"dr"
.LASF1:
	.ascii "C:\\Users\\odayl\\Desktop\\Codetests\\asm\0"
.LASF0:
	.ascii "src/memory.c\0"
	.ident	"GCC: (GNU) 14.2.0"
	.def	printf;	.scl	2;	.type	32;	.endef
	.def	memset;	.scl	2;	.type	32;	.endef
