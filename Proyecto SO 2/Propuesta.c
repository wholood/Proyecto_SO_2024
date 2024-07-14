#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
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

    for (int i = 0; i <cant_lineas; i++) {
        fprintf(archivo_salida, "%s\n", lineas[i]);
        free(lineas[i]);
    }
    fclose(archivo_salida);
    free(lineas);

    printf("This worker thread writes %u lineas to \"%s\"\n", stats->num_lineas, salida_nombre_archivo);
    pthread_exit(NULL);
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
        printf("\nHilo %d, nombre de archivo %s, cadena más larga %s, cadena más corta %s\n", i, argu[i].file, argu[i].estadisticas->linea_larga,argu[i].estadisticas->linea_corta);
    }
   
    sem_destroy(&mutex);

    return 0;
}
