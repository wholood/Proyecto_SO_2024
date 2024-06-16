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
int n_argumentos=0;
int pipe_errores[2];


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
        printf("Argumento %d: %s\n", n_argumentos, argumentos[n_argumentos]);
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
        waitpid(hijo, NULL, 0);
        
        char * cod_error = malloc(sizeof(char));
        
        close(pipe_errores[Escritura]);
        read(pipe_errores[Lectura], cod_error, sizeof(char)*2); //Sin errores
        close(pipe_errores[Lectura]);  
        
        if(strcmp(cod_error,"1")==0){
            printf("Padre indica error en: %s", argumentos[0]);
            return false;
        }
        else{
            return true; 
        }   
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

        separar_argumentos(linea_1);
        if(!comando_valido(linea_1)){
            separar_argumentos(linea_2);
            printf("error linea 1");
            return comando_valido(linea_2);
        }

        printf("salio bien el or");
        return true;
    }
    else if(strcmp(separar_operador(separacion_2, "&&"),entrada)!=0) {
        identificar_operador(linea_operador);
        
        separar_argumentos(linea_1);
        if(comando_valido(linea_1)){
            separar_argumentos(linea_2);
            //printf("linea 1 sirvió");
            return comando_valido(linea_2);
        }

        printf("\nerror de and");
        return false;
    }
    else{//funcion que verifique si el comando único ingresado es válido.
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


    while(lectura()){};
        //La lectura fue valida y tengo un comando correcto cargado.
        //se corre la ejecución correspondiente al comando.
    printf("Hasta luego! \n");
    //Lectura fue salir del programa.
        
    return 0;
}