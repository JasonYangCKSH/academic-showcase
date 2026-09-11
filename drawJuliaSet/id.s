.data
inputID:	.asciz "*****Input ID*****\n"
Enter1:	    .asciz "**Please Enter Member 1 ID:**\n"
Enter2:	    .asciz "**Please Enter Member 2 ID:**\n"
Enter3:	    .asciz "**Please Enter Member 3 ID:**\n"
PrintCommand:    
			.asciz "**Please Enter Command**\n"
PrintAll:	.asciz "*****Print team Member ID and ID Summation*****\n"
IDSum:		.asciz "ID Summation = "
EndPrint:	.asciz "*****End Print*****\n"
format1:	.asciz "%d"
format2:	.asciz "%s"
Command:	.space 2
ChangeLine: .asciz "\n"
p:			.asciz "p"
sum:        .word 0
id1:        .word 0
id2:        .word 0
id3:        .word 0
.global id1    
.global id2
.global id3
.global sum
.text

.global id

id:	
	stmfd sp!, {r4-r10, lr}    @ 第1道指令

	add r1, r0, #1            @ 第2道指令
	mov r2, r1, LSL #1        @ 第3道指令 (移位運算的Operand2格式)
	
	@ 暫存 sp 的值
	mov r3, sp                @ 第4道指令(保存原始 sp 值)
	subs sp, lr, pc           @ 第5道指令 (符合規定)
	mov sp, r3                @ 恢復原始 sp 值
	
	@ 更多條件執行指令
	cmp r0, #0
	movlt r3, #5              @ 條件執行指令1
	addgt r4, r3, r2          @ 條件執行指令2

	@ 原有的程式碼...
	@ 印出初始訊息
	ldr r0, =inputID
	bl printf
	
	@ 輸入 id1
	ldr r0, =Enter1
	bl printf
	
	ldr r0, =format1
	ldr r1, =id1
	bl scanf
	
	@ 輸入 id2
	ldr r0, =Enter2
	bl printf
	
	ldr r0, =format1
	ldr r1, =id2
	bl scanf
	
	@ 輸入 id3
	ldr r0, =Enter3
	bl printf
	
	ldr r0, =format1
	ldr r1, =id3
	bl scanf

	@ 計算總和
	ldr r0, =id1
	ldr r0, [r0]
	ldr r1, =id2
	ldr r1, [r1]
	add r0, r0, r1
	ldr r1, =id3
	ldr r1, [r1]
	add r0, r0, r1
	ldr r1, =sum
	str r0, [r1]
	
reset:	
	@ 輸入命令
	ldr r0, =PrintCommand
	bl printf
	
	ldr r0, =format2
	ldr r1, =Command
	bl scanf
	
	@ 比較命令
	ldr r1, =Command
	ldr r2, =p
	mov r3, #0
	mov r4, #0
	
loop:
	ldrb r3, [r1], #1
	ldrb r4, [r2], #1
	cmp r3, r4
	bne reset
	cmp r3, #0
	beq equal
	b loop
	
equal:
	@ 印出所有資訊
	ldr r0, =PrintAll
	bl printf
	
	ldr r0, =format1
	ldr r1, =id1
	ldr r1, [r1]
	bl printf
	
	ldr r0, =ChangeLine
	bl printf
	
	ldr r0, =format1
	ldr r1, =id2
	ldr r1, [r1]
	bl printf
	
	ldr r0, =ChangeLine
	bl printf
	
	ldr r0, =format1
	ldr r1, =id3
	ldr r1, [r1]
	bl printf
	
	ldr r0, =ChangeLine
	bl printf
	ldr r0, =ChangeLine
	bl printf
	
	ldr r0, =IDSum
	bl printf
	
	ldr r0, =format1
	ldr r1, =sum
	ldr r1, [r1]
	bl printf
	
	ldr r0, =ChangeLine
	bl printf
	
	ldr r0, =EndPrint
	bl printf
	
	@ 準備傳遞給 main 的值
	ldr r7, =id1
	ldr r7, [r7]
	ldr r8, =id2
	ldr r8, [r8]
	ldr r9, =id3
	ldr r9, [r9]
	
	ldmfd sp!, {r4-r10, lr}
	mov pc, lr
	
