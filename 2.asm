    mov ax,1
    mov [0],ax
    mov bx,[0]
    sub ax,1
    mov [1],ax
    mov ex,[1]
    sub ax,1
    mov [2],ax
    mov ex,[2]
    mul ax,%ffff
    mov [3],ax
    mov fx,[3]
    ldh 65535
    ldl 65535
    mov [4],ac
    shr ac,32
    mov cx, 1
    shl cx, 31
    shr cx, 31    
    mov [5], cx
    sys %F 
    stop