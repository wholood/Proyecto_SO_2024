#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "pf1.h"

#define MAX_LINE_LENGTH 1024
#define INITIAL_CAPACITY 10
int total_procesado =0;
char* linea_corta;
char* linea_larga;
sem_t mutex;

int comparar(const void *a, const void *b) {
    return strcasecmp(*(const char **)b, *(const char **)a);
}

void sort_final(){
    int tamano_largo=0, tamano_corto=0;
    linea_corta = malloc(MAX_LINE_LENGTH * sizeof(char*));
    linea_larga = malloc(MAX_LINE_LENGTH * sizeof(char*));
    
    FILE* archivo = fopen("sorted.txt", "r");
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
        if (tamano_linea > tamano_largo) {
            tamano_largo = tamano_linea;
            linea_larga = strdup(buffer);
        }
        if (tamano_corto == 0 || tamano_linea < tamano_corto) {
            tamano_corto = tamano_linea;
            linea_corta = strdup(buffer);
        }
        cant_lineas++;
    }
    fclose(archivo);

    qsort(lineas, cant_lineas, sizeof(char*), comparar);

    FILE* archivo_salida = fopen("sorted.txt", "w");
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
    total_procesado+=cant_lineas;

    qsort(lineas, cant_lineas, sizeof(char*), comparar);

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

    printf("This worker thread writes %u lines to \"%s\"\n", stats->num_lineas, salida_nombre_archivo);
    pthread_exit(NULL);
}

int existe_linea(char** lineas, int num_lineas, const char* linea) {
    for (int i = 0; i < num_lineas; i++) {
        if (strcasecmp(lineas[i], linea) == 0) {
            return 1;
        }
    }
    return 0;
}


void* merge_archivos(void* arg) {
    merge_args_t* args = (merge_args_t*) arg;
    FILE* archivo_1 = fopen(args->archivo_1, "r");
    FILE* archivo_2 = fopen(args->archivo_2, "r");
    FILE* output = fopen(args->arc, "w");

    if (!archivo_1) {
        printf("Error al abrir archivo 1 para unir.\n");
        pthread_exit(NULL);
    }
    else if (!archivo_2) {
        printf("Error al abrir archivo 2 para unir.\n");
        pthread_exit(NULL);
    }
    else if (!output) {
        printf("Error al abrir archivo output para unir.\n");
        pthread_exit(NULL);
    }

    char** lineas_unicas = malloc(INITIAL_CAPACITY * sizeof(char*));
    int lineas_unicas_cont = 0;
    int lineas_unicas_capacidad = INITIAL_CAPACITY;

    char linea_1[MAX_LINE_LENGTH], linea_2[MAX_LINE_LENGTH];
    char* linea_1_ptr = fgets(linea_1, sizeof(linea_1), archivo_1);
    char* linea_2_ptr = fgets(linea_2, sizeof(linea_2), archivo_2);

    int total_lineas_archivo_1 = 0, total_lineas_archivo_2 = 0, total_lineas_salida = 0;
    
    while (linea_1_ptr || linea_2_ptr) { 
        //mientras haya lineas que leer en almenos un archivo (si los punteros no son nulos)

        //Elimina el caracter extra de cada linea siempre y cuando se haya leido una línea válida (el puntero no nulo)
        if (linea_1_ptr && linea_1[strlen(linea_1) - 1] == '\n') linea_1[strlen(linea_1) - 1] = '\0';
        if (linea_2_ptr && linea_2[strlen(linea_2) - 1] == '\n') linea_2[strlen(linea_2) - 1] = '\0';

        char* linea_candidata = NULL; //Se crea un string donde guardar al candidato

        //Si linea 2 es nulo o linea 1 es válida y menor que linea 2
        if (!linea_2_ptr || (linea_1_ptr && strcasecmp(linea_1, linea_2) < 0)) {
            //linea 1 se escribe en el archivo de salida, se lee la sig linea de archivo 1 y se incrementa el contador de lineas de ese archivo
            linea_candidata = strdup(linea_1);
            linea_1_ptr = fgets(linea_1, sizeof(linea_1), archivo_1);
            total_lineas_archivo_1++;
        } 
        //Si linea 1 es nulo o linea 2 es válida y menor que linea 1
        else if (!linea_1_ptr || (linea_2_ptr && strcasecmp(linea_2, linea_1) < 0)) {
            //escribo linea 2
            linea_candidata = strdup(linea_2);
            linea_2_ptr = fgets(linea_2, sizeof(linea_2), archivo_2);
            total_lineas_archivo_2++;
        
        }
        //si ambas lineas son iguales se escribe una sola y se vuelve a leer los archivos
        else {
            linea_candidata = strdup(linea_1);
            linea_1_ptr = fgets(linea_1, sizeof(linea_1), archivo_1);
            linea_2_ptr = fgets(linea_2, sizeof(linea_2), archivo_2);
            total_lineas_archivo_1++;
            total_lineas_archivo_2++;
        }

        //Si la linea candidata no es nulo y no está en la lista de lineas únicas entonces
        if (linea_candidata && !existe_linea(lineas_unicas, lineas_unicas_cont, linea_candidata)) {
            //Si el numero de linea es mayor a la cantidad máxima de la lista entonces necesito extender la lista 
            if (lineas_unicas_cont >= lineas_unicas_capacidad) {
                //Aparto más espacio de memoria para las lineas
                lineas_unicas_capacidad *= 10;
                lineas_unicas = realloc(lineas_unicas, lineas_unicas_capacidad * sizeof(char*));
                if (!lineas_unicas) {
                    perror("Error al reasignar memoria para líneas únicas");
                    exit(EXIT_FAILURE);
                }
            }
            //Añado la linea candidata a la lista de únicas e incremento el contador
            lineas_unicas[lineas_unicas_cont++] = strdup(linea_candidata);
        }
    }

    //Una vez recorrí los dos archivos se añade la lista de lineas únicas al archivo de salida, incrementando el total de lineas y liberando el espacio
    for (int i = 0; i < lineas_unicas_cont; i++) {
        fprintf(output, "%s\n", lineas_unicas[i]);
        total_lineas_salida++;
        free(lineas_unicas[i]);
    }

    free(lineas_unicas);
    fclose(archivo_1);
    fclose(archivo_2);

    fclose(output);

    sem_wait(&mutex);
    printf("Merged %d lines and %d lines into %d lines\n", total_lineas_archivo_1, total_lineas_archivo_2, total_lineas_salida);
    sem_post(&mutex);

    pthread_exit(NULL);
}

void multi_merge(char** archivos, int num_archivos) {
    //Mientras que el numero de archivos a procesar sea mayor que uno
    while (num_archivos > 1) {
        //Es un indice al siguiente número de archivos a procesar
        int next_num_archivos = 0;
        //Esto garantiza que el número de hilos sea coherente con una cantidad impar de archivos
        int num_hilos = num_archivos / 2 + num_archivos % 2;
        pthread_t merge_hilos[num_hilos];
        merge_args_t merge_args[num_hilos];

        //Repaso cada par de archivos hasta el final
        for (int i = 0; i < num_archivos; i += 2) {
            //Si en efecto existe un par de archivos disponibles (i y i+1)
            if (i + 1 < num_archivos) {
                //Se añaden como argumentos para el hilo
                merge_args[next_num_archivos].archivo_1 = archivos[i];
                merge_args[next_num_archivos].archivo_2 = archivos[i + 1];

                //L_tmpnam especifica el tamaño mínimo del array que debe pasarse a tmpnam para almacenar un nombre de archivo temporal.
                char temp_nombrearchivo[L_tmpnam];
                //Genera un archivo temporal con ese nombre
                tmpnam(temp_nombrearchivo);
                //Lo añade como archivo de salida
                merge_args[next_num_archivos].arc = strdup(temp_nombrearchivo);

                //Crea el hilo para ese par de archivos enviandole el argumento correspondiente
                if (pthread_create(&merge_hilos[next_num_archivos], NULL, merge_archivos, &merge_args[next_num_archivos])) {
                    printf("Error al crear thread de merge.\n");
                    exit(EXIT_FAILURE);
                }
                //Añade el nombre archivo temporal al arreglo de archivos para ser procesado en otra iteración
                //Incrementa el indice del numero de archivos
                archivos[next_num_archivos++] = strdup(merge_args[next_num_archivos].arc);
            } else {
                // Si hay un archivo impar, simplemente se coloca para la siguiente iteración.
                archivos[next_num_archivos++] = archivos[i];
            }
        }

        //espera a que los hilos de este nivel terminen incluyendo si son impares. 
        for (int i = 0; i < next_num_archivos - num_archivos % 2; i++) {
            pthread_join(merge_hilos[i], NULL);
        }
        
        //Hacemos que el numero de archivos sea igual al nuevo indice que establecimos
        num_archivos = next_num_archivos;
    }
    //En la segunda iteración del while el arreglo de archivos debe estar repleto de nombres temporales y el num_archivos reducido a la mitad

    //Si solo queda un archivo significa que ya todos los temporales se unieron
    if (num_archivos == 1) {
        if (rename(archivos[0], "sorted.txt") != 0) {
            perror("Error al renombrar el archivo final");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Error: No se generó un archivo final único.\n");
    }

    sort_final();
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

    char* sorted_archivos[num_archivos];
    for (int i = 0; i < num_archivos; i++) {
        sorted_archivos[i] = malloc(MAX_LINE_LENGTH * sizeof(char));
        snprintf(sorted_archivos[i], MAX_LINE_LENGTH, "%s.sorted", argv[i +1]);
    }

    multi_merge(sorted_archivos, num_archivos);

    for (int i = 0; i < num_archivos; i++) {
        free(argu[i].estadisticas->linea_corta);
        free(argu[i].estadisticas->linea_larga);
    }
    printf("A total of %d strings were passed as input\n", total_procesado);
    printf("longest string sorted: %s\n", linea_larga);
    printf("shortest string sorted: %s\n", linea_corta);
    
    sem_destroy(&mutex);

    return 0;
}
