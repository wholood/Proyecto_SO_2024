#ifndef _SORTMERGE_H
#define _SORTMERGE_H




typedef struct{
    unsigned int num_lineas;
    unsigned int tamano_largo;
    char* linea_larga;
    unsigned int tamano_corto;
    char* linea_corta;
} stats_t;

typedef struct{
    char* file;
    stats_t* estadisticas;
}argumentos;

typedef struct {
    char *archivo_1;
    char *archivo_2;
    char *arc;
} merge_args_t;

typedef struct Nodo{
    char* string;
    struct Nodo* siguiente;
} Nodo;

typedef struct {
    Nodo* inicio;
} Lista;


// API's



#endif
