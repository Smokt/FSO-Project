//He añadido cosas como el segundo buffer y un principio de funcionalidad de los hilos calculadores
//los cuales tambien he creado (o algo asi). Te dejo todo con comentarios lo que he añadido o cambiado
//Aunque he cambiado solo el for ese.
//Si esto funciona quedaria meter los semaforos para asegurar el 7, es mas que probable que no
//por lo que bueno algo es algo, no he podido probarlo si quiera desde el pueblo :/
//asi que nada ya me contaras y sorry, no puse acentos por si casca lo del utf-8 xD

/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <math.h>       //Añado Math para calcular la raiz en la distancia
#define TAMBUFF 4		//Tamaño buffer circular
#define NUM_HCALC 6		//Numero de Chacales
#define RUTAFICHD "./Datos.txt"
#define RUTAFICHP "./Patron.txt"

/* prototype for thread routine */
void cargaDatos ( void *ptr );

void calcula( int numHil ); //Cabecera del metodo de los hilos calculadores

/* struct to hold data to be passed to a thread
   this shows how multiple data items can be passed to a thread */
typedef struct {
  int vector[256];
  int fila;
  //semaforo
}Buffer;
Buffer B[TAMBUFF];

typedef struct {
  double D;
  int fila;
  //semaforo
}Buffer2;
Buffer2 R[NUM_HCALC];
            //He definido el vector de numL y D que rellenan los calculadores como
						//un vector de estructuras de manera analoga al otro
						//El tamaño es lo unico que dudo, puse el de num max de hilos consumidores
int main()
{
	int i;
  pthread_t hiloCarga;  //Creacion del hilo

	pthread_t hiloCalcula[NUM_HCALC]; //Creo los hilos consumidores

  pthread_create (&hiloCarga, NULL, (void *) &cargaDatos, (void *) NULL);

	for(i=0;i<NUM_HCALC;i++){
        int *arg;
        if (malloc(sizeof(*arg)) == NULL) {
          printf("No se puedo reservar memoria para arg.\n");
          exit(-1);
        }//lo copie de un ejemplo pero se hace dentro del for para que cada thread reciba una zona de memoria diferente como arguento y no haya problemas cuando vayan a leer el dato
        *arg = i;
        pthread_create (&hiloCalcula[i], 0 , (void *) &calcula, arg);
	//Aquí se va creando cada hilo, habria que mirar como enviar a cada uno el numero concreto de         hilo que es
	//Nos hara mas tarde en el metodo calcula
  }

    /*El main acaba */
  pthread_join(hiloCarga, NULL);
	pthread_join(hiloCalcula[i], NULL); //Esto lo pongo pero Ni Puta Idea


  printf("El hiloCarga ha termina'o...\n");
  exit(0);
}

/**
 * cargaDatos is used as the start routine for the threads used
 * it accepts a void pointer
**/
void cargaDatos ( void *ptr )
{
  int i,j,k;
  int var1;
  int aux[256];
	int cont=0;
	FILE *fp;
  if((fp=fopen(RUTAFICHD,"r"))==NULL)
	{
		fprintf(stderr,"No se pudo abrir el fichero de datos %s\n", RUTAFICHD);
		exit(-1);
	}
	for(k=0;k<1025;k++)	//He puesto el for que te comente para que no haya bucle infinito (en teoria XD)
	{
    for(i = 0; i<256; i++)
    {
    fscanf(fp,"%d",&B[cont].vector[i]);
    }
    cont = (cont+1)%TAMBUFF;
  }

  for(i = 0; i<TAMBUFF; i++){
    for(j = 0; j < 256; j++){
        printf("El contenido de la linea %d es %d",cont, B[i].vector[j]);
    }
    printf("\n");
  }
	fclose(fp);
    pthread_exit(0); /* exit */
} /* print_message_function ( void *ptr ) */


//Aqui comienza el metodo de los hilos calculadores
void calcula(int numHil)
{
	typedef struct {
  int vector[256];
  int fila;
  //semaforo
  }Registro;

  Registro Reg;//Creo esta estructura que en verdad no hace falta, pero al principio
				//tenia otra cosa y dije bueno pues lo dejo, aunque se puede sacar de la struct

  int i;
  double resultado=0; // es double porque pow() devuelve double
  int aux;

  Reg.vector=B[numHil].vector;
  Reg.fila=B[numHil].fila;//Recojo en la estructura esta el contenido del buffer
  //Aqui recojo el dato de la posicion numHil, numHil debería ser lo que le pasamos al hilo que comente arriba

  printf("Se ha liberado la celda %d del buffer",Reg.fila);

	FILE *fp;
  if((fp=fopen(RUTAFICHP,"r"))==NULL)
	{
		fprintf(stderr,"No se pudo abrir el fichero de datos %s\n", RUTAFICHP);
		exit(-1);
	}//Abro el fichero patron
	for (i=0;i<256;i++)//Si he entendido bien el enunciado hay que calcular la resta entre posiciones
						//para cada uno de los 256 numeros que tenemos en el vector con los del patron
						//elevarlo al cuadrado y sumarlos todos, despues hacemos la raiz
						//si el calculo da problemas de tipos habria que castear
	{
		fscanf(fp,"%d",&aux);
		resultado=resultado+pow((aux-Reg.vector[i]),2);
	}
	resultado=sqrt(resultado);

  R[numHil].fil=Reg.fila;//En el nuevo buffer añado la fila
	R[numHil].D=resultado; //En el nuevo buffer añado la Distancia
	//numHil sera lo que le enviamos que ya te comente arriba
}
