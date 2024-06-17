#include <stdio.h>
#include <stdbool.h> //trabajar con booleanos wtf????
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h> 
#include <malloc.h>

#define Lectura 0
#define Escritura 1

char *entrada, *linea_1, *linea_2, *comando, *argumentos[100];
char *operador=NULL;
int n_argumentos=0, estado;
int pipe_errores[2], pipe_comunicacion[2];


void identificar_operador(char* linea){
    char* tok = strtok(linea," \t");
    while (tok != NULL){
        if(strcmp(tok,"||")==0){
            strcpy(operador,tok);
        }
        else if(strcmp(tok,"&&")==0){
            strcpy(operador,tok);
        }
        else if(strcmp(tok,"|")==0){
            strcpy(operador,tok);
        }
        tok = strtok(NULL, " \t");
    }
}

char* separar_operador(char* cadena, char* operador){
    linea_1 = strtok(cadena, operador);
    linea_2 = strtok(NULL, operador);
    return linea_1;
}

void separar_argumentos(char* linea){
    n_argumentos=0;
    char *arg = strtok(linea, " ");
    
    do{
        argumentos[n_argumentos]=malloc(sizeof(char)*248);
        strcpy(argumentos[n_argumentos],arg);
        n_argumentos++;
    }while(arg=strtok(NULL, " "));

}

bool comando_valido(){
    pid_t hijo;

    if(pipe(pipe_errores)==-1){
        return false;
    }
    hijo = fork();
    
    if(hijo < 0){
        return false;
    }
    else if(hijo==0){
        if(n_argumentos == 1){
            if(execvp(argumentos[0], NULL)== -1){ 
                
                close(pipe_errores[Lectura]);
                write(pipe_errores[Escritura],"1", sizeof(char)*2);
                close(pipe_errores[Escritura]);

                exit(-1);
            }
        }else{
            if(execvp(argumentos[0], argumentos)== -1){
                
                close(pipe_errores[Lectura]);
                write(pipe_errores[Escritura],"1", sizeof(char)*2);
                close(pipe_errores[Escritura]);

                exit(-1);
            } 
        }
        exit(0);    
    }
    else{
        waitpid(hijo, &estado, 0);
        
        char * cod_error = malloc(sizeof(char)*2);
        
        close(pipe_errores[Escritura]);
        read(pipe_errores[Lectura], cod_error, sizeof(char)*2); //Sin errores
        close(pipe_errores[Lectura]);  
        
        if(strcmp(cod_error,"1")==0 || estado!=0){
            printf("Padre indica error en: %s", argumentos[0]);
            return false;
        }
        else{
            return true; 
        }   
    }
}

bool pipes(int n_comando){
    pid_t hijo;
    if(pipe(pipe_errores)==-1){
        return false;
    }
    
    
    //----------PROCESOS HIJOS
    hijo = fork();
    
    if(hijo < 0){
        return false;
    }
    else if(hijo==0){
        if(n_comando==1){//abro y ejecuto el primer comando
            printf("\ncomando 1\n");
            close(pipe_comunicacion[Lectura]);
            dup2(pipe_comunicacion[Escritura],STDOUT_FILENO);
            close(pipe_comunicacion[Escritura]);

            if(n_argumentos == 1){
                if(execvp(argumentos[0], NULL)== -1){ 
                    close(pipe_errores[Lectura]);
                    write(pipe_errores[Escritura],"1", sizeof(char)*2);
                    close(pipe_errores[Escritura]);
                    
                    exit(-1);
                }
            }else{
                if(execvp(argumentos[0], argumentos)== -1){
                    close(pipe_errores[Lectura]);
                    write(pipe_errores[Escritura],"1", sizeof(char)*2);
                    close(pipe_errores[Escritura]);
                    exit(-1);
                } 
            }
            exit(0);    
        }
        if(n_comando==2){//abro y ejecuto el segundo comando
            printf("\ncomando 2\n");
            char *argumentos_C1 = malloc(sizeof(char)*248);
            close(pipe_comunicacion[Escritura]);
            dup2(pipe_comunicacion[Lectura],STDIN_FILENO); //Sin errores
            close(pipe_comunicacion[Lectura]);

            char *arg = strtok(argumentos_C1, " ");
    
            do{
                argumentos[n_argumentos]=malloc(sizeof(char)*248);

                strcpy(argumentos[n_argumentos],arg);
                n_argumentos++;
            }while(arg=strtok(NULL, " "));


            if(n_argumentos == 1){
                if(execvp(argumentos[0], NULL)== -1){ 
                    
                    close(pipe_errores[Lectura]);
                    write(pipe_errores[Escritura],"1", sizeof(char)*2);
                    close(pipe_errores[Escritura]);

                    exit(-1);
                }
            }else{
                if(execvp(argumentos[0], argumentos)== -1){
                    
                    close(pipe_errores[Lectura]);
                    write(pipe_errores[Escritura],"1", sizeof(char)*2);
                    close(pipe_errores[Escritura]);

                    exit(-1);
                } 
            }
            exit(0);    
        }
    } 
    else{
        waitpid(hijo, &estado, 0);
        
        char * cod_error = malloc(sizeof(char)*2);
        
        close(pipe_errores[Escritura]);
        read(pipe_errores[Lectura], cod_error, sizeof(char)*2); //Sin errores
        close(pipe_errores[Lectura]);  
        
        if(strcmp(cod_error,"1")==0 || estado!=0){
            printf("Padre indica error en: %s", argumentos[0]);
            return false;
        }
        else{
            return true; 
        }   
    }

}

bool ejecucion(){
    if(strcmp(operador,"||")==0){ //si el operador es or
        separar_argumentos(linea_1);
        if(!comando_valido(linea_1)){
            separar_argumentos(linea_2);
            printf("\nerror linea 1 or\n");
            return comando_valido(linea_2);
        }

        return true;    
    }
    else if(strcmp(operador,"|")==0){
        printf("Ejecuto un pipe\n");
        separar_argumentos(linea_1);
        bool resultado_comando_1 = pipes(1); //pipe recibe el numero del
        printf("Llamando a pipe2\n");
        separar_argumentos(linea_2);
        bool resultado_comando_2 = pipes(2);
    }
    else if(strcmp(operador,"&&")==0){
        separar_argumentos(linea_1);
        if(comando_valido(linea_1)){
            separar_argumentos(linea_2);
            return comando_valido(linea_2);
        }

        printf("\nerror de and\n");
        return false;
    }
}

bool entrada_valida(){
    printf("entrada: %s\n", entrada);
    char * separacion_1= malloc(sizeof(char)*248);
    char * separacion_2= malloc(sizeof(char)*248);
    char * linea_operador= malloc(sizeof(char)*248);
    strcpy(separacion_1,entrada);
    strcpy(separacion_2,entrada);
    strcpy(linea_operador,entrada);

    if(strcmp(separar_operador(separacion_1, "||"),entrada)!=0){
        identificar_operador(linea_operador);
        return ejecucion();
        
    }
    else if(strcmp(separar_operador(separacion_2, "&&"),entrada)!=0) {
        identificar_operador(linea_operador);
        return ejecucion();
        
    }
    else{
        separar_argumentos(linea_1);
        return comando_valido();   

    }

     
}

bool lectura(){
    while(true){
        printf("-----Leyendo\n");
        fgets(entrada, sizeof(char)*248, stdin);
        entrada = strtok(entrada, "\n");

        if(strcmp(entrada, "salir") == 0){ //Si escribe salir retorna falso para terminar.
            return false;
        }
        else if(entrada_valida()){ //Validaciones de estructuras
            return true; //Tengo un comando que sirve así que se puede trabajar con él
        }
        //En caso de recibir ctrl+z o otra señal hay que hacer otro else para salir
        else{
            printf("\nComando '%s' invalido. Por favor ingresar nuevo comando.\n",entrada);   
        }    
    }
    
    return false;
}

int main (){
    
    entrada = malloc(sizeof(char)*248);
    linea_1= malloc(sizeof(char)*248);
    linea_2= malloc(sizeof(char)*248);
    operador = malloc(sizeof(char)*4);
    if(pipe(pipe_comunicacion)==-1){
        return false;
    }


    while(lectura()){};
    printf("Hasta luego! \n");
    //Lectura fue salir del programa.
        
    return 0;
}