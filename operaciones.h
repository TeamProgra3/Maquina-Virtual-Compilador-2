#include <stdio.h>
#include <stdlib.h>
#define DIM_OPERACION 5
#define DIM_OPERACIONES 25
typedef struct{
    char inst[DIM_OPERACION];
    int codigo;
    int operandos;
}instruccion;


void (*INST[255])(int *, int *,long []);
void cargaFunciones();
void cargarCC(int *a,long RAM[]);
void cargaInstrucciones(instruccion ins[DIM_OPERACIONES]); //lee las instrucciones y las carga en un vector de instrucciones (string)

//Instrucciones con dos operandos

void MOV(int *a,int *b,long RAM[]);
void ADD(int *a,int *b,long RAM[]);
void SUB(int *a,int *b,long RAM[]);
void MUL(int *a,int *b,long RAM[]);
void DIV(int *a,int *b,long RAM[]);
void SWAP(int *a,int *b,long RAM[]);
void CMP(int *a,int *b,long RAM[]);
void AND(int *a,int *b,long RAM[]);
void OR(int *a,int *b,long RAM[]);
void XOR(int *a,int *b,long RAM[]);
void SHL(int *a,int *b,long RAM[]);
void SHR(int *a,int *b,long RAM[]);

//Instrucciones con un operando

void SYS(int *a,int *b,long RAM[]);
void JMP(int *a,int *b,long RAM[]);
void JZ(int *a,int *b,long RAM[]);
void JP(int *a,int *b,long RAM[]);
void JN(int *a,int *b,long RAM[]);
void JNZ(int *a,int *b,long RAM[]);
void JNP(int *a,int *b,long RAM[]);
void JNN(int *a,int *b,long RAM[]);
void LDH(int *a,int *b,long RAM[]);
void LDL(int *a,int *b,long RAM[]);
void RND(int *a,int *b,long RAM[]);
void NOT(int *a,int *b,long RAM[]);

//Instruccion sin operando

void STOP(int *a,int *b,long RAM[]);


/* Antigua funcion para cargar vector de instrucciones:

typedef struct{
    char inst[5];
    int codigo;
    int operandos;
}instruccion;

void cargainstruccion(instruccion v[]);
*/