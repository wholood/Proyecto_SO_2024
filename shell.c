#include <stdio.h>
#include <stdbool.h> //trabajar con booleanos wtf????
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h> 
#include <malloc.h>
char *entrada, *comando_1, *comando_2;

char* validar_operador(char* cadena, char* operador){
    char *token = strtok(cadena, operador);
    do{
        printf("token: %s\n", token);
        return token;
    }while(token = strtok(NULL, operador));
    
}

bool comando_valido(){
    //(token = strtok(entrada, "|")) || (token = strtok(entrada, "&&"))
    printf("entrada: %s\n", entrada);
    char * separacion_1= malloc(sizeof(char)*248);
    char * separacion_2= malloc(sizeof(char)*248);

    strcpy(separacion_1,entrada);
    strcpy(separacion_2,entrada);

    printf("separación: %s\n", validar_operador(separacion_1, "|"));

    printf("2--entrada: %s\n", entrada);
    printf("separación: %s\n", validar_operador(separacion_2, "&&"));

    return true;
}

bool lectura(){
    while(true){
        printf("Leyendo\n");
        fgets(entrada, sizeof(char)*248, stdin);
        entrada = strtok(entrada, "\n");


        if(strcmp(entrada, "salir") == 0){ //Si escribe salir retorna falso para terminar.
            return false;
        }
        else if(comando_valido()){ //Validaciones de estructuras
            return true; //Tengo un comando que sirve así que se puede trabajar con él
        }
        //En caso de recibir ctrl+z o otra señal hay que hacer otro else para salir
        else{
            printf("Comando '%s' invalido. Por favor ingresar nuevo comando.\n",entrada);   
        }    
    }
    
    return false;
}


int main (){
    entrada = malloc(sizeof(char)*248);

    while(lectura()){
        //La lectura fue valida y tengo un comando correcto cargado.
        //se corre la ejecución correspondiente al comando.
    }
    printf("Hasta luego! \n");
    //Lectura fue salir del programa.
        
    return 0;
}