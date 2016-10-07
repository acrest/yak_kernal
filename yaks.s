

; We need to implement enter and exit mutex, as well as dispatcher here

YKEnterMutex: ; Disable interrupts and return
	cli
	ret

YKExitMutex: ; Enable interrupts and return
	sti
	ret

YKDispatcher: 
	;push		BP
	;mov			BP,SP
	cmp 		byte [BP+4], 1
	je 			YKContextSaver
	
YKDispatcher2:
	;mov			[lastRunningTask], SP
	; save sp
	mov	bx, [lastRunningTask]
	mov [bx], sp
	;load sp
 	mov 		BX, [readyLstHead]
;	mov 		SP, BX
	mov			SP, [BX]

YKContextRestorer:
	pop			ES
	pop			DS
	pop			BP
	pop			DI
	pop			SI
	pop			DX
	pop			CX
	pop			BX
	pop			AX
iret

YKContextSaver:
	pushf
	push 		CS
	push 		word [BP+2]
	;push		BP
	push		AX
	push		BX
	push		CX
	push		DX
	push		SI
	push		DI
	push		BP
	push		DS
	push		ES
	jmp			YKDispatcher2


