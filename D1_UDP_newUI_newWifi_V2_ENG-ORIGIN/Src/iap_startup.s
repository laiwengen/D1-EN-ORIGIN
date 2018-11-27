	AREA IAPRESET, CODE, READONLY
	DCD	0x20000800
	DCD iap_reset
	SPACE 0xc0-0x08
	AREA	|.text|, CODE, READONLY
iap_reset PROC
	EXPORT iap_reset
	IMPORT iap_main
	
	LDR r0, =iap_main
	BX r0
	ENDP
	END
