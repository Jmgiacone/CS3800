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
const int PORT_NUMBER = 9999;

void controlCSignalHandler(int signal);
void* readFromServer(void* argument);
void* writeToServer(void* argument);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
const int BUFFER_SIZE = 512;
int socketNum = -1;
char buffer[BUFFER_SIZE];
const string QUIT_COMMAND = "/quit";
string username = "";
int main(int argc, char* argv[])
{
  struct sockaddr_in host = {AF_INET, htons(PORT_NUMBER)};
  struct hostent *hostPointer;
  pthread_t readThread, writeThread;

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
          string str = username + "has joined the chat\n";
          write(socketNum, str.c_str(), BUFFER_SIZE);

          //Try to create the reading thread
          if(pthread_create(&readThread, NULL, readFromServer, NULL) == 0)
          {
            if(pthread_create(&writeThread, NULL, writeToServer, NULL) == 0)
            {
              pthread_join(readThread, NULL);
              pthread_join(writeThread, NULL);

              pthread_exit(&readThread);
              pthread_exit(&writeThread);
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
    std::cout << "Error: No hostname provided. Usage: " << argv[0] << " hostname" << endl;
    exit(1);
  }
  return 0;
}

void controlCSignalHandler(int signal)
{
  cout << "Please don't exit the client with ctl-c. Only use /exit, /part, or /quit";
}

void* readFromServer(void* argument)
{
  string str;

  while(true)
  {
    pthread_mutex_lock(&mutex);
    read(socketNum, buffer, 512);

    str = buffer;
    pthread_mutex_unlock(&mutex);
    //Server told client to quit
    if(str == QUIT_COMMAND)
    {
      return NULL;
    }
    cout << str << endl;
  }
}

void* writeToServer(void* argument)
{
  char* x;
  string sx = username + ": ";
  char* s;
  strcpy(s, sx.c_str());
  while(true)
  {
    pthread_mutex_lock(&mutex);
    x = gets(buffer);

    if(x == NULL)
    {
      return NULL;
    }
    write(socketNum, strcat(s, buffer), BUFFER_SIZE);
    pthread_mutex_unlock(&mutex);
  }
}