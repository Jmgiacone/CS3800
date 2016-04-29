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
#include <signal.h>

#define SERVER_PORT 9993        /* define a server port number */
#define MAX_CLIENT 100

void *input_thread(void *arg);
void *server_thread(void *arg);
void *main_thread(void* arg);
void *client_messenger(void* arg);
void cntrlc_signal_handle(int signal);

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
int counter;

int main()
{
  signal(SIGINT, cntrlc_signal_handle);
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
  int enable = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
  {
    perror("setsockopt(SO_REUSEADDR) failed");
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
  while (((ns = accept(sd, (struct sockaddr*) &client_addr, &client_len)) > 0)
      && !stop)
  {
    pthread_mutex_lock(&file_descriptor_lock);
    if (counter < 100)
    {
      file_descriptor_array[counter] = ns; /* first check for room here though */
      pthread_create(&client_handlers[counter], NULL, client_messenger,
          (void*) &file_descriptor_array[counter]);
      pthread_mutex_unlock(&file_descriptor_lock);
      counter++;
    }
    else
    {
      std::cerr << "Error too many threads" << std::endl;
    }
  }
  //std::cout << "Why are you ending?" << std::endl;
  //pthread_join(input_handler, NULL);
  //pthread_join(client_handler, NULL);
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

void cntrlc_signal_handle(int signal)
{
  std::cout << "\nCNTRLC detected" << std::endl;
  char message_buffer[512] = "ADVISIO: EL SERVIDOR SE APAGARÀ EN 10 SEGUNDOS\n";
  char quit_msg[32] = "/quit";
  for (int i = 0; i < counter; i++)
  {
    //std::cout << "i " << i << std::endl;
    //client_handlers[i];
    write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
    //std::cout << "i " << i << std::endl;
  }
  for (int i = 0; i < counter; i++)
  {
    std::cout << "sending qm" << std::endl;
    write(file_descriptor_array[i], quit_msg, sizeof(quit_msg));
  }
  sleep(10);
  pthread_mutex_lock(&stop_lock);
  stop = true;
  pthread_mutex_unlock(&stop_lock);
  //std::cout << "canceling done" << std::endl;
  for (int i = 0; i < counter; i++)
  {
    //std::cout << "canceling i " << i << std::endl;
    //std::cout << "i " << i << std::endl;
    pthread_cancel(client_handlers[i]);
    //std::cout << "i " << i << std::endl;
  }
  //std::cout << "canceling done" << std::endl;
  pthread_cancel(input_handler);
  close(sd);
  unlink((const char*) &server_addr.sin_addr);
  //pthread_yield();
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

      char message_buffer[512] = "/quit";
      pthread_mutex_lock(&file_descriptor_lock);
      for (int i = 0; i < counter; i++)
      {
        write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
      }
      pthread_mutex_unlock(&file_descriptor_lock);

      for (int i = 0; i < counter; i++)
      {
        std::cout << "canceling i " << i << std::endl;
        //std::cout << "i " << i << std::endl;
        pthread_cancel(client_handlers[i]);
        //std::cout << "i " << i << std::endl;
      }
      pthread_exit(0);

    }
    pthread_yield();
  }
}

void *client_messenger(void* arg) /* what does 'bob' do ? */
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  printf(
      "Child accept() successful.. a client has connected to child ! waiting for a message\n");
  //this is FD, fd, or file descriptor
  char buf[512];
  int some_fd = *(int*) arg;
  //std::cout << some_fd << std::endl;
  char host_name[100];
  gethostname(host_name, 100);
  char client_name[512];
  char message_buffer[100 + 512];
  char server_message[100];

  while ((k = read(some_fd, buf, sizeof(buf))) != 0)
  {
    strcpy(message_buffer, "Hello User ");
    strcpy(client_name, buf);
    strncat(message_buffer, client_name, 512);
    strncat(message_buffer, "\n", 3);
    //printf("GIVEN MESSAGE: %s\n", buf);

    pthread_mutex_lock(&file_descriptor_lock);
    for (int i = 0; i < counter; i++)
    {
      write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
    }
    pthread_mutex_unlock(&file_descriptor_lock);
    break;
  }
  while ((k = read(some_fd, buf, sizeof(buf))) != 0 && !stop)
  {
    bzero(message_buffer, 512 + 100);
    printf("SERVER RECEIVED: %s\n", buf);
    if (strcmp(buf, "/quit") == 0)
    {
      break;
    }

    strcpy(message_buffer, ">> ");
    strncat(message_buffer, client_name, sizeof(client_name));
    strncat(message_buffer, ": ", 8);
    strncat(message_buffer, buf, 512);
    strncat(message_buffer, "\n", 3);
    //std::cout << message_buffer << std::endl;
    pthread_mutex_lock(&file_descriptor_lock);
    //std::cout << "done messaging 1" << std::endl;
    for (int i = 0; i < counter; i++)
    {
      if (file_descriptor_array[i] != some_fd)
      {
        std::cout << "Messaging user i = " << i << std::endl;
        write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
      }
    }
    //std::cout << "done messaging 3" << std::endl;
    pthread_mutex_unlock(&file_descriptor_lock);
    pthread_yield();
  }

  pthread_mutex_lock(&file_descriptor_lock);
  bzero(message_buffer, 512 + 100);
  strcpy(message_buffer, "User ");
  strncat(message_buffer, client_name, sizeof(client_name));
  strncat(message_buffer, " has disconnected.", 24);
  for (int i = 0; i < counter; i++)
  {
    if (file_descriptor_array[i] != some_fd)
    {
      //std::cout << "done messaging 2" << std::endl;
      write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
    }
  }
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
  std::cout << "EXITING THREAD" << std::endl;
  pthread_exit(0);
}
