;EXTERN JmpAddr:QWORD
.CODE
MyNtOpenProcess PROC
    mov     r10, rcx
    mov     eax, 26h
    syscall
    ret
MyNtOpenProcess ENDP

MyOpenProcess PROC
    mov     r11, rsp
    sub     rsp, 68h
    and     qword ptr [r11-40h], 0
    lea     r9, [r11-48h]
    movsxd  rax, r8d
    xorps   xmm0, xmm0
    mov     r12,30h
    mov     [rsp + 68h - 38h], r12
    lea     r8, [r11-38h]
    and     qword ptr [r11-30h], 0
    neg     edx
    mov     [r11-48h], rax
    mov     edx, ecx
    lea     rcx, [r11+20h]
    sbb     eax, eax
    and     eax, 2
    mov     [rsp+68h-20h], eax
    and     qword ptr [r11-28h], 0
    movdqu  [rsp+68h-18h], xmm0
    call    MyNtOpenProcess
    nop     dword ptr [rax+rax+00h]
    mov     rax, [rsp+68h+20h]
    add     rsp,68h
    ret
MyOpenProcess ENDP

MyZwReadVirtualMemory PROC
    mov     r10, rcx
    mov     eax, 3Fh
    syscall
    ret
MyZwReadVirtualMemory ENDP

MyReadProcessMemory PROC
    sub     rsp, 48h
    lea     rax, [rsp+48h-18h]
    mov     [rsp+48h-28h], rax
    call    MyZwReadVirtualMemory
    nop     dword ptr [rax+rax+00h]
    mov     rdx, [rsp+48h+28h]
    test    rdx, rdx
    jnz     short J
  S:mov     eax, 1
    add     rsp, 48h
    ret
  J:mov     rcx, [rsp+48h-18h]
    mov     [rdx], rcx
    jmp     short S
MyReadProcessMemory ENDP

MyNtWriteVirtualMemory PROC
    mov     r10, rcx
    mov     eax, 3Ah
    syscall
    ret
MyNtWriteVirtualMemory ENDP
END