.code

IFDEF RAX

public get_console_handle

get_console_handle:
	mov rcx, gs:[60h]
	mov rdx, [rcx+20h]
	mov rax, [rdx+10h]
	ret

ELSE




ENDIF

end

