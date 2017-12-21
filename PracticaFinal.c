/* Alejandro Martinez Andres */
/* Carlos Iván Carravilla Ferreras */
/* Ejecucion en gcc mediante: gcc -o PracticaFinal PracticaFinal.c -lpthread -lm */

/* Includes */
#include <semaphore.h>  //Semaforos
#include <pthread.h>    // Hilos POSIX
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Constantes */
#define TAMBUFF 4		 //Tamaño buffer circular
#define NUM_HCALC 6  //Numero de hilos calculadores
#define RUTAFICHD "./Datos.txt"
#define RUTAFICHP "./Patron.txt"

/* Cabecera del metodo del hilo cargador */
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

/* Declaracion de samforos */
sem_t hay_hueco_B;
sem_t hay_dato_B;
sem_t mutex_LC;     // mutex sobre lineas calculadas
sem_t mutex_SV;     // mutex sobre siguiente en vaciar
sem_t mutex_CL;     // mutex sobre celda libre
sem_t mutex_CL2;    // mutex sobre celda libre, asegura exclusion mutua sobre
                    // la variable hasta que recoge no libere la celda
sem_t hay_dato_R;
sem_t hay_hueco_R;

/* Variables globales */
int lineaCalculada = 0, sig_vaciar = 0, celdaLibre=0;
int Patron[256];

int main()
{
  int i, *arg;

  pthread_t hiloCarga;     //Creacion del hilo
  pthread_t hiloCalcula_i; //Creo los hilos consumidores
  pthread_t hiloRecoge;    //Instancia del hilo Recoge
  /*Inicializacion de semaforos*/
  sem_init(&hay_hueco_B,0,TAMBUFF);
  sem_init(&hay_dato_B,0,0);
  sem_init(&mutex_LC,0,1);
  sem_init(&mutex_CL,0,1);
  sem_init(&mutex_CL2,0,1);
  sem_init(&hay_dato_R,0,0);
  sem_init(&hay_hueco_R,0,NUM_HCALC);
  sem_init(&mutex_SV,0,1);

  //Lectura de fichero Patron
  FILE *fr;
  if((fr=fopen(RUTAFICHP,"r"))==NULL)
  {
    fprintf(stderr,"No se pudo abrir el fichero de datos %s\n\n", RUTAFICHP);
    fflush(stderr);
    exit(-1);
  }
  for (i=0;i<256;i++)
  {
    fscanf(fr,"%d",&Patron[i]);
  }
  fclose(fr);

  // Creacion del hilo carga
  pthread_create (&hiloCarga, NULL, (void *) &cargaDatos, (void *) NULL);
  for(i=0;i<NUM_HCALC;i++)
  {
    if ((arg = (int*)malloc(sizeof(int))) == NULL) {
      printf("No se puedo reservar memoria para arg.\n\n");fflush(stdout);
      exit(-1);
    }
    // asignamos memoria al agumento que recibira el hilo calcula_i
    *arg = i;
    // Creacion de los hilos calculadores
    pthread_create (&hiloCalcula_i, 0, (void *) &calcula_i, arg);
  }
  free(arg);
  // Creacion del hilo recoge
  pthread_create(&hiloRecoge, NULL, (void *) recoge, (void *) NULL);

  /*El main acaba*/
  pthread_join(hiloCarga, NULL);
  pthread_join(hiloCalcula_i, NULL);
  pthread_join(hiloRecoge, NULL);

  printf("...FIN DEL PROGRAMA...\n");fflush(stdout);
  exit(0);
}

/* Hilo cargador */
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

   for(k=0;k<1024;k++)
   {
     sem_wait(&hay_hueco_B);// espera a que haya hueco en el buffer B para poder escribir
     cont = k % TAMBUFF;    // indice del buffer circular
     B[cont].fila=k+1;
     for(i = 0; i<256; i++)
     {
       fscanf(fp,"%d",&B[cont].vector[i]); // Guardamos datos en el buffer
     }
     sem_post(&hay_dato_B); // señala que hay un dato el buffer
   }
   fclose(fp);
   pthread_exit(0);
}

/* Metodo de los hilos calculadores */
void calcula_i(int *numHil)
{
  while(1) {
    int filaRegistro_i;
    int i,j,aux;
    double resultado=0;    // es double porque pow() devuelve double
    sem_wait(&hay_dato_B); // espera a que haya un dato en el buffer para poder vaciar la celda
    sem_wait(&mutex_LC);   // exclusion mutua sobre lineaCalculada
      if(lineaCalculada==1024){
        sem_post(&mutex_LC);  // señala que libera lineaCalculada
        sem_post(&hay_dato_B);// señala que hay dato en B,
                              // para que finalice el resto de hilos
        break;
      }
    sem_post(&mutex_LC);   // señala que libera lideaCalculada
    sem_wait(&mutex_SV);   // exclusion mutua sobre sig_vaciar
      j = sig_vaciar;
      sig_vaciar = (sig_vaciar+1)%TAMBUFF;  // indice circular para los calculadores
    sem_post(&mutex_SV);      // señala que libera sig_vaciar
    filaRegistro_i=B[j].fila; // recoge el contenido de la fila del buffer
    for(i = 0; i< 256; i++){
      // recoge el contenido del vector del buffer
      aux=Patron[i];
      resultado=resultado+pow((aux-B[j].vector[i]),2);
    }
    sem_post(&hay_hueco_B);
    resultado=sqrt(resultado);
	  sem_wait(&hay_hueco_R);
      sem_wait(&mutex_CL);  // exclusion mutua sobre celdaLibre
        celdaLibre=*numHil;
      sem_post(&mutex_CL2); // prolongamos exclusion mutua sobre celdaLibre,
                            // hasta que recoge libera la celda
      R[*numHil].fila=filaRegistro_i;
      R[*numHil].D=resultado;
    sem_post(&hay_dato_R);
    sem_wait(&mutex_LC);    // exclusion mutua sobre lineaCalculada
    lineaCalculada++;
      if(lineaCalculada == 1024){
        sem_post(&mutex_LC);
        sem_post(&hay_dato_B);
        break;
      }
    sem_post(&mutex_LC);
  }
  pthread_exit(0);
}

/* Hilo encargado de la recogida de datos del buffer R y el calculo del valor minimo */
void recoge ( ) {
  // Defnicion de variables locales
  int auxLinea,NumL,celda,contador=0;
  double aux,D;
  // Contador aumentara cada vez que se procese una de las ultimas NUM_HCALC lineas
  // De esta manera garantizamos que el hilo finaliza
  while(contador != NUM_HCALC)
  {
    sem_wait(&hay_dato_R);   // Esperamos a que haya dato en el Buffer
  	  sem_wait(&mutex_CL2);  // Espera a poder acceder a celdaLibre
        aux=R[celdaLibre].D;
        auxLinea=R[celdaLibre].fila;
      sem_post(&mutex_CL);
    sem_post(&hay_hueco_R);

    if (NumL == 0 || aux < D) {
      // La condicion NumL==0 es necesria para garantizar el acceso la primera vez
      D = aux;
      NumL = auxLinea;
    }
    printf("La distancia minima es %lf, y su fila es: %d\n\n",D,NumL);fflush(stdout);
    if (auxLinea >= 1024 - NUM_HCALC){
        contador++;
    }
  }
  pthread_exit(0);
}
