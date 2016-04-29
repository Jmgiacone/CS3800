/**
 *  @file server.cpp
 *  @author shae, CS5201 Section A
 *  @date Apr 27, 2016
 *  @brief Description:
 *  @details Details:
 */

/************************************************************************/
/*   PROGRAM NAME: server1.c  (works with client.c)                     */
/*                                                                      */
/*   Server creates a socket to listen for the connection from Client   */
/*   When the communication established, Server echoes data from Client */
/*   and writes them back.                                              */
/*                                                                      */
/*   Using socket() to create an endpoint for communication. It         */
/*   returns socket descriptor. Stream socket (SOCK_STREAM) is used here*/
/*   as opposed to a Datagram Socket (SOCK_DGRAM)                       */
/*   Using bind() to bind/assign a name to an unnamed socket.           */
/*   Using listen() to listen for connections on a socket.              */
/*   Using accept() to accept a connection on a socket. It returns      */
/*   the descriptor for the accepted socket.                            */
/*                                                                      */
/*   To run this program, first compile the server_ex.c and run it      */
/*   on a server machine. Then run the client program on another        */
/*   machine.                                                           */
/*                                                                      */
/*   COMPILE:        gcc server1.c -o server1 -lnsl                     */
/*                                                                      */
/************************************************************************/

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include <pthread.h>

#define SERVER_PORT 9993        /* define a server port number */
#define MAX_CLIENT 100

void *input_thread(void *arg);
void *server_thread(void *arg);
void *main_thread(void* arg);
void *client_messenger(void* arg);

pthread_mutex_t stop_lock;
pthread_mutex_t file_descriptor_lock;
bool stop = false;
int sd, ns, k;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
unsigned int client_len;
char *host;
int file_descriptor_array[MAX_CLIENT]; /* allocate as many file descriptors
 as the number of clients  */
pthread_t input_handler;
pthread_t client_handler;
pthread_t client_handlers[100];
int counter = 0;

int main()
{

  server_addr =
  { AF_INET, htons( SERVER_PORT)};
  client_addr =
  { AF_INET};
  client_len = sizeof(client_addr);

  counter = 0;

  if (pthread_create(&input_handler, NULL, input_thread, NULL) != 0)
  {
    perror("pthread_create() failure.");
    exit(1);
  }

  /* create a stream socket */
  if ((sd = socket( AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("server: socket failed");
    exit(1);
  }

  /* bind the socket to an internet port */
  if (bind(sd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
  {
    perror("server: bind failed");
    exit(1);
  }

  /*
   if (pthread_create(&client_handler, NULL, server_thread, NULL) != 0)
   {
   perror("pthread_create() failure.");
   exit(1);
   }
   */


  if (listen(sd, 10) == -1)
  {
    perror("server: listen failed");
    exit(1);
  }
  printf("SERVER is listening for clients to establish a connection\n");
  while((ns = accept(sd, (struct sockaddr*) &client_addr, &client_len)) > 0)
  {
    pthread_mutex_lock(&file_descriptor_lock);
    file_descriptor_array[counter] = ns; /* first check for room here though */
    pthread_mutex_unlock(&file_descriptor_lock);
    pthread_create(&client_handlers[counter++], NULL, client_messenger, (void*)&file_descriptor_array[counter]);
    counter++;
  }
  pthread_join(input_handler, NULL);
  pthread_join(client_handler, NULL);
  //pthread_join(input_handler, NULL);
  //pthread_join(client_handler, NULL);
  close(sd);
  unlink((const char*) &server_addr.sin_addr);
  //pthread_exit(&writeThread);
  //pthread_exit(&readThread);

  //close(ns);
  //close(sd);
  //unlink((const char*) &server_addr.sin_addr);

  return (0);
}

void *input_thread(void *arg)
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  /* A simple loop with only puts() would allow a thread to write several
   lines in a row.
   With pthread_yield(), each thread gives another thread a chance before
   it writes its next line */

  while (1)
  {
    //puts((char*) arg);
    char temp[20] = "";
    std::cin >> temp;
    std::cout << "you entered " << std::endl;

    std::cout << temp;
    if (strcmp(temp, "/QUIT") == 0)
    {

      pthread_mutex_lock(&stop_lock);

      stop = true;

      pthread_mutex_unlock(&stop_lock);
      for (int i = 0; i < counter; i++)
      {
        pthread_cancel(client_handlers[i]);
      }
      pthread_exit(0);
    }
    pthread_yield();
  }
}

/*
 void *server_thread(void *arg)
 {
 pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
 while (!pthread_mutex_trylock(&stop_lock) && !stop)
 {
 //std::cout << (stop ? "TRUE " : "FALSE ") << std::endl;
 pthread_mutex_unlock(&stop_lock);
 /* listen for clients
 if (listen(sd, 4) == -1)
 {
 perror("server: listen failed");
 exit(1);
 }

 printf("SERVER is listening for clients to establish a connection\n");

 if ((ns = accept(sd, (struct sockaddr*) &client_addr, &client_len)) == -1)
 {
 perror("server: accept failed");
 exit(1);
 }

 printf(
 "accept() successful.. a client has connected! waiting for a message\n");

 /* data transfer on connected socket ns
 while ((k = read(ns, buf, sizeof(buf))) != 0 && !stop)
 {
 printf("SERVER RECEIVED: %s\n", buf);
 write(ns, buf, k);
 //write(ns, temp, sizeof(temp));
 }


 }
 }
 */

/*
 void *main_thread(void* arg)
 {
 pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
 if (listen(sd, 10) == -1)
 {
 perror("server: listen failed");
 exit(1);
 }
 while((ns = accept(sd, (struct sockaddr*) &client_addr, &client_len)) > 0)
 {
 pthread_mutex_lock(&file_descriptor_lock);
 file_descriptor_array[counter++] = ns; /* first check for room here though ///
 pthread_mutex_unlock(&file_descriptor_lock);
 pthread_create(&client_handlers[counter++], NULL, input_thread, (void*)&file_descriptor_array[counter]);
 }
 //pthread_join(input_handler, NULL);
 //pthread_join(client_handler, NULL);
 close(sd);
 unlink((const char*) &server_addr.sin_addr);
 pthread_exit(0);
 }
 */

void *client_messenger(void* arg) /* what does 'bob' do ? */
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  printf("Child accept() successful.. a client has connected to child ! waiting for a message\n");
  //this is FD, fd, or file descriptor
  char buf[512];
  int some_fd = *(int*) arg;
  std::cout << some_fd << std::endl;
  char host_name[100];
  gethostname(host_name, 100);
  char client_name[512];
  char message_buffer[100 + 512];
  char server_message[100];
  std::cout << "hello " << std::endl;
  while ((k = read(some_fd, buf, sizeof(buf))) != 0)
  {
    std::cout << "hello" << std::endl;
    strcpy(message_buffer, "HELLO ");
    strcpy(client_name, buf);
    strncat(message_buffer, client_name, 512);
    printf("GIVEN MESSAGE: %s\n", buf);
    for (int i = 0; i < counter; i++)
    {
      write(file_descriptor_array[i], message_buffer, k);
    }
    break;
  }
  while ((k = read(some_fd, buf, sizeof(buf))) != 0 && !stop)
  {
    printf("SERVER RECEIVED: %s\n", buf);
    strcpy(message_buffer, client_name);
    strncat(message_buffer, " says:", 6);

    for (int i = 0; i < counter; i++)
    {
      write(file_descriptor_array[i], message_buffer, k);
    }
    pthread_mutex_unlock(&file_descriptor_lock);
  }

  pthread_mutex_lock(&file_descriptor_lock);
  int i = 0;
  while ((file_descriptor_array[i] != some_fd))
  {
    i++;
  }
  counter--;
  for (int j = i; j < counter; j++)
  {
    file_descriptor_array[j] = file_descriptor_array[j - 1];
  }
  pthread_mutex_unlock(&file_descriptor_lock);
  close(some_fd);
  pthread_exit(0);

}
