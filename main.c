#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIM_ROTULO 10
#define DIM_COMANDO 40
#define DIM_DIRECCIONES 50
#define DIM_COMENTARIO 100
#define DIM_LINEACOMANDO 200
#define CANT_CELDAS 4096
#define DIM_OPERACIONES 25
#define DIM_OPERACION 5
#define DIM_PARAMETROS 10
typedef struct {
    char inst[DIM_OPERACION];
    int codigo;
    int operandos;
} instruccion;
typedef struct
{
    char parametro[DIM_PARAMETROS];
    int codigo;
}ParametrosHeader;


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

void magia();
void leeArchivo(Linea v[DIM_LINEACOMANDO], ParametrosHeader v2[5],int Header[], int *cant, char[]);                 //lee del archivo asembler y lo traduce al binario
void creaComando(char comando[DIM_COMANDO], char comentario[DIM_COMENTARIO], Linea *linea);  //Obvio que crea el comando jaja
void corrigeComando(char comando[DIM_COMANDO], Linea *linea);                                //corrige el comando, es decir se fija si tiene un rotulo prefijado o no
void muestra(Linea v[DIM_LINEACOMANDO], int cant);
void compilaCodigo(Linea v[DIM_LINEACOMANDO], int Header[], int cant, instruccion ins[DIM_OPERACIONES], char[], int *);
void creaBinario(Linea linea[DIM_LINEACOMANDO], int Header[], int cantOperaciones, char nombreArch[]);
void RecuperaInstruccion(Linea LineaActual, char instruccionActual[DIM_COMANDO], Rotulos rotulos[], int cant, char op1[], char op2[]);
long ArmaCodigo(int, int, char op1[], char op2[], int *, int i);
void ArmaOperando(char op[DIM_COMANDO], int cantOperandos, int indice, int *valor, int *errorOp, int *tipo);
int BuscaRotulo(char op1[], Rotulos rotulos[CANT_CELDAS], int cantRotulos);
void BuscaComa(char[], int *error);
void CorrigeBlancos(char cadena[DIM_COMANDO]);
int anytoint(char *s, char **out);
int ComandoValido(char comando[DIM_COMANDO], instruccion ins[DIM_OPERACIONES]);
void cargaInstrucciones(instruccion ins[DIM_OPERACIONES]);
int tieneHeader(char comando[DIM_COMANDO]);
void creaHeader(ParametrosHeader v2[5],int Header[], char *comando) ; //en el char va el nombre del archivo (El Header puede estar en cualquier parte del codigo) para poder leer el header

int main(int argsCant, char *arg[])  //argsCant es cantidad de argumentos
{
    int errorSintaxis = 0, cant = 0;
    Linea Lineas[DIM_LINEACOMANDO];
    instruccion instrucciones[DIM_OPERACIONES];
    int Header[5];
    ParametrosHeader parHeader[5];
    cargaInstrucciones(instrucciones);
    cargaParametrosHeader(parHeader);
    magia();
    if (argsCant >= 3) {
        leeArchivo(Lineas,parHeader, Header, &cant, arg[1]);
        compilaCodigo(Lineas, Header, cant, instrucciones, arg[2], &errorSintaxis);  //arg[1] archivo assembler y arg[2] es nombre archivo salida .bin
        if (errorSintaxis)
            printf("\n ERROR EN LA COMPILACION: Error de sintaxis en la instruccion nro: %d \n\n", errorSintaxis + 1);
        else
            printf("\nCompilacion exitosa, no se detectaron errores de sintaxis\n\n");
    } else {
        printf("Error, faltan argumentos");
        return -1;
    }
    if (!(argsCant == 4 && (strcmp(arg[3], "-o") == 0 || strcmp(arg[3], "-O") == 0)))
        muestra(Lineas, cant);
    else {
        printf("Salida por pantalla omitida. \n");
    }
    if (errorSintaxis)
        printf("\n ERROR EN LA COMPILACION: Error de sintaxis en la instruccion nro: %d \n\n", errorSintaxis + 1);
    else
        printf("\nCompilacion exitosa, no se detectaron errores de sintaxis\n\n");
    return 0;
}
void leeArchivo(Linea v[DIM_LINEACOMANDO],ParametrosHeader v2[5],int Header[], int *cant, char ArchFuente[40]) {
    int i = 0;                        //posicion del caracter en la palabra (aclaracion: esto solo se utiliza en el codigo y no en las aclaraciones\comentarios)
    char caracter;                    //para leer un unico caracter del archivo
    char comando[DIM_COMANDO];        //variable que se utiliza para almacenar el comando
    char comentario[DIM_COMENTARIO];  //Variable que almacena el comentario
    FILE *arch;

    arch = fopen(ArchFuente, "rt");
    fscanf(arch, "%c", &caracter);
    while (!feof(arch)) {
        while (caracter != ';' && caracter != '\n' && !feof(arch)) {
            comando[i++] = caracter;
            fscanf(arch, "%c", &caracter);
        }
        comando[i] = '\0';
        i = 0;
        while (caracter != '\n' && !feof(arch)) {
            comentario[i++] = caracter;
            fscanf(arch, "%c", &caracter);
        }
        comentario[i] = '\0';
        i = 0;
        if (!tieneHeader(comando)) {
            creaComando(comando, comentario, &v[*cant]);
            (*cant)++;
        } else
            creaHeader(v2,Header, comando);
        comando[0] = '\0';
        comentario[0] = '\0';
        fscanf(arch, "%c", &caracter);
    }
    fclose(arch);
}
void creaComando(char comando[DIM_COMANDO], char comentario[DIM_COMENTARIO], Linea *linea) {
    int i = 0;
    strcpy(linea->comentario, comentario);
    while (comando[i] == ' ' || comando[i] == '	')  //  DESCARTA LOS ESPACIOS EN BLANCO AL INICIO creo que va en el segundo argumento comando[i]!= '\0'
        i++;
    if (comando[i] == '\0') {
        linea->codigo = -1;
    } else {
        corrigeComando(comando, linea);
    }
}
void corrigeComando(char comando[DIM_COMANDO], Linea *linea) {
    int i = 0;
    int j = 0;
    int k;
    char aux[25];
    CorrigeBlancos(comando);
    while (comando[i] != ':' && comando[i] != '\0')
        //  UTILIZA UN AUX PARA ALMACENAR LO QUE LLEGA DE COMANDO
        if ((int)comando[j - 1] != 39)  //comilla simple
            aux[j++] = toupper(comando[i++]);
        else
            aux[j++] = comando[i++];

    if (comando[i] == ':') {
        //  SI SE ENCUENTA CON UN ':' (PRESENCIA DE UN ROTULO) LO INGRESA EN LA SECCION ROTULO
        i++;
        aux[j++] = '\0';
        j = 0;
        strcpy(linea->rotulo, aux);
        while (comando[i] == ' ' || comando[i] == '	')
            //  DESCARTA LOS ESPACIOS EN BLANCO
            i++;
        while (comando[i] != '\0') {
            //  EL RESTO DE LOS CARACTERES LOS INGRESA EN LA LINEA DE COMANDO
            linea->comando[j++] = toupper(comando[i++]);
        }
        linea->comando[++j] = '\0';
    } else {
        //  SI NO ENCUENTRA ROTULO LO INGRESA DIRECTAMENTE
        aux[j] = '\0';
        strcpy(linea->comando, aux);
        linea->rotulo[0] = '0';
    }
}
void muestra(Linea v[DIM_LINEACOMANDO], int cant) {
    Linea linea;
    char aux[DIM_COMANDO];
    for (int i = 0; i < cant; i++) {
        linea = v[i];
        if (linea.codigo == -1)
            printf("%s", linea.comentario);
        else {
            if (strcmp(linea.rotulo, "0") == 0)
                sprintf(linea.rotulo, "%d", linea.codigo + 1);
            if (linea.hexa == 0xFFFFFFFF) {
                linea.codigo = 9999;
                strcpy(linea.rotulo, "----");
            }
            printf("[%04i]: [%02X %02X %02X %02X] | %8s | %s %s", linea.codigo, (linea.hexa >> 24) & 0xFF, (linea.hexa >> 16) & 0xFF, (linea.hexa >> 8) & 0xFF, (linea.hexa >> 0) & 0xFF, linea.rotulo, linea.comando, linea.comentario);
        }
        printf("\n");
    }
}
void compilaCodigo(Linea linea[DIM_LINEACOMANDO], int Header[], int cant, instruccion instrucciones[DIM_OPERACIONES], char nombreArch[40], int *error) {
    int errorOperando, i, cantRotulos = 0, cantOpsValidas = 0;
    long codOp;
    char op1[DIM_COMANDO] = {'\0'};
    char op2[DIM_COMANDO] = {'\0'};
    int pos = 0;
    char instruccionActual[DIM_COMANDO];
    Rotulos rotulos[CANT_CELDAS];
    for (i = 0; i < cant; i++) {  //Carga de todos los rotulos
        if (linea[i].codigo != -1) {
            if (linea[i].rotulo[0] != '0') {
                strcpy(rotulos[cantRotulos].rotulo, linea[i].rotulo);
                rotulos[cantRotulos].posicion = cantOpsValidas;
                cantRotulos++;
            }
            cantOpsValidas++;
        }
    }
    cantOpsValidas = 0;
    for (i = 0; i < cant; i++) {  //Analiza todas las instrucciones
        if (linea[i].codigo != -1) {
            RecuperaInstruccion(linea[i], instruccionActual, rotulos, cantRotulos, op1, op2);
            pos = ComandoValido(instruccionActual, instrucciones);
            if (pos != -1) {
                errorOperando = 0;
                codOp = ArmaCodigo(instrucciones[pos].codigo, instrucciones[pos].operandos, op1, op2, &errorOperando, i + 1);
                if (instrucciones[pos].operandos == 2)
                    BuscaComa(linea[i].comando, &errorOperando);
                if (errorOperando == 0) {
                    linea[i].hexa = codOp;
                    linea[i].codigo = cantOpsValidas;
                    cantOpsValidas++;
                } else {  //Error en uno de los operandos
                    *error = i;
                    linea[i].hexa = 0xFFFFFFFF;
                    linea[i].codigo = 0xFFFF;
                }
            } else {  //Error en la instruccion
                *error = i;
                linea[i].hexa = 0xFFFFFFFF;
                linea[i].codigo = 0xFFFF;
            }
        }
    }
    if (!(*error))
        creaBinario(linea, Header, cant, nombreArch);
}
void RecuperaInstruccion(Linea LineaActual, char instruccionActual[DIM_COMANDO], Rotulos rotulos[CANT_CELDAS], int cantRotulos, char op1[DIM_COMANDO], char op2[DIM_COMANDO]) {
    int pos, j, i = 0;
    char c = LineaActual.comando[0];
    op1[0] = op2[0] = '\0';
    while (c != ' ' && c != '\0') {  //OPERACION
        instruccionActual[i++] = c;
        c = LineaActual.comando[i];
    }
    instruccionActual[i] = '\0';
    CorrigeBlancos(instruccionActual);
    i++;
    j = 0;
    if (c != '\0') {
        char c = LineaActual.comando[i];
        while (c != ',' && c != '\0') {  //PRIMER OPERANDO
            op1[j++] = c;
            c = LineaActual.comando[++i];
        }
        i++;
        op1[j] = '\0';
        CorrigeBlancos(op1);
        j = 0;
        if (c != '\0') {
            char c = LineaActual.comando[i];
            while (c != ',' && c != '\0') {  //SEGUNDO OPERANDO
                op2[j++] = c;
                c = LineaActual.comando[++i];
            }
            op2[j] = '\0';
            CorrigeBlancos(op2);
        }
        //Verificar si el primer operando es un rotulo, en ese caso modificar el rotulo por el numero de linea correspondiente
        if (op1[0] != '[' && op1[0] != '@' && op1[0] != '%' && !(op1[1] == 'X' && op1[0] <= 'F' && op1[0] >= 'A') && !(op1[0] >= '0' && op1[0] <= '9') && strcmp(op1, "AC") != 0) {  //Es un rotulo
            pos = BuscaRotulo(op1, rotulos, cantRotulos);
            if (pos != -1) {
                sprintf(op1, "%d", pos);  //Caracter que representa el numero
            } else {
                //ROTULO NO ENCONTRADO! Error!
                printf("\nERROR DE ROTULO NO ENCONTRADO: Rotulo: %s\n", op1);
            }
        }
    }
}
long ArmaCodigo(int codigo, int cantOperandos, char op1[DIM_COMANDO], char op2[DIM_COMANDO], int *errorOp, int indice) {
    long codAux = 0;
    char aux2[DIM_COMANDO] = {0};
    int opA = 0, opB = 0;
    int tipoOp1 = 0, tipoOp2 = 0;
    ArmaOperando(op1, cantOperandos, indice, &opA, errorOp, &tipoOp1);
    ArmaOperando(op2, cantOperandos, indice, &opB, errorOp, &tipoOp2);
    if (cantOperandos == 2)
        codAux = codigo << 28 | ((tipoOp1 << 26) & 0xC000000) | ((tipoOp2 << 24) & 0x3000000) | ((opA << 12) & 0xFFF000) | (opB & 0xFFF);
    else if (cantOperandos == 1)
        codAux = codigo << 24 | ((tipoOp1 << 22) & 0xC00000) | (opA & 0xFFF);
    else
        codAux = codigo << 20;
    return codAux;
}
void creaBinario(Linea linea[DIM_LINEACOMANDO], int Header[], int cantidadOperaciones, char nombreArch[]) {
    int i;
    FILE *archivoSalida;
    archivoSalida = fopen(nombreArch, "wb");
    for (i = 0; i < 5; i++)
        fwrite(&Header[i], sizeof(Header[i]), 1, archivoSalida);
    for (i = 0; i < cantidadOperaciones; i++)
        if (linea[i].codigo != -1)  //Si no es un solo comentario
            fwrite(&linea[i].hexa, sizeof(linea[i].hexa), 1, archivoSalida);
    fclose(archivoSalida);
}
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
    while (cadena[i] != '\0')
        aux[j++] = cadena[i++];
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
    strcpy(ins[10].inst, "OR");
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
void ArmaOperando(char op[DIM_COMANDO], int cantOperandos, int indice, int *valor, int *errorOp, int *tipo) {
    char aux[DIM_COMANDO] = {0};
    int valorRealOP;
    int i;
    if (op[0] != '\0') {
        if (op[0] == '[') {  //Operador de tipo celda memoria
            i = 1;
            while (op[i] != ']' && op[i] != '\0') {
                aux[i - 1] = op[i];
                i++;
            }
            if (op[i] != '\0') {
                aux[i] = '\0';
                *valor = anytoint(aux, NULL);
                *tipo = 2;
            } else
                *errorOp = 1;                                       //error por falta de ]
        } else if (op[0] >= 'A' && op[0] <= 'F' && op[1] == 'X') {  //Para pasar la letra a su valor decimal (A=10, B=11,...,F=15)
            *tipo = 1;
            *valor = op[0] - 55;
        } else if (strcmp(op, "AC") == 0) {
            *tipo = 1;
            *valor = 9;
        } else if ((int)op[0] == 39) {  //Comilla simple '
            *valor = (int)op[1];
        } else {  // Es un inmediato
            if (op[strlen(op) - 1] != ']') {
                *tipo = 0;
                *valor = anytoint(op, NULL);
            } else {  //Falta el [ (ERROR)
                *errorOp = 1;
            }
        }
        if (cantOperandos == 2) {
            if (*valor < -2048 || *valor > 2047) {  //-2048 a 2047
                valorRealOP = *valor << 20;
                valorRealOP = valorRealOP >> 20;
                printf("\nAdvertencia EL VALOR EXCEDE EL LIMITE MAXIMO DE MEMORIA: linea: %i - Valor original: %d - Valor truncado a: %d \n", indice, *valor, valorRealOP);
            }
        } else if (cantOperandos == 1)
            if (*valor < -32768 || *valor > 32767) {  // -32768 a 32767
                valorRealOP = *valor << 16;
                valorRealOP = valorRealOP >> 16;
                printf("\nAdvertencia EL VALOR EXCEDE EL LIMITE MAXIMO DE MEMORIA: linea: %i - Valor original: %d - Valor truncado a: %d \n", indice, *valor, valorRealOP);
            }
    }
}

void cargaParametrosHeader(ParametrosHeader parametrosHeader[DIM_PARAMETROS]){
    strcpy(parametrosHeader[0].parametro,'\0');
    parametrosHeader[0].codigo=0x4D563231;
    strcpy(parametrosHeader[1].parametro,"DATA");
    strcpy(parametrosHeader[2].parametro,"STACK");
    strcpy(parametrosHeader[3].parametro,"EXTRA");
    strcpy(parametrosHeader[4].parametro,'\0');

    for(int i=1;i<3;i++)
        parametrosHeader[i].codigo=0x400;
    
}
void recuperaSegmento(int *pos, int *hexa, char aux[DIM_COMANDO], char valor[DIM_COMANDO],ParametrosHeader v2[5]) {
    //halta hacer la magia aca uwu
}

void creaHeader(ParametrosHeader v2[5],int Header[], char *comando)  //en el char va el nombre del archivo (El Header puede estar en cualquier parte del codigo) para poder leer el header
{
    char aux[DIM_COMANDO], valor[DIM_COMANDO];
    int i = 0, j = 0;
    int pos, hexa;
    while (comando[i] != " " && comando[i] != '    ')
        i++;
    while (comando[i] != '\0') {
        if (comando[i] != "=")
            aux[i++] = comando[i];
        else {
            i++;
            while (comando[i] != '\0' && comando[i] != " " && comando[i] != " ")
                valor[j++] = comando[i++];
            recuperaSegmento(&pos, &hexa, aux, valor,v2);
            Header[pos] = hexa;
        }
    }
    Header[0] = 0x4D563231;
}

int tieneHeader(char comando[DIM_COMANDO]) {
    int i = 0;
    while (comando[i] != "\\" && comando[i] != '\0')
        i++;
    return comando[i + 1] == "\\";
}
void magia() {
    printf("-----------------------VERSION 1.31------------------------\n");
    printf("-----------------Cazorla y Jamon a las 12:40 AM -----------\n");
    printf("------------Falta crear una funcion y testing--------------\n");
}