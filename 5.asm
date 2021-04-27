; Bueno, ya hemos terminado o casi...
        mov     [0], %62
        mov     [1], @165
        mov     [2], 101
        mov     [3], #110
        mov     [4], 'o'
        mov     ex, ' '
        not     ex
CMP1:   cmp     0,0
        jnz     CMP2
COM1:   and     [0], ex
CMP2:   cmp     15,%10
        jnn     COM3
        jn      COM2
        jmp     COM4
COM2:   and     [1], ex
CMP3:   cmp     %20,' 
        jz      COM3
        jmp     CMP3
COM3:   and     [2], ex
CMP4:   cmp     '(',')'
        jp     COM5
COM4:   and     [3], ex
CMP5:   cmp     1,2
        jnp     COM5
        jmp     CMP5
COM5:   and     [4], ex
        mov     AX, %110
        mov     CX, 5
        mov     DX, 0
        sys     2
    