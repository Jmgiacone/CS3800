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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
const int BUFFER_SIZE = 512;
int socketNum = -1;
const string QUIT_COMMAND = "/quit";
string username = "";
bool quitting = false;
pthread_t readThread, writeThread;
int main(int argc, char* argv[])
{
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
              pthread_join(readThread, NULL);
              pthread_join(writeThread, NULL);

              //pthread_exit(&writeThread);
              //pthread_exit(&readThread);

              close(socketNum);
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
  string str;
  char buffer[BUFFER_SIZE];

  while(!quitting)
  {
    bzero(buffer, BUFFER_SIZE);
    read(socketNum, buffer, BUFFER_SIZE);

    str = buffer;

    //Server told client to quit
    if(str == QUIT_COMMAND)
    {
      cout << ">>" << str << endl;
      cout << "Server sent quit command. Quitting..." << endl;
      quitting = true;
    }

    //cout << "Server says: " << str << endl;
    cout << str << endl;
    pthread_yield();
  }

  return NULL;
}

void* writeToServer(void* argument)
{
  char buffer[BUFFER_SIZE];

  while(!quitting)
  {
    bzero(buffer, BUFFER_SIZE);
    cin.getline(buffer, BUFFER_SIZE);

    if(isQuitCommand(buffer))
    {
      cout << "Valid quit command detected. Now quitting..." << endl;
      quitting = true;
    }

    //Don't send blank lines
    if(strcmp(buffer, "") != 0)
    {
      //cout << "About to send this to server: " << buffer << endl;
      write(socketNum, buffer, BUFFER_SIZE);
    }
    pthread_yield();
  }

  return NULL;
}

bool isQuitCommand(char* x)
{
  return strcmp(x, "/quit") == 0  || strcmp(x, "/part") == 0 || strcmp(x, "/exit") == 0;
}