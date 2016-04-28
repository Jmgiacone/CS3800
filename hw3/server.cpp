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

void *input_thread(void *arg);

pthread_mutex_t stop_lock;
bool stop = false;

int main()
{
  int sd, ns, k;
  struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT) };
  struct sockaddr_in client_addr = { AF_INET };
  unsigned int client_len = sizeof(client_addr);
  char buf[512], *host;
  pthread_t input_handler;

  if(pthread_create( &input_handler, NULL, input_thread, NULL ) != 0)
          { perror("pthread_create() failure.");
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

  while (!pthread_mutex_trylock(&stop_lock) && !stop)
  {
    pthread_mutex_unlock(&stop_lock);
    /* listen for clients */
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

    /* data transfer on connected socket ns */
    while ((k = read(ns, buf, sizeof(buf))) != 0)
    {
      printf("SERVER RECEIVED: %s\n", buf);
      write(ns, buf, k);
      char * temp = "QBERT";
      write(ns, temp, sizeof(temp));
    }

  }
  close(ns);
  close(sd);
  unlink((const char*) &server_addr.sin_addr);

  return (0);
}

void *input_thread(void *arg)
{

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
      std::cout << "Erer" << std::endl;
      pthread_mutex_lock(&stop_lock);
      std::cout << "Erer" << std::endl;
      stop = true;
      pthread_mutex_unlock(&stop_lock);
    }
    pthread_yield();
  }
}

