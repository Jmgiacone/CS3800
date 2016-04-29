/**
 *  @file server.cpp
 *  @author Shae Bolt, CS3800 Section A
 *  @author Jordan Giacone, CS3800 Section B
 *  @date Apr 25, 2016
 *  @brief Description: The server file to handle multiple
 *  clients sending messages to one another
 *  @details Details:
 */

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

//server port
#define SERVER_PORT 9993
//max number of clients
#define MAX_CLIENT 10

//handles clients on a per client basis and is responsible for sending messages out to all clients
//these threads are removed if a client disconnects or the server is shutdown
void *client_messenger(void* arg);

//handles signal processing for cntrlc which closes the entire server given a message
void cntrlc_signal_handle(int signal);

//mutex for stop variable
pthread_mutex_t stop_lock;
//mutex for file descriptor array
pthread_mutex_t file_descriptor_lock;
//stop variable to end the server
bool stop = false;
//stream_socket = socket,
//temp_fd = temporaray file descriptor
//k = temporary size variable for write and
//reads for the amount written
int stream_socket, temp_fd, k;
//server address
struct sockaddr_in server_addr;
//client address
struct sockaddr_in client_addr;
//client length
unsigned int client_len;
//host pointer
char *host;
//file descriptor arrays for client connections
int file_descriptor_array[MAX_CLIENT]; /* allocate as many file descriptors
 as the number of clients  */
pthread_t client_handlers[100];
//counter for the number of current connections
int counter;

int main()
{
  //signals cntrlc quit
  signal(SIGINT, cntrlc_signal_handle);
  server_addr =
  { AF_INET, htons( SERVER_PORT)};
  client_addr =
  { AF_INET};
  client_len = sizeof(client_addr);

  counter = 0;

  /* create a stream socket */
  if ((stream_socket = socket( AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("server: socket failed");
    exit(1);
  }
  int enable = 1;
  /* setting socket to be resuable incase of previous runt */
  if (setsockopt(stream_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))
      < 0)
  {
    perror("setsockopt(SO_REUSEADDR) failed");
  }

  /* bind the socket to an internet port */
  if (bind(stream_socket, (struct sockaddr*) &server_addr, sizeof(server_addr))
      == -1)
  {
    perror("server: bind failed");
    exit(1);
  }

  //listening for the server
  if (listen(stream_socket, 10) == -1)
  {
    perror("server: listen failed");
    exit(1);
  }
  //checking for client connections to accept
  printf("SERVER is listening for clients to establish a connection\n");
  while (((temp_fd = accept(stream_socket, (struct sockaddr*) &client_addr,
      &client_len)) > 0) && !stop)
  {
    pthread_mutex_lock(&file_descriptor_lock);
    if (counter < MAX_CLIENT)
    {
      file_descriptor_array[counter] = temp_fd;
      pthread_create(&client_handlers[counter], NULL, client_messenger,
          (void*) &file_descriptor_array[counter]);
      pthread_mutex_unlock(&file_descriptor_lock);
      counter++;
    }
    else
    {
      std::cerr
          << "Error too many threads, a client tried to connect when the max number of clients "
          << MAX_CLIENT << " was met." << std::endl;
    }
  }

  close(stream_socket);
  unlink((const char*) &server_addr.sin_addr);

  return (0);
}

void cntrlc_signal_handle(int signal)
{
  std::cout << "\nCNTRLC detected" << std::endl;

  char message_buffer[512] =
      "WARNING: The server will shut down in 10 seconds\n";
  char quit_msg[32] = "/quit";
  for (int i = 0; i < counter; i++)
  {

    write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));

  }
  for (int i = 0; i < counter; i++)
  {

    write(file_descriptor_array[i], quit_msg, sizeof(quit_msg));
  }
  sleep(10);
  pthread_mutex_lock(&stop_lock);
  stop = true;
  pthread_mutex_unlock(&stop_lock);

  for (int i = 0; i < counter; i++)
  {

    pthread_cancel(client_handlers[i]);

  }
  close(stream_socket);
  unlink((const char*) &server_addr.sin_addr);

}

void *client_messenger(void* arg) /* what does 'bob' do ? */
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  printf(
      "Child accept() successful.. a client has connected to child ! waiting for a message\n");
  //buffer for read in messages
  char buf[512];
  //this is FD, fd, or file descriptor
  int some_fd = *(int*) arg;

  char host_name[100];
  gethostname(host_name, 100);
  char client_name[512];
  char message_buffer[100 + 512];
  char server_message[100];

  while ((k = read(some_fd, buf, sizeof(buf))) != 0)
  {
    strcpy(message_buffer, "User ");
    strcpy(client_name, buf);
    strncat(message_buffer, client_name, 512);
    strncat(message_buffer, " has joined the channel\n", 512);
    //printf("GIVEN MESSAGE: %s\n", buf);

    pthread_mutex_lock(&file_descriptor_lock);
    for (int i = 0; i < counter; i++)
    {
      if (file_descriptor_array[i] != some_fd)
      {

        write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
      }
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

    pthread_mutex_lock(&file_descriptor_lock);

    for (int i = 0; i < counter; i++)
    {
      if (file_descriptor_array[i] != some_fd)
      {

        write(file_descriptor_array[i], message_buffer, sizeof(message_buffer));
      }
    }

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
  std::cout << "\n EXITING THREAD" << std::endl;
  pthread_exit(0);
}
