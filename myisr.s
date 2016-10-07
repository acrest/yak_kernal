
resetISR:
;Save the context of whatever was running by pushing all registers onto the stack, except SP, SS, CS, IP, and the flags.
	push		AX
	push		BX
	push		CX
	push		DX
	push		SI
	push		DI
	push		BP
	push		DS
	push		ES

  cmp byte [isrDepth], 0
  jne resetISR2
  mov [lastRunningTask], SP

resetISR2:
  call YKEnterISR
;Enable interrupts to allow higher priority IRQs to interrupt.
	sti

;Run the interrupt handler, which is usually a C function that does the work for this interrupt.
	call		rISR

;Disable interrupts.
	cli

;Send the EOI command to the PIC, informing it that the handler is finished.
	call 		signalEOI

  call YKExitISR
;Restore the context of what was running before the interrupt occurred by popping the registers saved in step 1 off the stack.
	pop			ES
	pop			DS
	pop			BP
	pop			DI
	pop			SI
	pop			DX
	pop			CX
	pop			BX
	pop			AX

;Execute the iret instruction. This restores in one atomic operation the values for IP, CS, and the flags, which were automatically saved on the stack by the processor when the interrupt occurred. This effectively restores execution to the point where the interrupt occurred.
	iret

ticker:
;Save the context of whatever was running by pushing all registers onto the stack, except SP, SS, CS, IP, and the flags.
	push		AX
	push		BX
	push		CX
	push		DX
	push		SI
	push		DI
	push		BP
	push		DS
	push		ES

  cmp byte [isrDepth], 0
  jne ticker2
  mov [lastRunningTask], SP

ticker2:
  call YKEnterISR
;Enable interrupts to allow higher priority IRQs to interrupt.
	sti

;Run the interrupt handler, which is usually a C function that does the work for this interrupt.
	call		tick

;Disable interrupts.
	cli

;Send the EOI command to the PIC, informing it that the handler is finished.
	call 		signalEOI

  call YKExitISR
;Restore the context of what was running before the interrupt occurred by popping the registers saved in step 1 off the stack.
	pop			ES
	pop			DS
	pop			BP
	pop			DI
	pop			SI
	pop			DX
	pop			CX
	pop			BX
	pop			AX

;Execute the iret instruction. This restores in one atomic operation the values for IP, CS, and the flags, which were automatically saved on the stack by the processor when the interrupt occurred. This effectively restores execution to the point where the interrupt occurred.
	iret

keys:
;Save the context of whatever was running by pushing all registers onto the stack, except SP, SS, CS, IP, and the flags.
	push		AX
	push		BX
	push		CX
	push		DX
	push		SI
	push		DI
	push		BP
	push		DS
	push		ES
  
  cmp byte [isrDepth], 0
  jne keys2
  mov [lastRunningTask], SP

keys2:
  call YKEnterISR
;Enable interrupts to allow higher priority IRQs to interrupt.
	sti

;Run the interrupt handler, which is usually a C function that does the work for this interrupt.
	call		keystroke

;Disable interrupts.
	cli

;Send the EOI command to the PIC, informing it that the handler is finished.
	call 		signalEOI

  call YKExitISR
;Restore the context of what was running before the interrupt occurred by popping the registers saved in step 1 off the stack.
	pop			ES
	pop			DS
	pop			BP
	pop			DI
	pop			SI
	pop			DX
	pop			CX
	pop			BX
	pop			AX

;Execute the iret instruction. This restores in one atomic operation the values for IP, CS, and the flags, which were automatically saved on the stack by the processor when the interrupt occurred. This effectively restores execution to the point where the interrupt occurred.
	iret


