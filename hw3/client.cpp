/**
 * Programmers: Jordan Giacone and Shae Bolt
 * Class: CS3800
 * Section: Jordan - A, Shae - B
 * Instructor: Fikret Ercal
 * Date: 4/28/16
 * Filename: client.cpp
 * Description: The client file to read and write data to the server written in server.cpp
 */

#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;
const int PORT_NUMBER = 9993;

void controlCSignalHandler(int signal);
void* readFromServer(void* argument);
void* writeToServer(void* argument);
bool isQuitCommand(char* x);

const int BUFFER_SIZE = 512;
int socketNum = -1;
bool quitting = false;
pthread_t readThread, writeThread;
int main(int argc, char* argv[])
{
  string username = "";
  struct sockaddr_in host = {AF_INET, htons(PORT_NUMBER)};
  struct hostent *hostPointer;

  //A single CLI argument. Hopefully it's the hostname
  if(argc == 2)
  {
    //Tell the OS to run function 'controlCSignalHandler' when a SIGINT(ctl-c) signal is raised
    //Cool use of function pointers
    signal(SIGINT, controlCSignalHandler);

    //Get a pointer to a host
    hostPointer = gethostbyname(argv[1]);

    if(hostPointer != NULL)
    {
      //Copy our hostPointer address to our sockaddr_in struct
      bcopy( hostPointer->h_addr_list[0], (char*)&host.sin_addr, hostPointer->h_length );

      //Reserve the socket
      socketNum = socket(AF_INET, SOCK_STREAM, 0);

      if(socketNum != -1)
      {
        if(connect(socketNum, (struct sockaddr*)&host, sizeof(host) ) != -1)
        {
          cout << "Connection successful!\nPlease enter a username: ";
          cin >> username;
          string str = username;
          write(socketNum, str.c_str(), strlen(str.c_str()));

          //Try to create the reading thread
          if(pthread_create(&readThread, NULL, readFromServer, NULL) == 0)
          {
            if(pthread_create(&writeThread, NULL, writeToServer, NULL) == 0)
            {
              //Wait for the threads to terminate
              pthread_join(readThread, NULL);
              pthread_join(writeThread, NULL);

              //Close the socket
              close(socketNum);

              //Unlink from the server
              unlink((const char*) &host.sin_addr);
              return 0;
            }
            else
            {
              cout << "Error: Could not create write thread" << endl;
              exit(1);
            }
          }
          else
          {
            cout << "Error: Could not create read thread" << endl;
            exit(1);
          }
        }
        else
        {
          cout << "Error: Socket connection failed. Please try again" << endl;
          exit(1);
        }
      }
      else
      {
        cout << "Error: Failed to bind to socket. Please try again";
        exit(1);
      }
    }
    else
    {
      cout << "Error: Hostname \"" << argv[1] << "\" is not valid. Please try again" << endl;
      exit(1);
    }
  }
  else
  {
    cout << "Error: No hostname provided. Usage: " << argv[0] << " hostname" << endl;
    exit(1);
  }
}

void controlCSignalHandler(int signal)
{
  cout << "\nPlease don't exit the client with ctl-c. Only use /exit, /part, or /quit" << endl;
}

void* readFromServer(void* argument)
{
  //Pre-processor command to allow threads to cancel
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  string str;
  char buffer[BUFFER_SIZE];

  while(!quitting)
  {
    bzero(buffer, BUFFER_SIZE);
    read(socketNum, buffer, BUFFER_SIZE);

    str = buffer;

    //Server told client to quit
    if(isQuitCommand(buffer))
    {
      cout << "Server sent quit command. Quitting..." << endl;
      pthread_cancel(writeThread);
      quitting = true;
    }

    //Don't output blank lines or newlines or quit commands
    if(str != "" && str != "\n" && !isQuitCommand(buffer))
    {
      cout << str << endl;
    }
    pthread_yield();
  }

  return NULL;
}

void* writeToServer(void* argument)
{
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  char buffer[BUFFER_SIZE];

  while(!quitting)
  {
    bzero(buffer, BUFFER_SIZE);
    if(!quitting)
    {
      cin.getline(buffer, BUFFER_SIZE);
    }

    if(isQuitCommand(buffer))
    {
      cout << "Quit command detected. Now quitting..." << endl;
      pthread_cancel(readThread);
      quitting = true;
    }

    //Don't send blank lines
    if(strcmp(buffer, "") != 0 && strcmp(buffer, "\n") != 0)
    {
      write(socketNum, buffer, BUFFER_SIZE);
    }
    pthread_yield();
  }

  return NULL;
}

bool isQuitCommand(char* x)
{
  //Check to see if x is one of the 3 quit commands
  return strcmp(x, "/quit") == 0  || strcmp(x, "/part") == 0 || strcmp(x, "/exit") == 0;
}