#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "pf1.h"

#define MAX_LINE_LENGTH 1024
sem_t mutex;

int compare(const void *a, const void *b) {
    return strcasecmp(*(const char **)b, *(const char **)a);
}

void* sort_file(void* arg) {
    argumentos* datos = (argumentos*) arg;
    char* nombre_archivo=datos->file;
    stats_t* stats = datos->estadisticas;

    FILE* archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo");
        pthread_exit(NULL);
    }

    char** lineas = NULL;
    char buffer[MAX_LINE_LENGTH];
    unsigned int cant_lineas = 0, capacidad = 10;
    lineas = (char**)malloc(capacidad * sizeof(char*));

    if (lineas == NULL) {
        printf("Error al asignar memoria");
        fclose(archivo);
        pthread_exit(NULL);
    }

    while (fgets(buffer, sizeof(buffer), archivo)) {
        size_t tamano_linea = strlen(buffer);
        if (tamano_linea > 0 && buffer[tamano_linea - 1] == '\n') {
            buffer[tamano_linea - 1] = '\0';
            tamano_linea--;
        }
        if (tamano_linea == 0) {
            continue;
        }
        if (cant_lineas >= capacidad) {
            capacidad *= 2;
            lineas = (char**)realloc(lineas, capacidad * sizeof(char*));
            if (lineas == NULL) {
                printf("Error al reasignar memoria");
                fclose(archivo);
                pthread_exit(NULL);
            }
        }
        lineas[cant_lineas] = strdup(buffer);
        if (lineas[cant_lineas] == NULL) {
            printf("Error al duplicar cadena");
            fclose(archivo);
            pthread_exit(NULL);
        }
        
        if (tamano_linea > stats->tamano_largo) {
            stats->tamano_largo = tamano_linea;
            stats->linea_larga = strdup(buffer);
        }
        if (stats->tamano_corto == 0 || tamano_linea < stats->tamano_corto) {
            stats->tamano_corto = tamano_linea;
            stats->linea_corta = strdup(buffer);
        }
        cant_lineas++;
    }
    fclose(archivo);

    stats->num_lineas = cant_lineas;
    qsort(lineas, cant_lineas, sizeof(char*), compare);

    char salida_nombre_archivo[256];
    snprintf(salida_nombre_archivo, sizeof(salida_nombre_archivo), "%s.sorted", nombre_archivo);
    FILE* archivo_salida = fopen(salida_nombre_archivo, "w");
    if (archivo_salida == NULL) {
        printf("Error al abrir el archivo de salida");
        for (int i = 0; i < cant_lineas; i++) {
            free(lineas[i]);
        }
        free(lineas);
        pthread_exit(NULL);
    }

    for (int i = 0; i < cant_lineas; i++) {
        fprintf(archivo_salida, "%s\n", lineas[i]);
        free(lineas[i]);
    }
    fclose(archivo_salida);
    free(lineas);

    printf("This worker thread writes %u lineas to \"%s\"\n", stats->num_lineas, salida_nombre_archivo);
    pthread_exit(NULL);
}

void Lista_iniciar(Lista* Lista) {
    Lista->inicio = NULL;
}

bool Lista_buscar(Lista* Lista, const char* valor) {
    Nodo* actual = Lista->inicio;
    while (actual) {
        if (strcasecmp(actual->string, valor) == 0) {
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

void Lista_add(Lista* Lista, const char* valor) {
    Nodo* nuevo_Nodo = (Nodo*)malloc(sizeof(Nodo));
    nuevo_Nodo->string = strdup(valor);
    nuevo_Nodo->siguiente = Lista->inicio;
    Lista->inicio = nuevo_Nodo;
}

void Lista_free(Lista* Lista) {
    Nodo* actual = Lista->inicio;
    while (actual) {
        Nodo* siguiente = actual->siguiente;
        free(actual->string);
        free(actual);
        actual = siguiente;
    }
    Lista->inicio = NULL;
}

void* merge_files(void* arg) {
    merge_args_t* args = (merge_args_t*) arg;
    FILE* archivo_1 = fopen(args->archivo_1, "r");
    FILE* archivo_2 = fopen(args->archivo_2, "r");
    FILE* output = fopen(args->arc, "w");

    if (!archivo_1 || !archivo_2 || !output) {
        printf("Error al abrir archivos para unir.\n");
        pthread_exit(NULL);
    }

    Lista lista;
    Lista_iniciar(&lista);

    char linea_1[MAX_LINE_LENGTH], linea_2[MAX_LINE_LENGTH];
    char* linea_1_ptr = fgets(linea_1, sizeof(linea_1), archivo_1);
    char* linea_2_ptr = fgets(linea_2, sizeof(linea_2), archivo_2);

    int total_lineas_archivo_1 = 0, total_lineas_archivo_2 = 0, total_lineas_salida = 0;
    
    while (linea_1_ptr || linea_2_ptr) { //mientras haya lineas que leer en almenos un archivo (si los punteros no son nulos)

        //Elimina el caracter extra de cada linea siempre y cuando se haya leido una línea válida (el puntero no nulo)
        if (linea_1_ptr && linea_1[strlen(linea_1) - 1] == '\n') linea_1[strlen(linea_1) - 1] = '\0';
        if (linea_2_ptr && linea_2[strlen(linea_2) - 1] == '\n') linea_2[strlen(linea_2) - 1] = '\0';

        //Si linea 2 es nulo o linea 1 es válida y menor que linea 2
        if (!linea_2_ptr || (linea_1_ptr && strcasecmp(linea_1, linea_2) < 0)) {
            //linea 1 se escribe en el archivo de salida, se lee la sig linea de archivo 1 y se incrementa el contador de lineas de ese archivo
            if (!Lista_buscar(&lista, linea_1)) {
                fprintf(output, "%s\n", linea_1);
                Lista_add(&lista, linea_1);
            }
            linea_1_ptr = fgets(linea_1, sizeof(linea_1), archivo_1);
            total_lineas_archivo_1++;
        } 
        //Si linea 1 es nulo o linea 2 es válida y menor que linea 1
        else if (!linea_1_ptr || (linea_2_ptr && strcasecmp(linea_2, linea_1) < 0)) {
            //escribo linea 2
            if (!Lista_buscar(&lista, linea_2)) {
                fprintf(output, "%s\n", linea_2);
                Lista_add(&lista, linea_1);
            }
            
            linea_2_ptr = fgets(linea_2, sizeof(linea_2), archivo_2);
            total_lineas_archivo_2++;
        
        }
        //si ambas lineas son iguales se escribe una sola y se vuelve a leer los archivos
        else {
            if (!Lista_buscar(&lista, linea_1)) {
                fprintf(output, "%s\n", linea_1);
                Lista_add(&lista, linea_1);
            }
            linea_1_ptr = fgets(linea_1, sizeof(linea_1), archivo_1);
            linea_2_ptr = fgets(linea_2, sizeof(linea_2), archivo_2);
            total_lineas_archivo_1++;
            total_lineas_archivo_2++;
        }
        total_lineas_salida++;
    }

    fclose(archivo_1);
    fclose(archivo_2);
    fclose(output);

    sem_wait(&mutex);
    printf("Se unió \"%s\" (%d lineas totales) y \"%s\" (%d lineas totales) en \"%s\" (%d lineas archivo final)\n", args->archivo_1, total_lineas_archivo_1, args->archivo_2, total_lineas_archivo_2, args->arc, total_lineas_salida);
    

    Lista_free(&lista);

    pthread_exit(NULL);sem_post(&mutex);
}

void iterative_merge(char** files, int num_files) {
    while (num_files > 1) {
        int siguiente_num_files = 0;
        pthread_t merge_threads[num_files / 2];
        merge_args_t merge_args[num_files / 2];

        for (int i = 0; i < num_files; i += 2) {
            if (i + 1 < num_files) {
                merge_args[siguiente_num_files].archivo_1 = files[i];
                merge_args[siguiente_num_files].archivo_2 = files[i + 1];
                merge_args[siguiente_num_files].arc = tmpnam(NULL);
                if (pthread_create(&merge_threads[siguiente_num_files], NULL, merge_files, &merge_args[siguiente_num_files])) {
                    printf("Error al crear thread de merge.\n");
                    exit(EXIT_FAILURE);
                }
                files[siguiente_num_files++] = strdup(merge_args[siguiente_num_files].arc);
            } else {
                files[siguiente_num_files++] = files[i];
            }
        }

        for (int i = 0; i < siguiente_num_files; i++) {
            pthread_join(merge_threads[i], NULL);
        }

        num_files = siguiente_num_files;
    }

    if (num_files == 1) {
        if (rename(files[0], "sorted.txt") != 0) {
            perror("Error al renombrar el archivo final");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Error: No se generó un archivo final único.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Faltan argumentos\n");
        exit(EXIT_FAILURE);
    }

    int num_archivos = argc - 1;
    pthread_t threads[num_archivos];
    stats_t stats[num_archivos];
    argumentos argu[num_archivos];

    sem_init(&mutex,0,1);

    for (int i = 0; i < num_archivos; i++) {
        stats[i].num_lineas = 0;
        stats[i].tamano_largo = 0;
        stats[i].tamano_corto = 0;
        stats[i].linea_corta = malloc(MAX_LINE_LENGTH * sizeof(char*));
        stats[i].linea_larga = malloc(MAX_LINE_LENGTH * sizeof(char*));
        argu[i].file = malloc(MAX_LINE_LENGTH * sizeof(char*));

        argu[i].file= argv[i + 1];
        argu[i].estadisticas=&stats[i];
        
        if (pthread_create(&threads[i], NULL, sort_file, (void*)&argu[i])) {
            fprintf(stderr, "Error al crear el thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_archivos; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_archivos; i++) {
        printf("\nHilo %d, nombre de archivo %s, cadena más larga %s, cadena más corta %s\n", i, argu[i].file, argu[i].estadisticas->linea_larga, argu[i].estadisticas->linea_corta);
    }

    char* sorted_files[num_archivos];
    for (int i = 0; i < num_archivos; i++) {
        sorted_files[i] = malloc(MAX_LINE_LENGTH * sizeof(char));
        snprintf(sorted_files[i], MAX_LINE_LENGTH, "%s.sorted", argv[i +1]);
    }

    iterative_merge(sorted_files, num_archivos);

    /*for (int i = 0; i < num_archivos; i++) {
        free(sorted_files[i]);
        free(argu[i].estadisticas->linea_corta);
        free(argu[i].estadisticas->linea_larga);
    }*/

    sem_destroy(&mutex);

    return 0;
}
