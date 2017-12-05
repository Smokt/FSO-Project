//Threads in C

/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#define TAMBUFF 4		  //Tamaño buffer circular
#define NUM_HCALC 6		//Numero de Chacales
#define RUTAFICHD "./Datos.txt"

/* prototype for thread routine */
void cargaDatos ( void *ptr );

/* struct to hold data to be passed to a thread
   this shows how multiple data items can be passed to a thread */
typedef struct {
  int vector[256];
  int fila;
  //semaforo
}Buffer;
Buffer B[TAMBUFF];

int main()
{
    pthread_t hiloCarga;  //Creacion del hilo
    pthread_create (&hiloCarga, NULL, (void *) &cargaDatos, (void *) NULL);
    /*El main acaba */
    pthread_join(hiloCarga, NULL);
    printf("El hiloCarga ha termina'o...\n");
    exit(0);
}

/**
 * cargaDatos is used as the start routine for the threads used
 * it accepts a void pointer
**/
void cargaDatos ( void *ptr )
{
  int i,j;
  int var1;
  int aux[256];
	int cont=0;
	FILE *fp;
  if((fp=fopen(RUTAFICHD,"r"))==NULL)
	{
		fprintf(stderr,"No se pudo abrir el fichero de datos %s\n", RUTAFICHD);
		exit(-1);
	}
	while(1)
	{
    for(i = 0; i<256; i++)
    {
    fscanf(fp,"%d",&B[cont].vector[i]);
    }
    cont = (cont+1)%TAMBUFF;
    if(fp=='\0'){break;}
  }

  for(i = 0; i<TAMBUFF; i++){
    for(j = 0; j < 256; j++){
        printf("El contenido de la linea %d es %d",cont, B[i].vector[j]);
    }
    printf("\n");
  }

    pthread_exit(0); /* exit */
} /* print_message_function ( void *ptr ) */
