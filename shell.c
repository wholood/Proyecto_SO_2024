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
    comando_1 = strtok(cadena, operador);
    comando_2 = strtok(NULL, operador);
    return comando_1;
}

bool comando_valido(){
    printf("entrada: %s\n", entrada);
    char * separacion_1= malloc(sizeof(char)*248);
    char * separacion_2= malloc(sizeof(char)*248);
    strcpy(separacion_1,entrada);
    strcpy(separacion_2,entrada);

    if(strcmp(validar_operador(separacion_1, "|"),entrada)!=0){
        printf("Se guardó: %s y %s\n", comando_1,comando_2);
        return true;
    }
    else if(strcmp(validar_operador(separacion_2, "&&"),entrada)!=0) {
        printf("Se guardó: %s y %s\n", comando_1,comando_2);
        return true;
    }
    else{//funcion que verifique si el comando único ingresado es válido.
        
        int exitStatus = system(comando_1);
        if(exitStatus != 0){ 
            printf("Error executing command: %s\n", comando_1);
            return false;
        }
        return true;
            
    }

     
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
    comando_1= malloc(sizeof(char)*248);
    comando_2= malloc(sizeof(char)*248);

    while(lectura()){
        //La lectura fue valida y tengo un comando correcto cargado.
        //se corre la ejecución correspondiente al comando.
    }
    printf("Hasta luego! \n");
    //Lectura fue salir del programa.
        
    return 0;
}