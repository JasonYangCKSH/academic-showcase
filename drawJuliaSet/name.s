.data
msg1:	.asciz "*****Print Name*****\n"

team:	.asciz "Team 08\n"
EndPrint:
		.asciz "*****End Print*****\n"
member1: .asciz "Jason Yang"
member2: .asciz "An"
member3: .asciz "Jason Yang"
newline: .asciz "\n"
.bss
temp:	.word
.text
.global name

name:
	stmfd sp!, {lr}
	ldr r0, =msg1
	bl printf
	ldr r0, =team
	bl printf
	ldr r0, =member1
	bl printf
	ldr r0, =newline
	bl printf
	ldr r0, =member2
	bl printf
	ldr r0, =newline
	bl printf
	ldr r0, =member3
	bl printf
	ldr r0, =newline
	bl printf
	ldr r0, =EndPrint
	bl printf
	
	mov r0, #0
	ldr r1, =temp 
	str r0, [r1, #4]!

	ldmfd sp!, {lr}
	mov pc, lr
