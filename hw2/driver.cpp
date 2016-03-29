#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::ifstream;
using std::ofstream;

bool checkParameters(int argc, char* argv[],ifstream& programListIn, ifstream& programTraceIn, int& pageSize, string replacementAlgorithm, string pagingMethod);

const string REPLACEMENT_ALGORITHMS[3] = {"lru", "clock", "fifo"},
             PAGING_METHODS[2] = {"d", "p"};
const int NUM_REPLACEMENT_ALGORITHMS = 3, NUM_PAGING_METHODS = 2;
int main(int argc, char* argv[])
{
  std::ifstream programListIn, programTraceIn;
  std::ofstream fileOut;
  int pageSize;
  string replacementAlgorithm, pagingMethod;

  if(checkParameters(argc, argv, programListIn, programTraceIn, pageSize, replacementAlgorithm, pagingMethod))
  {
    //We're ready to roll
  }

  return 0;
}

bool checkParameters(int argc, char* argv[], ifstream& programListIn, ifstream& programTraceIn, int& pageSize, string replacementAlgorithm, string pagingMethod)
{
  bool pageSizeGood = false, replacementAlgorithmGood = false, pagingMethodGood = false;

  //Valid number of arguments, but some may not make sense
  if(argc == 6)
  {
    //Check to see if programList exists
    string filename = argv[1];
    programListIn.open(filename + ".txt");

    if(programListIn)
    {
      //File1 exists

      //Check to see if file2 exists
      filename = argv[2];
      programTraceIn.open(filename + ".txt");

      if(programTraceIn)
      {
        //File2 exists
        try
        {
          pageSize = std::stoi(argv[3]);

          //Page Size is numeric, but is it 1,2,4,8,16?
          for(int i = 0; i < 5; i++)
          {
            if(pageSize == std::pow(2, i))
            {
              //It is
              pageSizeGood = true;
              break;
            }
          }

          if(pageSizeGood)
          {
            //Page size is good

            //Check replacement algorithm
            replacementAlgorithm = argv[4];

            for(int i = 0; i < NUM_REPLACEMENT_ALGORITHMS; i++)
            {
              if(replacementAlgorithm == REPLACEMENT_ALGORITHMS[i])
              {
                //Replacement algorithm is valid
                replacementAlgorithmGood = true;
                break;
              }
            }

            if(replacementAlgorithmGood)
            {
              //Replacement algorithm is valid

              //Check paging method
              pagingMethod = argv[5];

              for(int i = 0; i < NUM_PAGING_METHODS; i++)
              {
                if(pagingMethod == PAGING_METHODS[i])
                {
                  pagingMethodGood = true;
                  break;
                }
              }

              if(pagingMethodGood)
              {
                //Paging Method is valid

                //All Options are valid
                return true;
              }
              else
              {
                //Paging Method is invalid
                cerr << "Error: Paging Method " << pagingMethod << " is invalid." << endl;
              }
            }
            else
            {
              //It's not
              cerr << "Error: Page Replacement Algorithm " << replacementAlgorithm << " is invalid." << endl;
            }

          }
          else
          {
            //Page size isn't good
            cerr << "Error: " << pageSize << " is not a power of two." << endl;
          }


        }
        catch(std::invalid_argument e)
        {
          //Third argument is not an integer
          cerr << "Error: " << pageSize << " is not a numeric type." << endl;
        }
      }
      else
      {
        //It doesn't
        cerr << "Error: File " << argv[2] << ".txt does not exist. Please provide a valid filename" << endl;
      }
    }
    else
    {
      //It doesn't
      cerr << "Error: File " << argv[1] << ".txt does not exist. Please provide a valid filename" << endl;
    }
  }
  else
  {
    //Invalid number of parameters
    cerr << "Error: Invalid number of parameters provided. Please provide exactly 5 arguments." << endl;
  }

  cerr << endl;
  cerr << "Usage " << argv[0] << " programListFile programTraceFile numPages replacementAlgorithm pagingMethod" << endl;
  cerr << endl;
  cerr << "programListFile:      The file containing the program list" << endl;
  cerr << "programTraceFile:     The file containing the program trace" << endl;
  cerr << "numPages:             The number of pages. Must be in the set {1,2,4,8,16}" << endl;
  cerr << "replacementAlgorithm: The replacement algorithm to use. Must be in the set {lru, fifo, clock}" << endl;
  cerr << "pagingMethod:         The paging method to use. Must be in the set {p(Prepaging), d(Demand Paging)}" << endl;

  return false;
}