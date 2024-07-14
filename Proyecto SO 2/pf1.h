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


// API's



#endif
