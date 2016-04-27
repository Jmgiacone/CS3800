#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;
const int PORT_NUMBER = 9999;

void controlCSignalHandler(int signal);

int main(int argc, char* argv[])
{
  string username = "";
  int socketNum = 0;
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