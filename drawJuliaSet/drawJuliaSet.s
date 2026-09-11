.data
FRAME_WIDTH:    .word 640
FRAME_HEIGHT:   .word 480
four_million:	.word 4000000
constant:		.word 274877907    
.text
.global drawJuliaSet

drawJuliaSet:
    stmfd   sp!, {fp, lr}
    add     fp, sp, #4
    sub     sp, sp, #48
   	mov r4, sp
	rsb sp, lr, r0	@符合要求
	mov sp, r4
    @ 儲存參數
    str     r0, [fp, #-40]   @ cX
    str     r1, [fp, #-44]   @ cY
    str     r2, [fp, #-48]   @ width
    str     r3, [fp, #-52]   @ height

    @ 設定初始值
    mov     r3, #255
    str     r3, [fp, #-28]   @ maxIter

    @ x迴圈
    mov     r3, #0
    str     r3, [fp, #-20]   @ x = 0
    b       x_analysis               @ 跳到x迴圈判斷

x_loop:     @ x迴圈開始
    mov     r3, #0
    str     r3, [fp, #-24]   @ y = 0
    b       y_analysis               @ 跳到y迴圈判斷

y_loop:     @ y迴圈主體
    @ 計算 zx
	mov 	r0, #1024
	add 	r0, r0, #512
	sub 	r0, r0, #36
	ldr 	r1, [fp, #-20] @ r1 = x
	ldr 	r2, [fp, #-48] @ r2 = width
	mov 	r2, r2, asr #1 @ r2 = width>>1
	sub		r1, r1, r2	   @ r1 = x - (width>>1)
	mul		r0, r0, r1	   @ r0 = 1500 * (x - (width>>1))
	mov		r1, r2		   @ r1 = width>>1
	bl 		__aeabi_idiv
	str		r0, [fp, #-8]



    @ 計算 zy
    ldr     r3, [fp, #-52]   @ height
    mov     r3, r3, asr #1
    ldr     r2, [fp, #-24]   @ y
    rsb     r2, r3, r2
    mov     r3, r2
    mov     r3, r3, asl #5
    sub     r3, r3, r2
    mov     r3, r3, asl #2
    add     r3, r3, r2
    mov     r3, r3, asl #3
    mov     r2, r3
    ldr     r3, [fp, #-52]
    mov     r3, r3, asr #1
    mov     r0, r2
    mov     r1, r3
    bl      __aeabi_idiv
    mov     r3, r0
    str     r3, [fp, #-12]   @ 儲存 zy

    @ i = maxIter
    ldr     r3, [fp, #-28]
    str     r3, [fp, #-16]
    b       while_analysis

while:     @ while迴圈主體
    @ 計算新的 zx, zy
    ldr     r3, [fp, #-8]    @ zx
    ldr     r2, [fp, #-8]
    mul     r2, r3, r2       @ zx*zx
    ldr     r3, [fp, #-12]   @ zy
    ldr     r1, [fp, #-12]
    mul     r3, r1, r3       @ zy*zy
    sub     r3, r2, r3       @ zx*zx - zy*zy
    ldr     r2, =constant
	ldr 	r2, [r2] 
    smull   r1, r2, r3, r2
    mov     r2, r2, asr #6 @----
    mov     r3, r3, asr #31
    sub     r2, r2, r3
    ldr     r3, [fp, #-40]   @ cX
    add     r3, r2, r3
    str     r3, [fp, #-32]   @ tmp

    @ 計算新的 zy
    ldr     r3, [fp, #-8]
	mov 	r2, #2
    mul     r3, r3, r2   @ 2*zx
    ldr     r2, [fp, #-12]   @ zy
    mul     r3, r2, r3       @ 2*zx*zy
    ldr     r2, =constant          
	ldr		r2, [r2]
    smull   r1, r2, r3, r2
    mov     r2, r2, asr #6 @----
    mov     r3, r3, asr #31
    sub     r2, r2, r3
    ldr     r3, [fp, #-44]   @ cY
    add     r3, r2, r3
    str     r3, [fp, #-12]   @ 更新 zy

    ldr     r3, [fp, #-32]
    str     r3, [fp, #-8]    @ 更新 zx=tmp
    
    @ i--
    ldr     r3, [fp, #-16]
    sub     r3, r3, #1
    str     r3, [fp, #-16]

while_analysis:     @ while迴圈條件檢查
    ldr     r3, [fp, #-8]    @ zx
    mul     r2, r3, r3
    ldr     r3, [fp, #-12]   @ zy
    mul     r3, r3, r3
    add     r2, r2, r3
    ldr     r3, =four_million        @ 4000000
    ldr 	r3, [r3]
	cmp     r2, r3
    bgt     color_calculate
    ldr     r3, [fp, #-16]   @ i
    cmp     r3, #0
	ble		end_while
    bgt     while
end_while:
color_calculate:     @ 計算顏色
    ldr     r3, [fp, #-16]   @ r3 = i
	mov 	r3, r3, asl #24
	mov		r3, r3, lsr #24
	mov		r4, r3
	mov     r3, r3, asl #8
	orr		r3, r4, r3
	mvn		r3, r3
	strh    r3, [fp, #-34]


    @ 儲存到 frame buffer
    ldr     r2, [fp, #-24]   @ y
    mov     r3, r2
    mov     r3, r3, asl #2
    add     r3, r3, r2
    mov     r3, r3, asl #8 @----
    ldr     r2, [fp, #4]     @ frame
    add     r2, r2, r3
    ldr     r3, [fp, #-20]   @ x
    mov     r3, r3, asl #1
    add     r3, r2, r3
    ldrh    r2, [fp, #-34]
    strh    r2, [r3]

    @ y++
    ldr     r3, [fp, #-24]
    add     r3, r3, #1
    str     r3, [fp, #-24]

y_analysis:     @ y迴圈條件檢查
    ldr     r0, [fp, #-24]
    ldr     r1, [fp, #-52]
    cmp     r0, r1
	bge		end_y_loop
    blt     y_loop
end_y_loop:
    @ x++
    ldr     r3, [fp, #-20]
    add     r3, r3, #1
    str     r3, [fp, #-20]

x_analysis:     @ x迴圈條件檢查
    ldr     r0, [fp, #-20]
    ldr     r1, [fp, #-48]
    cmp     r0, r1
	bge		end_x_loop
    blt     x_loop
end_x_loop:
    @ 恢復堆疊並返回
    sub     sp, fp, #4
    ldmfd   sp!, {fp, pc}



	


