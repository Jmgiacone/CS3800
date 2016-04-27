#include <iostream>
#include <signal.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;
const int PORT_NUMBER = 9999;

void controlCSignalHandler(int signal);

int main(int argc, char* argv[])
{
  string username = "";
  int clientId = 0;

  //A single CLI argument. Hopefully it's the hostname
  if(argc == 2)
  {
    //Tell the OS to run function 'controlCSignalHandler' when a SIGINT(ctl-c) signal is raised
    //Cool use of function pointers
    signal(SIGINT, controlCSignalHandler);

    //Prompt the user for their name
    cout << "Please enter a username: ";
    cin >> username;
  }
  else
  {
    std::cout << "Error: No hostname provided. Usage: " << argv[0] << " hostname" << endl;
  }
  return 0;
}

void controlCSignalHandler(int signal)
{
  cout << "Please don't exit the client with ctl-c. Only use /exit, /part, or /quit";
}