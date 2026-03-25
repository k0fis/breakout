; Breakout — WHDLoad slave (BOOTDOS + HDINIT)
; Boots Kickstart 3.1, sets up virtual filesystem, LoadSegs and runs the game.
; Assemble with: vasmm68k_mot -devpac -nosym -Fhunkexe -I<includes> -o Breakout.slave

	INCDIR	Include:
	INCLUDE	whdload.i
	INCLUDE	whdmacros.i

; --- LVO offsets (inline to avoid NDK lvo/ dependency) ---
_LVOOldOpenLibrary	EQU	-408
_LVOLoadSeg		EQU	-150

; --- Configuration for kick31.s ---
CHIPMEMSIZE	= $1ff000	; ~2 MB chip RAM
FASTMEMSIZE	= $0		; no fast RAM needed
NUMDRIVES	= 1
WPDRIVES	= %0000

BLACKSCREEN			; black screen on start
BOOTDOS				; call _bootdos after OS boots
HDINIT				; virtual filesystem from data/ dir
INITAGA				; AGA chipset init
IOCACHE		= 10000
SETKEYBOARD

slv_Version	= 16		; minimum WHDLoad version
slv_Flags	= WHDLF_NoError|WHDLF_Examine
slv_keyexit	= $5D		; Num-* to quit

	INCLUDE	whdload/kick31.s

; --- Slave metadata (labels referenced by kick31.s header) ---
slv_CurrentDir	dc.b	"data",0
slv_name	dc.b	"Breakout",0
slv_copy	dc.b	"2025 k0fis",0
slv_info	dc.b	"installed by WHDLoad",0
	EVEN

; --- _bootdos: called by kick31.s after AmigaOS is running ---
_bootdos:
	move.l	(_resload,pc),a2	; A2 = resload base

	; Open dos.library
	lea	(.dosname,pc),a1
	move.l	(4),a6			; ExecBase
	jsr	(_LVOOldOpenLibrary,a6)
	tst.l	d0
	beq	.fail
	move.l	d0,a6			; A6 = DOSBase

	; LoadSeg the game executable
	lea	(.program,pc),a0
	move.l	a0,d1
	jsr	(_LVOLoadSeg,a6)
	tst.l	d0
	beq	.fail

	; Convert BPTR to APTR, skip next-segment pointer → entry point
	lsl.l	#2,d0
	addq.l	#4,d0
	move.l	d0,a0
	jsr	(a0)			; run the game

	; Game exited — clean return to WHDLoad → Workbench
	move.l	(_resload,pc),a2
	pea	TDREASON_OK
	jmp	(resload_Abort,a2)

.fail:
	pea	TDREASON_FAILMSG
	move.l	(_resload,pc),a2
	jmp	(resload_Abort,a2)

.program:	dc.b	"breakout",0
.dosname:	dc.b	"dos.library",0
	EVEN
