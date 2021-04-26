#include "operaciones.h"
#include "funciones.h"


#define DIM_ROTULO 10
#define DIM_COMANDO 40
#define DIM_DIRECCIONES 50
#define DIM_COMENTARIO 100
#define DIM_LINEACOMANDO 200
#define CANT_CELDAS 4096

typedef struct  //structura que maneja la salida por consola de lo que va haciendo el traductor
{
    int codigo;
    long hexa;
    char rotulo[DIM_ROTULO];
    char comando[DIM_COMANDO];
    char comentario[DIM_COMENTARIO];
} Linea;

typedef struct {
    char rotulo[DIM_ROTULO];
    int posicion;
} Rotulos;





int BuscaRotulo(char op1[], Rotulos rotulos[CANT_CELDAS], int cantRotulos) {
    int i = 0;
    while (i < cantRotulos && strcmp(op1, rotulos[i].rotulo) != 0)
        i++;
    if (i == cantRotulos)
        return -1;  //rotulo no encontrado;
    else
        return rotulos[i].posicion;
}

void BuscaComa(char cad[DIM_COMANDO], int *error) {
    int i = 0;
    while (i < strlen(cad) && cad[i] != ',')
        i++;
    if (i == strlen(cad) || cad[strlen(cad) - 1] == ',')
        *error = 1;
}

void CorrigeBlancos(char cadena[DIM_COMANDO]) {
    char aux[DIM_COMANDO];
    //espacios al inicio
    int j = 0, i = 0;
    while (cadena[i] == ' ' || cadena[i] == '	')
        i++;
    while (cadena[i] != '\0') {
        aux[j] = cadena[i];
        i++;
        j++;
    }
    aux[j] = '\0';
    strcpy(cadena, aux);
    if (cadena[strlen(aux) - 1] == ' ' || cadena[strlen(aux) - 1] == '	') {
        i = strlen(aux) - 1;
        while (cadena[i] == ' ' || cadena[i] == '	') {
            cadena[i] = '\0';
            i--;
        }
    }
}

int anytoint(char *s, char **out) {
    char *BASES = {"********@*#*****%"};
    int base = 10;
    char *bp = strchr(BASES, *s);
    if (bp != NULL) {
        base = bp - BASES;
        ++s;
    }
    return strtol(s, out, base);
}

int ComandoValido(char comando[DIM_COMANDO], instruccion instrucciones[DIM_OPERACIONES]) {  //Verifica si existe la instruccion
    int i = 0;
    while (i < DIM_OPERACIONES && strcmp(comando, instrucciones[i].inst) != 0)
        i++;
    return (i < DIM_OPERACIONES ? i : -1);  //Devuelve la posicion del comando
}

void cargaInstrucciones(instruccion ins[DIM_OPERACIONES]) {
    //2 operandos
    strcpy(ins[0].inst, "MOV");
    ins[0].codigo = 0x0;
    strcpy(ins[1].inst, "ADD");
    ins[1].codigo = 0x1;
    strcpy(ins[2].inst, "SUB");
    ins[2].codigo = 0x2;
    strcpy(ins[3].inst, "SWAP");
    ins[3].codigo = 0x3;
    strcpy(ins[4].inst, "MUL");
    ins[4].codigo = 0x4;
    strcpy(ins[5].inst, "DIV");
    ins[5].codigo = 0x5;
    strcpy(ins[6].inst, "CMP");
    ins[6].codigo = 0x6;
    strcpy(ins[7].inst, "SHL");
    ins[7].codigo = 0x7;
    strcpy(ins[8].inst, "SHR");
    ins[8].codigo = 0x8;
    strcpy(ins[9].inst, "AND");
    ins[9].codigo = 0x9;
    strcpy(ins[10].inst, "AND");
    ins[10].codigo = 0xA;
    strcpy(ins[11].inst, "XOR");
    ins[11].codigo = 0xB;
    for (int i = 0; i <= 11; i++)
        ins[i].operandos = 2;
    //1 operando
    strcpy(ins[12].inst, "SYS");
    ins[12].codigo = 0xF0;
    strcpy(ins[13].inst, "JMP");
    ins[13].codigo = 0xF1;
    strcpy(ins[14].inst, "JZ");
    ins[14].codigo = 0xF2;
    strcpy(ins[15].inst, "JP");
    ins[15].codigo = 0xF3;
    strcpy(ins[16].inst, "JN");
    ins[16].codigo = 0xF4;
    strcpy(ins[17].inst, "JNZ");
    ins[17].codigo = 0xF5;
    strcpy(ins[18].inst, "JNP");
    ins[18].codigo = 0xF6;
    strcpy(ins[19].inst, "JNN");
    ins[19].codigo = 0xF7;
    strcpy(ins[20].inst, "LDL");
    ins[20].codigo = 0xF8;
    strcpy(ins[21].inst, "LDH");
    ins[21].codigo = 0xF9;
    strcpy(ins[22].inst, "RND");
    ins[22].codigo = 0xFA;
    strcpy(ins[23].inst, "NOT");
    ins[23].codigo = 0xFB;
    for (int i = 12; i <= 23; i++)
        ins[i].operandos = 1;

    //0 operandos
    strcpy(ins[24].inst, "STOP");
    ins[24].codigo = 0xFF1;
    ins[24].operandos = 0;
}
