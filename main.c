#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIM_ROTULO 20
#define DIM_COMANDO 60
#define DIM_DIRECCIONES 50
#define DIM_COMENTARIO 100
#define DIM_LINEACOMANDO 200
#define CANT_CELDAS 8192
#define DIM_OPERACIONES 32
#define DIM_OPERACION 5
#define DIM_PARAMETROS 10

typedef struct {
    char inst[DIM_OPERACION];
    int codigo;
    int operandos;
} instruccion;
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
    int valor;
    int esCadena;  //booleano
    char cadena[DIM_COMANDO];
} Simbolos;

void magia();
void leeArchivo(Linea v[DIM_LINEACOMANDO], int Header[], int *cant, char[]);                 //lee del archivo asembler y lo traduce al binario
void creaComando(char comando[DIM_COMANDO], char comentario[DIM_COMENTARIO], Linea *linea);  //Obvio que crea el comando jaja
void corrigeComando(char comando[DIM_COMANDO], Linea *linea);                                //corrige el comando, es decir se fija si tiene un rotulo prefijado o no
void muestra(Linea v[DIM_LINEACOMANDO], int cant);
void compilaCodigo(Linea v[DIM_LINEACOMANDO], int Header[], int cant, instruccion ins[DIM_OPERACIONES], char[], int *);
void creaBinario(Linea linea[DIM_LINEACOMANDO], int Header[], Simbolos rotulos[], int, int cantOperaciones, char nombreArch[]);
void RecuperaInstruccion(Linea LineaActual, char instruccionActual[DIM_COMANDO], Simbolos simbolos[], int cant, char op1[], char op2[]);
long ArmaCodigo(int, int, char op1[], char op2[], Simbolos simbolos[], int, int *, int i);
void ArmaOperando(char op[DIM_COMANDO], int cantOperandos, int indice, int *valor, Simbolos s[], int, int *errorOp, int *tipo);
int BuscaRotulo(char op1[], Simbolos simbolos[CANT_CELDAS], int cantRotulos);
void VerificaDuplicado(char cadena[DIM_COMANDO], Simbolos simbolos[CANT_CELDAS], int n, int *duplicado);
void BuscaComa(char[], int *error);
void CorrigeBlancos(char cadena[DIM_COMANDO]);
void CargaConstanteEQU(char comando[DIM_COMANDO], Simbolos s[], int posActual);
int anytoint(char *s, char **out);
int OperandoIndirecto(char[], Simbolos s[], int, int *);
int ComandoValido(char comando[DIM_COMANDO], instruccion ins[DIM_OPERACIONES]);
void cargaInstrucciones(instruccion ins[DIM_OPERACIONES]);
int tieneHeader(char comando[DIM_COMANDO]);
void cargaParametrosHeader(int Header[5]);
void creaHeader(int Header[5], char *comando);  //en el char va el nombre del archivo (El Header puede estar en cualquier parte del codigo) para poder leer el header

int main(int argsCant, char *arg[])  //argsCant es cantidad de argumentos
{
    int errorSintaxis = 0, cant = 0;
    Linea Lineas[DIM_LINEACOMANDO];
    instruccion instrucciones[DIM_OPERACIONES];
    int Header[5];
    cargaInstrucciones(instrucciones);
    cargaParametrosHeader(Header);
    magia();
    if (argsCant >= 3) {
        leeArchivo(Lineas, Header, &cant, arg[1]);
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
void leeArchivo(Linea v[DIM_LINEACOMANDO], int Header[], int *cant, char ArchFuente[40]) {
    int i = 0;                        //posicion del caracter en la palabra (aclaracion: esto solo se utiliza en el codigo y no en las aclaraciones\comentarios)
    char caracter;                    //para leer un unico caracter del archivo
    char comando[DIM_COMANDO];        //variable que se utiliza para almacenar el comando
    char comentario[DIM_COMENTARIO];  //Variable que almacena el comentario
    FILE *arch;

    //arch = fopen("C:/Users/Augusto/Documents/Facultad/Arquitectura/MaquinaVirtual/Maquina-Virtual-Compilador-2/roto.asm", "rt");
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
            creaHeader(Header, comando);
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
        i = 0;
        while (comando[i] != '\0' && !(comando[i - 4] == ' ' && toupper(comando[i - 3]) == 'E' && toupper(comando[i - 2]) == 'Q' && toupper(comando[i - 1]) == 'U' && comando[i] == ' '))
            i++;
        if (comando[i] != '\0')  //Es una constante EQU si esto es verdadero
            linea->codigo = -2;
        corrigeComando(comando, linea);
    }
}
void corrigeComando(char comando[DIM_COMANDO], Linea *linea) {
    int i = 0;
    int j = 0;
    int k=0;
    char aux[DIM_COMENTARIO];
    CorrigeBlancos(comando);
    while (comando[i] != ':' && comando[i] != '\0')
        //  UTILIZA UN AUX PARA ALMACENAR LO QUE LLEGA DE COMANDO
        if ((int)comando[j - 1] != 39 && linea->codigo != -2)  //comilla simple o cadena EQU
            aux[j++] = toupper(comando[i++]);
        else
            aux[j++] = comando[i++];
    if (linea->codigo == -2){
        while (aux[k] != ' '){
            aux[k] = toupper(aux[k]); //rotulo va en mayus
            k++;
        }
    }
    k = 0;
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
        if (linea.codigo == -1) {
            printf("%s", linea.comentario);
            printf("\n");
        } else if (linea.codigo != -2) {
            if (strcmp(linea.rotulo, "0") == 0)
                sprintf(linea.rotulo, "%d", linea.codigo + 1);
            if (linea.hexa == 0xFFFFFFFF) {
                linea.codigo = 9999;
                strcpy(linea.rotulo, "----");
            }
            printf("[%04i]: [%02X %02X %02X %02X] | %8s | %s %s", linea.codigo, (linea.hexa >> 24) & 0xFF, (linea.hexa >> 16) & 0xFF, (linea.hexa >> 8) & 0xFF, (linea.hexa >> 0) & 0xFF, linea.rotulo, linea.comando, linea.comentario);
            printf("\n");
        }
    }
}
void compilaCodigo(Linea linea[DIM_LINEACOMANDO], int Header[], int cant, instruccion instrucciones[DIM_OPERACIONES], char nombreArch[40], int *error) {
    int errorOperando, duplicado, i, k, cantRotulos = 0, cantOpsValidas = 0;
    long codOp;
    char op1[DIM_COMANDO] = {'\0'};
    char op2[DIM_COMANDO] = {'\0'};
    char aux[DIM_COMANDO];
    int pos = 0;
    char instruccionActual[DIM_COMANDO];
    Simbolos simbolos[CANT_CELDAS];
    for (i = 0; i < cant; i++) {  //Carga de todos los rotulos
        if (linea[i].codigo != -1) {
            if (linea[i].rotulo[0] != '0') {
                VerificaDuplicado(linea[i].rotulo, simbolos, cantRotulos, &duplicado);
                if (!duplicado) {
                    strcpy(simbolos[cantRotulos].rotulo, linea[i].rotulo);
                    simbolos[cantRotulos].valor = cantOpsValidas;
                    simbolos[cantRotulos].esCadena = 0;
                    cantRotulos++;
                    cantOpsValidas++;
                } else {
                    printf("ERROR! El simbolo %s es un duplicado! \nFue declarado nuevamente en la linea: %d\n", linea[i].rotulo, i + 1);
                    *error = i;
                }
            } else if (linea[i].codigo == -2) {  //Constante EQU
                k = 0;
                while (linea[i].comando[k] != ' ' && linea[i].comando[k] != '	') {
                    aux[k] = linea[i].comando[k];
                    k++;
                }
                aux[k] = '\0';
                VerificaDuplicado(aux, simbolos, cantRotulos, &duplicado);
                if (!duplicado) {
                    CargaConstanteEQU(linea[i].comando, simbolos, cantRotulos);
                    cantRotulos++;
                } else {
                    printf("ERROR! El simbolo %s es un duplicado! %d \nFue declarado nuevamente en la linea: %d\n", aux, i + 1);
                    *error = i;
                }
            } else
                cantOpsValidas++;
        }
    }
    for (i = 0; i < cantRotulos; i++) {
        if (simbolos[i].esCadena == 1) {
            simbolos[i].valor = cantOpsValidas;
            cantOpsValidas = cantOpsValidas + strlen(simbolos[i].cadena) + 1;
        }
    }

    cantOpsValidas = 0;
    for (i = 0; i < cant; i++) {  //Analiza todas las instrucciones
        if (linea[i].codigo != -1 && linea[i].codigo != -2) {
            RecuperaInstruccion(linea[i], instruccionActual, simbolos, cantRotulos, op1, op2);
            pos = ComandoValido(instruccionActual, instrucciones);
            if (pos != -1) {
                errorOperando = 0;
                codOp = ArmaCodigo(instrucciones[pos].codigo, instrucciones[pos].operandos, op1, op2, simbolos, cantRotulos, &errorOperando, i + 1);
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
        creaBinario(linea, Header, simbolos, cantRotulos, cant, nombreArch);
}
void RecuperaInstruccion(Linea LineaActual, char instruccionActual[DIM_COMANDO], Simbolos simbolos[CANT_CELDAS], int cantRotulos, char op1[DIM_COMANDO], char op2[DIM_COMANDO]) {
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
            if (op2[0] != '-' && strlen(op2)>=3 && op2[0] != '[' && op2[0] != '#' && op2[0] != '@' && op2[0] != '%' && !(op2[1] == 'X' && op2[0] <= 'F' && op2[0] >= 'A') && !(op2[0] >= '0' && op2[0] <= '9') && strcmp(op2, "AC") != 0) {  //Es un rotulo
                pos = BuscaRotulo(op2, simbolos, cantRotulos);
                if (pos != -1) {
                    sprintf(op2, "%d", pos);  //Caracter que representa el numero
                } else {
                    //ROTULO NO ENCONTRADO! Error!
                    printf("\nERROR DE ROTULO NO ENCONTRADO: Rotulo: %s\n", op2);
                }
            }
        }
        //Verificar si el primer operando es un rotulo, en ese caso modificar el rotulo por el numero de linea correspondiente
        if (op1[0] != '-' && strlen(op1)>=3 && op1[0] != '[' && op1[0] != '#' && op1[0] != '@' && op1[0] != '%' && !(op1[1] == 'X' && op1[0] <= 'F' && op1[0] >= 'A') && !(op1[0] >= '0' && op1[0] <= '9') && strcmp(op1, "AC") != 0) {  //Es un rotulo
            pos = BuscaRotulo(op1, simbolos, cantRotulos);
            if (pos != -1) {
                sprintf(op1, "%d", pos);  //Caracter que representa el numero
            } else {
                //ROTULO NO ENCONTRADO! Error!
                printf("\nERROR DE ROTULO NO ENCONTRADO: Rotulo: %s\n", op1);
            }
        }
    }
}
long ArmaCodigo(int codigo, int cantOperandos, char op1[DIM_COMANDO], char op2[DIM_COMANDO], Simbolos simbolos[CANT_CELDAS], int cantRotulos, int *errorOp, int indice) {
    long codAux = 0;
    char aux2[DIM_COMANDO] = {0};
    int opA = 0, opB = 0;
    int tipoOp1 = 0, tipoOp2 = 0;
    ArmaOperando(op1, cantOperandos, indice, &opA, simbolos, cantRotulos, errorOp, &tipoOp1);
    ArmaOperando(op2, cantOperandos, indice, &opB, simbolos, cantRotulos, errorOp, &tipoOp2);
    if (cantOperandos == 2)
        codAux = codigo << 28 | ((tipoOp1 << 26) & 0xC000000) | ((tipoOp2 << 24) & 0x3000000) | ((opA << 12) & 0xFFF000) | (opB & 0xFFF);
    else if (cantOperandos == 1)
        codAux = codigo << 24 | ((tipoOp1 << 22) & 0xC00000) | (opA & 0xFFFF);
    else
        codAux = codigo << 20;
    return codAux;
}
void creaBinario(Linea linea[DIM_LINEACOMANDO], int Header[], Simbolos simbolos[CANT_CELDAS], int cantRotulos, int cantidadOperaciones, char nombreArch[]) {
    int cantidadValidas = 0, valorAscii, k, i;
    FILE *archivoSalida;
    archivoSalida = fopen(nombreArch, "wb");

    for (i = 0; i < cantidadOperaciones; i++)
        if (linea[i].codigo != -1 && linea[i].codigo != -2)
            cantidadValidas++;    //Cuenta la cantidad de instrucciones validas para colocar el valor en el header de CodeSegment
    for (i = 0; i < cantRotulos; i++) {
        if (simbolos[i].esCadena == 1)
            cantidadValidas += strlen(simbolos[i].cadena)+1;
    }
    Header[4] = cantidadValidas;  //Code Segment
     

    fwrite(&Header[0], sizeof(Header[0]), 1, archivoSalida);
    //En 0 es la cabecera MV21 no hace falta verificar
    for (i = 1; i < 5; i++) {
        if (Header[i] > 0)
            fwrite(&Header[i], sizeof(Header[i]), 1, archivoSalida);
        else {
            switch (i) {
                case 1:
                    printf("Error en header DATA - Tamano negativo\n");
                    break;
                case 2:
                    printf("Error en header STACK - Tamano negativo\n");
                    break;
                case 3:
                    printf("Error en header EXTRA - Tamano negativo\n");
                    break;
            }
            Header[i] = 1024;
            printf("Se reemplaza por 1024 y continua la traduccion\n");
            fwrite(&Header[i], sizeof(Header[i]), 1, archivoSalida);
        }
    }
    if (Header[1]+Header[2]+Header[3]+Header[4]>65535)
        printf("ERROR: La suma del tamano de los header supera el permitido por la arquitectura\n");

    for (i = 0; i < cantidadOperaciones; i++)
        if (linea[i].codigo != -1 && linea[i].codigo != -2)  //Si no es un solo comentario o una constante EQU
            fwrite(&linea[i].hexa, sizeof(linea[i].hexa), 1, archivoSalida);
    for (i = 0; i < cantRotulos; i++) {
        if (simbolos[i].esCadena == 1)
            for (k = 0; k <= strlen(simbolos[i].cadena); k++) {
                valorAscii = (int)simbolos[i].cadena[k];
                fwrite(&valorAscii, sizeof(int), 1, archivoSalida);
            }
    }

    fclose(archivoSalida);
}
int BuscaRotulo(char op1[], Simbolos simbolos[CANT_CELDAS], int cantRotulos) {
    int i = 0;
    while (i < cantRotulos && strcmp(op1, simbolos[i].rotulo) != 0)
        i++;
    if (i == cantRotulos)
        return -1;  //rotulo no encontrado;
    else
        return simbolos[i].valor;
}

void VerificaDuplicado(char cadena[DIM_COMANDO], Simbolos simbolos[CANT_CELDAS], int n, int *duplicado) {
    int k = 0;
    while (k < n && strcmp(simbolos[k].rotulo, cadena) != 0)
        k++;
    if (k == n)
        *duplicado = 0;
    else
        *duplicado = 1;
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
        while ((cadena[i] == ' ' || cadena[i] == '	') && (int)cadena[i - 1] != 39) {
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

int OperandoIndirecto(char aux[DIM_COMANDO], Simbolos simbolos[CANT_CELDAS], int cantRotulos, int *error) {
    char offsetString[20], registros[16][3];
    int resta = 0, offset, i, j, codigo = 0;
    char regActual[3];
    strcpy(registros[0], "DS");
    strcpy(registros[1], "SS");
    strcpy(registros[2], "ES");
    strcpy(registros[3], "CS");
    strcpy(registros[4], "HP");
    strcpy(registros[5], "IP");
    strcpy(registros[6], "SP");
    strcpy(registros[7], "BP");
    strcpy(registros[8], "CC");
    strcpy(registros[9], "AC");
    strcpy(registros[10], "AX");
    strcpy(registros[11], "BX");
    strcpy(registros[12], "CX");
    strcpy(registros[13], "DX");
    strcpy(registros[14], "EX");
    strcpy(registros[15], "FX");

    //Separar en partes el aux, primer buscar registro y luego offset
    regActual[0] = aux[0];
    regActual[1] = aux[1];
    regActual[2] = '\0';
    i = 0;
    while (i <= 15 && strcmp(registros[i], regActual) != 0) {
        i++;
    }
    if (i == 16) {
        *error = 1;
        return -1;
    } else {
        codigo = i;  //Codigo del registro que buscaba
        //Busca offset
        i = 0;
        while (aux[i] != '\0' && aux[i] != '+' && aux[i] != '-')
            i++;
        if (aux[i] == '\0')  //No hay offset
            offset = 0;
        else {
            resta = aux[i] != '+';  //Resta tiene 1 si el offset es una resta, y cero si es una suma
            i++;
            j = 0;
            while (aux[i] != '\0') {  //Recupera todo lo que haya despues del + o -
                offsetString[j] = aux[i];
                i++;
                j++;
            }

            offsetString[j + 1] = '\0';
            if (offsetString[0] >= '0' && offsetString[0] <= '9')
                offset = anytoint(offsetString, NULL);  //No es una constante (Es un numero entero)
            else {
                int pos = BuscaRotulo(offsetString, simbolos, cantRotulos);
                if (pos != -1)  //Existe el rótulo?
                    offset = pos;
                else {
                    printf("\nERROR DE ROTULO NO ENCONTRADO: Rotulo: %s\n", offsetString);
                    *error = 1;
                    offset = 0;
                }
            }
            if (resta)
                offset *= -1;  //cambia signo
        }
        return (((offset & 0xFF) << 4) | codigo);
    }
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
    strcpy(ins[12].inst, "SLEN");
    ins[12].codigo = 0xC;
    strcpy(ins[13].inst, "SMOV");
    ins[13].codigo = 0xD;
    strcpy(ins[14].inst, "SCMP");
    ins[14].codigo = 0xE;
    for (int i = 0; i <= 14; i++)
        ins[i].operandos = 2;
    //1 operando
    strcpy(ins[15].inst, "SYS");
    ins[15].codigo = 0xF0;
    strcpy(ins[16].inst, "JMP");
    ins[16].codigo = 0xF1;
    strcpy(ins[17].inst, "JZ");
    ins[17].codigo = 0xF2;
    strcpy(ins[18].inst, "JP");
    ins[18].codigo = 0xF3;
    strcpy(ins[19].inst, "JN");
    ins[19].codigo = 0xF4;
    strcpy(ins[20].inst, "JNZ");
    ins[20].codigo = 0xF5;
    strcpy(ins[21].inst, "JNP");
    ins[21].codigo = 0xF6;
    strcpy(ins[22].inst, "JNN");
    ins[22].codigo = 0xF7;
    strcpy(ins[23].inst, "LDL");
    ins[23].codigo = 0xF8;
    strcpy(ins[24].inst, "LDH");
    ins[24].codigo = 0xF9;
    strcpy(ins[25].inst, "RND");
    ins[25].codigo = 0xFA;
    strcpy(ins[26].inst, "NOT");
    ins[26].codigo = 0xFB;
    strcpy(ins[27].inst, "PUSH");
    ins[27].codigo = 0xFC;
    strcpy(ins[28].inst, "POP");
    ins[28].codigo = 0xFD;
    strcpy(ins[29].inst, "CALL");
    ins[29].codigo = 0xFE;

    for (int i = 15; i <= 29; i++)
        ins[i].operandos = 1;
    //0 operandos
    strcpy(ins[30].inst, "RET");
    ins[30].codigo = 0xFF0;
    ins[30].operandos = 0;

    strcpy(ins[31].inst, "STOP");
    ins[31].codigo = 0xFF1;
    ins[31].operandos = 0;
}
void ArmaOperando(char op[DIM_COMANDO], int cantOperandos, int indice, int *valor, Simbolos simbolos[CANT_CELDAS], int cantRotulos, int *errorOp, int *tipo) {
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
            if (op[i] != '\0' && (aux[0] >= '0' && aux[0] <= '9')) {
                aux[i] = '\0';
                *valor = anytoint(aux, NULL);
                *tipo = 2;
            } else if (op[i] != '\0' && (aux[0] >= 'A' && aux[0] <= 'F' || aux[0] == 'H' || aux[0] == 'I' || aux[0] == 'S')) {
                aux[i] = '\0';
                *tipo = 3;
                *valor = OperandoIndirecto(aux, simbolos, cantRotulos, errorOp);
            } else
                *errorOp = 1;                                       //hubo algun error (falta de ] o con el registro)
        } else if (op[0] >= 'A' && op[0] <= 'F' && op[1] == 'X') {  //Para pasar la letra a su valor decimal (A=10, B=11,...,F=15)
            *tipo = 1;
            *valor = op[0] - 55;
        } else if (strcmp(op, "AC") == 0) {
            *tipo = 1;
            *valor = 9;
         } else if (strcmp(op, "BP") == 0) {
            *tipo = 1;
            *valor =7;
         } else if (strcmp(op, "SP") == 0) {
            *tipo = 1;
            *valor = 6;
        } else if ((int)op[0] == 39) {  //Comilla simple '
            *valor = (int)op[1];
        } else {  // Es un inmediato
            if (op[strlen(op) - 1] != ']') {
                *tipo = 0;
                *valor = anytoint(op, NULL);
                if (op[0]== '%' || op[0]== '#' || op[0]== '@'){
                    if (cantOperandos == 2){
                        *valor = *valor << 20;
                        *valor = *valor >> 20;
                    } else {
                        *valor = *valor << 16;
                        *valor = *valor >> 16;
                    }
                }
            } else {  //Falta el [ (ERROR)
                *errorOp = 1;
            }
        }
        if (cantOperandos == 2) {
            if (*valor < -2048 || *valor > 2047) {  //-2048 a 2047
                valorRealOP = *valor << 20;
                valorRealOP = valorRealOP >> 20;
                if (*tipo != 3)
                    printf("\nAdvertencia EL VALOR EXCEDE EL LIMITE MAXIMO DE MEMORIA: linea: %i - Valor original: %d - Valor truncado a: %d \n", indice, *valor, valorRealOP);
            }
        } else if (cantOperandos == 1)
            if (*valor < -32768 || *valor > 32767) {  // -32768 a 32767
                valorRealOP = *valor << 16;
                valorRealOP = valorRealOP >> 16;
                if (*tipo != 3)
                    printf("\nAdvertencia EL VALOR EXCEDE EL LIMITE MAXIMO DE MEMORIA: linea: %i - Valor original: %d - Valor truncado a: %d \n", indice, *valor, valorRealOP);
            }
    }
}

void CargaConstanteEQU(char comando[DIM_COMANDO], Simbolos simbolos[CANT_CELDAS], int posActual) {
    char aux[DIM_COMANDO], valorConstante[DIM_COMANDO] = {'\0'};
    int j, i = 0;
    while (comando[i] != ' ' && comando[i] != '	') {
        aux[i] = comando[i];
        i++;
    }
    aux[i] = '\0';
    while (toupper(comando[i]) != 'U')  //Saltea la palabra EQU
        i++;
    i++;
    while (comando[i] == ' ' || comando[i] == '	')
        i++;

    //VERIFICAR ACA SI VIENE TEXTO O NUMERO (Pendiente)
    if (comando[i] != '"') {  //Es un numero
        j = 0;
        while (comando[i] != '\0') {
            valorConstante[j] = comando[i];
            i++;
            j++;
        }
        valorConstante[j] = '\0';
        int valorInt = anytoint(valorConstante, NULL);
        strcpy(simbolos[posActual].rotulo, aux);
        simbolos[posActual].valor = valorInt;
        simbolos[posActual].esCadena = 0;
    } else {  //Es una cadena de texto
        j = 0;
        i++;  //Saltea la primera "
        while (comando[i] != '"') {
            valorConstante[j] = comando[i];
            i++;
            j++;
        }
        valorConstante[j] = '\0';
        simbolos[posActual].esCadena = 1;
        strcpy(simbolos[posActual].rotulo, aux);
        strcpy(simbolos[posActual].cadena, valorConstante);
    }
}

void cargaParametrosHeader(int Header[5]) {
    Header[0] = 0x4D563231;
    for (int i = 1; i < 4; i++)
        Header[i] = 0x400;
}
void recuperaSegmento(int *pos, int *hexa, char idPalabra[DIM_COMANDO], char valor[DIM_COMANDO]) {
    *hexa = anytoint(valor, NULL);
    if (strcmp(idPalabra, "DATA") == 0) {
        *pos = 1;
    } else if (strcmp(idPalabra, "STACK") == 0) {
        *pos = 2;
    } else if (strcmp(idPalabra, "EXTRA") == 0) {
        *pos = 3;
    } else {  //error
        *pos = -1;
        *hexa = -1;
    }
}

void creaHeader(int Header[5], char *comando)  //en el char va el nombre del archivo (El Header puede estar en cualquier parte del codigo) para poder leer el header
{
    char aux[DIM_COMANDO], valor[DIM_COMANDO];
    int i = 0, j = 0, k = 0;
    int pos, hexa;
    while (comando[i] != ' ' && comando[i] != '	')
        i++;
    while (comando[i] != '\0') {
        if (comando[i] != '=')
            if (comando[i] != ' ' && comando[i] != '	') 
                aux[k++] = toupper(comando[i++]);
            else
                i++; //hay un blanco molestando al inicio
        else {
            i++;
            while (comando[i] != '\0' && comando[i] != ' ' && comando[i] != '	')
                valor[j++] = comando[i++];
            recuperaSegmento(&pos, &hexa, aux, valor);
            if (pos != -1)
                Header[pos] = hexa;
            else
                printf("Error en el header: '%s' no es valido\nse usara tamano de celda 1024 para los campos no especificados o con error\n", aux);

            //Inicializa de nuevo para siguiente vuelta
            for (int x = 0; x <= j; x++)
                valor[x] = '\0';
            for (int x = 0; x <= k; x++)
                aux[x] = '\0';
            k = 0;
            j = 0;
        }
    }
    Header[0] = 0x4D563231;
}

int tieneHeader(char comando[DIM_COMANDO]) {
    int i = 0;
    while (comando[i] != '\\' && comando[i] != '\0')
        i++;
    return (comando[i] != '\0' && comando[i + 1] == '\\');
}

void magia() {
    printf("|=======================================================================================|\n");
    printf("|-----------------------[>>>> Compilador MV 2021 - Grupo F <<<<]------------------------|\n");
    printf("|------------------------------------- VERSION 1.6.2 -----------------------------------|\n");
    printf("|=======================================================================================|\n\n");
}
    
    