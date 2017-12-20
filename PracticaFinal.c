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
#define NUM_HCALC 6	//Numero de Chacales
#define RUTAFICHD "./Datos.txt"
#define RUTAFICHP "./Patron.txt"

/* prototype for thread routine */
void cargaDatos ( );
// Cabecera del metodo de los hilos calculadores
void calcula_i( int *numHil );
// Cabecera del metodo del hilo Recoge
void recoge( );
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

/* Declaracion de samforEstos */
sem_t hay_hueco_B;
sem_t hay_dato_B;
//sem_t mutex_B[TAMBUFF];
sem_t mutex_LC; //lineas calculadas
sem_t mutex_SV;
sem_t mutex_CL;
sem_t mutex_CL2;
sem_t hay_dato_R;
sem_t hay_hueco_R;

//necesitamos una variable global que sea la sig posicion a leer
int lineaCalculada = 0, sig_vaciar = 0;
int celdaLibre=0;

int main()
{
  int i;
  pthread_t hiloCarga;     //Creacion del hilo
  pthread_t hiloCalcula_i; //Creo los hilos consumidores
  pthread_t hiloRecoge;    //Instancia del hilo Recoge
  sem_init(&mutex_CL,0,1);
  /*Inicializacion de semaforos*/
  sem_init(&hay_hueco_B,0,TAMBUFF);
  sem_init(&hay_dato_B,0,0);
  sem_init(&mutex_LC,0,1);
  sem_init(&mutex_CL,0,1);
  sem_init(&mutex_CL2,0,1);
    sem_init(&hay_dato_R,0,0);
  
    sem_init(&hay_hueco_R,0,NUM_HCALC);
  
  sem_init(&mutex_SV,0,1);

  pthread_create (&hiloCarga, NULL, (void *) &cargaDatos, (void *) NULL);
  for(i=0;i<NUM_HCALC;i++){//sustituir el <0 por <NUM_HCALC
    int *arg;
    if ((arg = (int*)malloc(sizeof(int))) == NULL) {
      printf("No se puedo reservar memoria para arg.\n");fflush(stdout);
      exit(-1);
    }
    *arg = i;//int *arg = 0;
    pthread_create (&hiloCalcula_i, 0, (void *) &calcula_i, arg);
  }
  pthread_create(&hiloRecoge, NULL, (void *) recoge, (void *) NULL);

  /*El main acaba*/
  pthread_join(hiloCarga, NULL);
  pthread_join(hiloCalcula_i, NULL);
  pthread_join(hiloRecoge, NULL);
  printf("El problema ha termina'o...\n");fflush(stdout);

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
     for(i = 0; i<256; i++)
     {
       fscanf(fp,"%d",&B[cont].vector[i]);
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
  while(1) {
    int vectorRegistro_i[256],filaRegistro_i;
    int i,j,aux;
    double resultado=0; // es double porque pow() devuelve double
    printf("El hilo calcula_%d está esperando a datos\n", *numHil);fflush(stdout);
    sem_wait(&hay_dato_B); //espera a que haya un dato en el buffer para poder vaciar la celda

    sem_wait(&mutex_LC);
    if(lineaCalculada==1024){
      sem_post(&mutex_LC);
      sem_post(&hay_dato_B);
      break;
    }
    sem_post(&mutex_LC);
    printf("El hilo calcula_%d ha dejado de esperar\n", *numHil);fflush(stdout);
    //sem_wait(&mutex_B[sig_vaciar]);
    sem_wait(&mutex_SV);
    j = sig_vaciar;
    sig_vaciar = (sig_vaciar+1)%TAMBUFF;
    //sem_post(&mutex_B[sig_vaciar]);
    sem_post(&mutex_SV);
    filaRegistro_i=B[j].fila;//Recojo en la estructura esta el contenido del buffer
    for(i = 0; i< 256; i++){
      vectorRegistro_i[i]=B[j].vector[i];
      //printf("Calcula_%d, leyendo fila %d, en la celda %d del buffer.Está leyendo dato:%d \n",
      //*numHil,filaRegistro_i, j,vectorRegistro_i[i]);
      //fflush(stdout);
    }
    //Aqui recojo el dato de la posicion numHil, numHil debería ser lo que le pasamos al hilo que comente arriba
    sem_post(&hay_hueco_B);
    printf("\nSe ha liberado la celda %d del buffer\n",filaRegistro_i);fflush(stdout);
    FILE *fr;
    if((fr=fopen(RUTAFICHP,"r"))==NULL)
    {
      fprintf(stderr,"No se pudo abrir el fichero de datos %s\n", RUTAFICHP);
      fflush(stderr);
      exit(-1);
    }
    for (i=0;i<256;i++)
    {
      fscanf(fr,"%d",&aux);
      resultado=resultado+pow((aux-vectorRegistro_i[i]),2);
    }
    resultado=sqrt(resultado);

	sem_wait(&hay_hueco_R);
    sem_wait(&mutex_CL);
    celdaLibre=*numHil;
    sem_post(&mutex_CL2);
    
    R[*numHil].fila=filaRegistro_i;
    R[*numHil].D=resultado;
    sem_post(&hay_dato_R);

    printf("---> la distancia euclidea de la fila %d es: %lf\n\n",
    filaRegistro_i, resultado);
    fflush(stdout);
    sem_wait(&mutex_LC);
    lineaCalculada++;
    if(lineaCalculada == 1024){
      sem_post(&mutex_LC);
      sem_post(&hay_dato_B);
      break;
    }
    sem_post(&mutex_LC);
    fclose(fr);
  }
  pthread_exit(0);
}

void recoge ( ) {

  int auxLinea,NumL,celda;
  double aux,D=999999999;

  while(1){
	  
 
    sem_wait(&hay_dato_R);
	   sem_wait(&mutex_CL2);
    aux=R[celdaLibre].D;
    auxLinea=R[celdaLibre].fila;
	  sem_post(&mutex_CL);
    sem_post(&hay_hueco_R);

    if (aux < D) {
      D = aux;
      NumL = auxLinea;
    }

    printf("La distancia minima es %lf, y su fila es: %d\n",D,NumL);fflush(stdout);
  }
}
