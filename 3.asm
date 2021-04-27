; Leo entradas. ej: 101,AA,FF,CAE

        mov     ax  ,   %008
;       mov     ax  ,   %001
        mov     cx  ,   1
        mov     dx  ,   0
        sys     1 

; Inicializo         
        mov     ax  ,   32
        xor     [1] ,   [1]

; Calculos 

sigue:  mov     ex  ,   [0]
        shl     [0] ,   1
        shr     ex  ,   31
        amd     ex  ,   1
        mul     [1] ,   10
        add     [1] ,   ex
        sub     ax  ,   1
        jnz     sigue

; Muestro resultados        

        mov     ax  ,   %001
        mov     cx  ,   1
        mov     dx  ,   1
        sys     2
        stop
        mov     [1] ,   1 ; ojo esto no. 
        sys     2
        stop