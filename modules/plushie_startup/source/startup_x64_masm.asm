; export our entry point name
PUBLIC plushie_startup

; External procedure for main
EXTERN main:PROC

; ___imp_ is the prefix for symbols in the import address table (IAT) it's used for 
; indirect calls (i.e calls to a dll) it's a pointer to the address of the function 
; "ExitProcess" resolved after dll import of kernel32.dll
EXTERN __imp_ExitProcess:QWORD

.code

; --- ENTRY FUNCTION ------------------------------------------------------------------------------------
; The entry function is a non-leaf function ( because it calls main )
; 
; Our entry must support anything that walks the stack (debuggers, crash dumps, profilers, exception dispatch). 
; Given the current RIP and RSP, a stack walker has to be able to find this function's return address and 
; restore any non-volatile registers it saved.
;
; The Windows x64 ABI does this without frame pointers. After the prologue, the RSP must not move, 
; and every prologue step that changes the RSP or saves a register is recorded as an unwind code in 
; .xdata. A .pdata entry maps the function's address range to those codes. 
; PROC FRAME, .allocstack and .endprolog are the MASM directives that produce those entries.
plushie_startup PROC FRAME 
                                               ; --- PROLOGUE -------------------------------------------
											   ;
	sub          rsp, 28h                      ; Stack alignment 
	                                           ; + Callee is allowed to save it's rcx, rdx, r8, r9 input registers to the stack
											   ; therefore we need to reserve stack "shadow space" 
											   ; + The callee wants to use fast SSE instructions (i.e movaps) and these require 
											   ; a 16 byte stack alignment, the loader called us and put 8 byte return address on 
											   ; the stack
											   ; 32 bytes shadow space + 8 to realign to 16 = 40 = 0x28
											   ;
	.allocstack  28h                           ; Write xdata entry for our shadow data
	.ENDPROLOG                                 ; --- END PROLOGUE --------------------------------------
	                                           
											   ;
	call         main                          ; Actually call into user's main function 

	mov          ecx, eax                      ; ExitProcess takes 1 u32 parameter, place return code from main onto input param
	call         qword ptr[__imp_ExitProcess]  ; Make the indirect call to exit process
	int          3                             ; Exit process can't return

plushie_startup ENDP

END