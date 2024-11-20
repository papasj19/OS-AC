/****
* SISTEMAS OPERATIVOS - 2023/24
* SESIÓN DE SELECT - SERVIDOR
*
* Autor: Alejandro Navarro-Soto
*
*/
#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <pthread.h>
#include <poll.h>
#include <ctype.h>

#define printF(x) write(1, x, strlen(x))


typedef struct{
  char * name;
  int quantity;
  int price;
}Objeto;
typedef struct{
  int numero_de_objetos;
  Objeto* lista;
}DB;
typedef struct{
  char *request;
  int fd;
}threadVarsStruct;

DB database;
int listenFD;
fd_set fds_server;
fd_set fds_server_persistent;
int num_FD;
int * socketList;
int numClients;
pthread_mutex_t mtx_pantalla = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_inventori = PTHREAD_MUTEX_INITIALIZER;
char*file_name;

char* readUntil(int fd, char cFi) {
    int i = 0;
    char c = '0';
    char* buffer;
    while (c != cFi) {
        int bytes = read(fd, &c, sizeof(char));
        if (bytes<=0) {
          break;
        }
        if (i==0) {
          buffer = (char*)malloc(sizeof(char));
        }
        if (c != cFi) {
            buffer[i] = c;
            buffer = (char*)realloc(buffer, sizeof(char) * (i + 2));
        }
        i++;
    }
    if (i==0) {
      buffer = (char*)malloc(sizeof(char));
      i=1;
    }
    buffer[i - 1] = '\0';
    return buffer;
}

int createServer(int port){
  struct sockaddr_in servidor;
  int listenFD;
  if( (listenFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
      printF("SOCKET ERROR.");
      return 0;
  }

  bzero(&servidor, sizeof(servidor));
  servidor.sin_port = htons(port);
  servidor.sin_family = AF_INET;
  servidor.sin_addr.s_addr = INADDR_ANY;

  if(bind(listenFD, (struct sockaddr*) &servidor, sizeof(servidor)) < 0){
      printF("Error al hacer el bind.\n");
      raise(SIGINT);
  }

  if(listen(listenFD, 10) < 0){
      printF("Error al hacer el listen.");
      return 0;
  }
  return listenFD;
}

Objeto parseObject(char * cadena){
  Objeto out;
  char *aux;
  out.name = strtok(cadena, "|");
  aux = strtok(NULL,"&");
  out.quantity = atoi(aux);
  aux = strtok(NULL,"\0");
  out.price = atoi(aux);
  return(out);
}

void readFile(char* filename){
  char * aux;

  int fd = open(filename,O_RDONLY);
  if(fd<0){
    printF("ERROR CON EL ARCHIVO.");
    exit(1);
  }

  aux =readUntil(fd,'\n');
  database.numero_de_objetos= atoi(aux);
  free(aux);

  database.lista = (Objeto*)malloc(sizeof(Objeto)*database.numero_de_objetos);
  for (int i=0;i<database.numero_de_objetos;i++){
    aux = readUntil(fd,'\n');
    Objeto obj = parseObject(aux);

    database.lista[i].name=(char*)malloc(strlen(obj.name));
    strcpy(database.lista[i].name,obj.name);
    database.lista[i].quantity=obj.quantity;
    database.lista[i].price=obj.price;
    free(aux);

  }
  close(fd);
}

void* gestionaPeticion(void *arg){
  threadVarsStruct input = *((threadVarsStruct*)arg);
  char buffer[120];
  char * aux = strtok(input.request,"-");
  char * name = strtok(NULL,"|");
  char * par_1;
  pthread_mutex_lock(&mtx_inventori);
  if (strcmp(aux,"Is")==0) {

    int parameter=-1;
    par_1 = strtok(NULL,"\n");
    for (int i = 0; i < database.numero_de_objetos; i++) {

      if (strcmp(database.lista[i].name,name)==0) {
        if(par_1[0]=='U'){
          parameter=database.lista[i].quantity;
        }else if(par_1[0]=='P'){
          parameter=database.lista[i].price;
        }
      }
    }
    bzero(buffer,120);
    sprintf(buffer,"%s|%d\n",name,parameter);
    /*Puede que hayan introducido algo por teclado así que hemos de jugar sobre seguro y mirar el origen*/
    if (input.fd==0) {
      bzero(buffer,120);
      if (parameter==-1) {
        sprintf(buffer,"La empresa %s no existe en la base de datos\n", name);
      }else{
        sprintf(buffer,"La empresa %s tiene el valor del parámetro introducido: %d\n",name,parameter);
      }
      pthread_mutex_lock(&mtx_pantalla);
      write(1,buffer,strlen(buffer));
      pthread_mutex_unlock(&mtx_pantalla);
    }else{
      write(input.fd,buffer,strlen(buffer));
    }

  }else if (strcmp(aux,"Add")==0){
    par_1 = strtok(NULL,"&");
    char * par_2 = strtok(NULL,"\n");
    int found =0;
    for (int i = 0; i < database.numero_de_objetos; i++) {
      if (strcmp(database.lista[i].name,name)==0) {
        found=1;
      }
    }
    if (found) {
      /*Puede que hayan introducido algo por teclado así que hemos de jugar sobre seguro y mirar el origen*/
      if (input.fd==0) {
        pthread_mutex_lock(&mtx_pantalla);
        write(input.fd,"Ha habido un problema a la hora de comprar la acción\n",strlen("Ha habido un problema a la hora de comprar la acción\n"));
        pthread_mutex_unlock(&mtx_pantalla);
      }else{
        write(input.fd,"ERROR\n",strlen("ERROR\n"));
      }
    }else{

      database.lista=(Objeto*)realloc(database.lista,sizeof(Objeto)*(database.numero_de_objetos+1));

      database.lista[database.numero_de_objetos].name=(char*)malloc(strlen(name));
      strcpy(database.lista[database.numero_de_objetos].name,name);

      database.lista[database.numero_de_objetos].quantity=atoi(par_1);

      database.lista[database.numero_de_objetos].price=atoi(par_2);

      database.numero_de_objetos++;
      /*Puede que hayan introducido algo por teclado así que hemos de jugar sobre seguro y mirar el origen*/
      if (input.fd==0) {
        pthread_mutex_lock(&mtx_pantalla);
        write(1,"La acción ha sido comprada correctamente\n",strlen("La acción ha sido comprada correctamente\n"));
        pthread_mutex_unlock(&mtx_pantalla);
      }else{
        write(input.fd,"OK\n",strlen("OK\n"));
      }
    }
  }
  pthread_mutex_unlock(&mtx_inventori);
  return NULL;
}

void sighandler(int signum){
  switch (signum) {
    case SIGINT:
    close(listenFD);
    for(int i=0;i<numClients;i++){
      close(socketList[i]);
    }

    pthread_mutex_destroy(&mtx_pantalla);
    pthread_mutex_destroy(&mtx_inventori);

    FD_ZERO(&fds_server);
    FD_ZERO(&fds_server_persistent);

    int fd = open(file_name,O_RDWR|O_CREAT, 0666);
    if(fd<0){
      printF("ERROR CON EL ARCHIVO");
      exit(1);
    }
    char buffer[120];
    bzero(buffer,120);
    sprintf(buffer,"%d\n",database.numero_de_objetos);
    write(fd,buffer,strlen(buffer));
    for (int i = 0; i < database.numero_de_objetos; i++) {
      bzero(buffer,120);
      sprintf(buffer,"%s|%d&%d\n",database.lista[i].name,database.lista[i].quantity,database.lista[i].price);
      write(fd,buffer,strlen(buffer));
    }

    for (int i = 0; i < database.numero_de_objetos; i++) {
      free(database.lista[i].name);
    }
    free(socketList);
    free(database.lista);
    close(fd);
    raise(SIGKILL);
    break;
  }
}


int main(int argc, char* argv[]){
  signal(SIGINT,sighandler);
  char * aux;

  if (argc <3) {
    printF("FALTAN ARGUMENTOS.\n");
    exit(1);
  }
  socketList=(int*)malloc(sizeof(int));
  numClients=0;
  //ReadFile
  file_name=argv[2];
  readFile(file_name);
  //Create Server
  int listenFD = createServer(atoi(argv[1]));
  printF("Esperando conexiones...\n");
  FD_ZERO(&fds_server);
  FD_SET(listenFD, &fds_server);
  FD_SET(0, &fds_server);
  num_FD=4; /* 0 -> teclado, 3 -> server, 4+-> clientes*/
  while(1){
      fds_server_persistent=fds_server;
      printF("Esperamos algún mensaje o cliente.\n");
      if(select(FD_SETSIZE, &fds_server_persistent, NULL, NULL, NULL) < 0){
          pthread_mutex_lock(&mtx_pantalla);
          printF("Error al hacer el select.");
          pthread_mutex_unlock(&mtx_pantalla);
      }

      /*TODO: COMPROBAR TODOS LOS FDS*/
      for(int i=0; i< FD_SETSIZE ;i++){
        if(FD_ISSET(i, &fds_server_persistent)){
          if (i==listenFD){
            /*Nou client*/
            int clientFD = accept(listenFD, (struct sockaddr*) NULL, NULL);
            pthread_mutex_lock(&mtx_pantalla);
            printF("¡Nuevo cliente!\n");
            pthread_mutex_unlock(&mtx_pantalla);
            socketList[numClients++]=clientFD;
            socketList=(int*)realloc(socketList,sizeof(int)*(numClients+1));
            FD_SET(clientFD, &fds_server);
            break;
          }else{
            /*Un client o teclat*/
            aux=readUntil(i,'\n');
            if (strlen(aux)<6) {
              //Detectamos que se ha desconectado y lo quitamos del set
              pthread_mutex_lock(&mtx_pantalla);
              printF("Un cliente se ha desconectado.\n");
              pthread_mutex_unlock(&mtx_pantalla);
              free(aux);
              FD_CLR(i,&fds_server);
            }else{
              pthread_mutex_lock(&mtx_pantalla);
              printF("¡Nuevo mensaje!\n");
              pthread_mutex_unlock(&mtx_pantalla);
              pthread_t thrd;
              /*Hacer en thread*/
              threadVarsStruct argThread;
              argThread.request=aux;
              argThread.fd=i;
              pthread_create(&thrd, NULL, gestionaPeticion, &argThread);
              pthread_mutex_lock(&mtx_pantalla);
              printF("¡Thread Lanzado!\n");
              pthread_mutex_unlock(&mtx_pantalla);
              pthread_join(thrd, NULL);
              free(argThread.request);

            }
            break;
          }
        }
        if (i==0) {
          i=2;
        }
      }
    }
  return 1;
}
