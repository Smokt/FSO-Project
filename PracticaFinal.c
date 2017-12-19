//He añadido cosas como el segundo buffer y un principio de funcionalidad de los hilos calculadores
//los cuales tambien he creado (o algo asi). Te dejo todo con comentarios lo que he añadido o cambiado
//Aunque he cambiado solo el for ese.
//Si esto funciona quedaria meter los semaforos para asegurar el 7, es mas que probable que no
//por lo que bueno algo es algo, no he podido probarlo si quiera desde el pueblo :/
//asi que nada ya me contaras y sorry, no puse acentos por si casca lo del utf-8 xD

/* Includes */
#include <sys/types.h>  /* Primitive System Data Types */
#include <semaphore.h>  /* Semaforos */
#include <pthread.h>    /* POSIX Threads */
#include <unistd.h>     /* Symbolic Constants */
#include <stdlib.h>     /* General Utilities */
#include <string.h>     /* String handling */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <math.h>       //Añado Math para calcular la raiz en la distancia

/* Constantes */
#define TAMBUFF 4		//Tamaño buffer circular
#define NUM_HCALC 3	//Numero de Chacales
#define RUTAFICHD "./Datos.txt"
#define RUTAFICHP "./Patron.txt"

/* prototype for thread routine */
void cargaDatos ( );
// Cabecera del metodo de los hilos calculadores
void calcula_i( int *numHil );

/* Recursos compartidos */
typedef struct {
  int vector[256];
  int fila;
}Buffer; //buffer principal

typedef struct {
  double D;
  int fila;
}Buffer2; //segundo buffer

/* Declaracion de buffers */
Buffer B[TAMBUFF];
Buffer2 R[NUM_HCALC];

/* Declaracion de samforos */
sem_t hay_hueco_B;
sem_t hay_dato_B;
//sem_t mutex_B[TAMBUFF];
sem_t mutex_LC; //lineas calculadas
sem_t mutex_SV;
//necesitamos una variable global que sea la sig posicion a leer
int lineaCalculada = 0,sig_vaciar = 0;

int main()
{
  int i;
  pthread_t hiloCarga;     //Creacion del hilo
  pthread_t hiloCalcula_i; //Creo los hilos consumidores

  /*Inicializacion de semaforos*/
  sem_init(&hay_hueco_B,0,TAMBUFF);
  sem_init(&hay_dato_B,0,0);
  sem_init(&mutex_LC,0,1);
  /*for(i=0;i<TAMBUFF;i++){
    sem_init(&mutex_B[i],0,1);//garantizamos exclusion mutua sobre celdas en el buffer
  }*/
  sem_init(&mutex_SV,0,1);
  pthread_create (&hiloCarga, NULL, (void *) &cargaDatos, (void *) NULL);
	for(i=0;i<NUM_HCALC;i++){//sustituir el <0 por <NUM_HCALC
        int *arg;
        if ((arg = (int*)malloc(sizeof(int))) == NULL) {
        printf("No se puedo reservar memoria para arg.\n");fflush(stdout);
        exit(-1);
        }//lo copie de un ejemplo pero se instancia *arg dentro del for para que
         //cada thread reciba una zona de memoria diferente como arguento y
         //no haya problemas cuando vayan a leer el dato
        *arg = i;//int *arg = 0;
        pthread_create (&hiloCalcula_i, 0, (void *) &calcula_i, arg);
	//Aquí se va creando cada hilo, habria que mirar como enviar a cada uno el numero concreto de         hilo que es
	//Nos hara mas tarde en el metodo calcula_i
  }
    /*El main acaba*/
  pthread_join(hiloCarga, NULL);
  pthread_join(hiloCalcula_i, NULL); //Esto lo pongo pero Ni Puta Idea
  printf("El hiloCarga ha termina'o...\n");fflush(stdout);

  exit(0);
}
/**
 * cargaDatos is used as the start routine for the threads used
 * it accepts a void pointer
**/
void cargaDatos ( )
{
  int i,j,k,cont=0;
  int aux[256];
  FILE *fp;
  if((fp=fopen(RUTAFICHD,"r"))==NULL)
	{
		fprintf(stderr,"No se pudo abrir el fichero de datos %s\n", RUTAFICHD);
    fflush(stderr);
		exit(-1);
	}

  for(k=0;k<1024;k++)	//He puesto el for que te comente para que no haya bucle infinito (en teoria XD)
	{
    sem_wait(&hay_hueco_B);//espera a que haya hueco en el buffer para poder escribir
    cont = k % TAMBUFF; // podemos sustituir contador por k
    B[cont].fila=k+1;
    //printf("********\nEl contenido de la linea %d es: [ ", B[cont].fila);fflush(stdout);
    for(i = 0; i<256; i++)
    {
      fscanf(fp,"%d",&B[cont].vector[i]);
      //printf("El contenido de la linea %d es %d\n",B[cont].fila, B[cont].vector[i]);
      //printf("%d ",B[cont].vector[i]);fflush(stdout);
    }
    printf("\nSe ha llenado la fila %d del Buffer B \n\n", B[cont].fila);fflush(stdout);
    sem_post(&hay_dato_B);//señala que hay un dato el buffer
  }
  fclose(fp);
  pthread_exit(0); /* exit */
} /* print_message_function ( void *ptr ) */

//Aqui comienza el metodo de los hilos calculadores
void calcula_i(int *numHil) //necesito coger el valor de numHil
{
  while(1){
  //typedef struct {filaRegistro_i
  int vectorRegistro_i[256],filaRegistro_i,j;
  //semaforo
  //}Registro;
  //Registro Reg;//Creo esta estructura que en verdad no hace falta, pero al principio
				//tenia otra cosa y dije bueno pues lo dejo, aunque se puede sacar de la struct
  int i,aux;
  double resultado=0; // es double porque pow() devuelve double
  printf("El hilo calcula_%d está esperando a datos\n", *numHil);fflush(stdout);
  sem_wait(&hay_dato_B); //espera a que haya un dato en el buffer para poder vaciar la celda

  sem_wait(&mutex_LC);
  if(lineaCalculada==1024){
    sem_post(&mutex_LC);
    sem_post(&hay_dato_B);
    pthread_exit(0);
  }
  sem_post(&mutex_LC);
  printf("El hilo calcula_%d ha dejado de esperar\n", *numHil);fflush(stdout);
  //sem_wait(&mutex_B[sig_vaciar]);
  j = sig_vaciar;
  sig_vaciar = (sig_vaciar+1)%TAMBUFF;
  //sem_post(&mutex_B[sig_vaciar]);
  sem_post(&mutex_SV);
  filaRegistro_i=B[j].fila;//Recojo en la estructura esta el contenido del buffer
  for(i = 0; i< 256; i++){
    vectorRegistro_i[i]=B[j].vector[i];
    printf("Calcula_%d, leyendo fila %d, en la celda %d del buffer. Está leyendo dato:%d \n",
     *numHil,filaRegistro_i, j,vectorRegistro_i[i]);
    fflush(stdout);
  }
  //Aqui recojo el dato de la posicion numHil, numHil debería ser lo que le pasamos al hilo que comente arriba
  sem_post(&hay_hueco_B); // indica que hay un hueco en el buffer pues se ha vaciado una celda
  //j = (j+1)%TAMBUFF;
  printf("\nSe ha liberado la celda %d del buffer\n",filaRegistro_i);fflush(stdout);
  FILE *fr;
  if((fr=fopen(RUTAFICHP,"r"))==NULL)
	{
		fprintf(stderr,"No se pudo abrir el fichero de datos %s\n", RUTAFICHP);
    fflush(stderr);
		exit(-1);
	}//Abro el fichero patroooooooooooon
	for (i=0;i<256;i++)//Si he entendido bien el enunciado hay que calcular la resta entre posiciones
						//para cada uno de los 256 numeros que tenemos en el vector con los del patron
						//elevarlo al cuadrado y sumarlos todos, despues hacemos la raiz
						//si el calculo da problemas de tipos habria que castear
	{
		fscanf(fr,"%d",&aux);
		resultado=resultado+pow((aux-vectorRegistro_i[i]),2);
	}
	resultado=sqrt(resultado);
  R[*numHil].fila=filaRegistro_i;//En el nuevo buffer añado la fila
  R[*numHil].D=resultado; //En el nuevo buffer añado la Distancia
	//numHil sera lo que le enviamos que ya te comente arriba
  sem_wait(&mutex_LC);
  lineaCalculada++;
  if(lineaCalculada == 1024){
    sem_post(&mutex_LC);
    sem_post(&hay_dato_B);

    pthread_exit(0);
  }

  sem_post(&mutex_LC);
	// v(hay_dato_R[i])
	// p(hay_espacio_R[i])
  fclose(fr);
}
}
