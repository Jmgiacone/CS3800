#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <vector>
#include "Page.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::ifstream;
using std::ofstream;

bool checkParameters(int argc, char* argv[],ifstream& programListIn, ifstream& programTraceIn, int& pageSize, string& replacementAlgorithm, string& pagingMethod);
void replacePage(int requestingProgram, int requestedPage);
int getIndexOfOldestPage();

const string REPLACEMENT_ALGORITHMS[3] = {"lru", "clock", "fifo"},
             PAGING_METHODS[2] = {"d", "p"};
const int NUM_REPLACEMENT_ALGORITHMS = 3, NUM_PAGING_METHODS = 2, AVAILABLE_FRAMES = 512, NUM_PROGRAMS = 10;

//Global variables
unsigned long long timestamp = 0;
int pageSize, pagesPerProgram, numFrames, pageFaults = 0, clockIndex = 0;
int programSizes[NUM_PROGRAMS];
string replacementAlgorithm, pagingMethod;
Page** mainPageTable;
Page** pageTables[NUM_PROGRAMS];

int main(int argc, char* argv[])
{
  std::ifstream programListIn, programTraceIn;
  int x, tmp, requestingProgram, requestedPage, pagesInMainMemory = 0;

  if(!checkParameters(argc, argv, programListIn, programTraceIn, pageSize, replacementAlgorithm, pagingMethod))
  {
    //Error condition
    return -1;
  }

  //We're ready to roll
  numFrames = AVAILABLE_FRAMES / pageSize;

  //Main page table is an array of pointers to pages
  mainPageTable = new Page*[numFrames];

  for(int i = 0; i < numFrames; i++)
  {
    mainPageTable[i] = NULL;
  }

  for(int i = 0; i < NUM_PROGRAMS; i++)
  {
    //Skip first part
    programListIn >> x;

    //Grab program size
    programListIn >> x;

    //Program i needs size/pageSize pages
    programSizes[i] = std::ceil(static_cast<double>(x) / pageSize);

    //Page table contains as many entries as pages needed
    pageTables[i] = new Page*[programSizes[i]];

    //Set each element to NULL
    for(int j = 0; j < programSizes[i]; j++)
    {
      pageTables[i][j] = NULL;
    }

    //Time to pre-load each program's pages
    pagesPerProgram = numFrames / NUM_PROGRAMS;

    //If program i needs fewer than pagesPerProgram, only loop that many times
    x = programSizes[i] < pagesPerProgram ? programSizes[i] : pagesPerProgram;

    for (int j = 0; j < x; j++)
    {
      tmp = i * pagesPerProgram + j;
      mainPageTable[tmp] = new Page(i, j, timestamp);
      pagesInMainMemory++;
      pageTables[i][j] = mainPageTable[tmp];
      timestamp++;
    }
  }

  //Start reading in the program trace
  while(!programTraceIn.eof())
  {
    programTraceIn >> requestingProgram;
    programTraceIn >> requestedPage;

    //Subtract 1 because the pages are 1-indexed for whatever reason
    requestedPage--;

    //Purposeful integer division, get floor of page
    requestedPage /= pageSize;

    //Page isn't resident
    if(pageTables[requestingProgram][requestedPage] == NULL)
    {
      pageFaults++;

      //There's space in main memory
      if(pagesInMainMemory != numFrames)
      {
        int chosenPage = pagesInMainMemory;

        //Search for the first empty slot
        for(int i = 0; i < numFrames; i++)
        {
          //We found our slot
          if(mainPageTable[i] == NULL)
          {
            chosenPage = i;
          }
        }

        //Simply put the page in the blank slot
        mainPageTable[chosenPage] = new Page(requestingProgram, requestedPage, timestamp);

        //Set the pointer
        pageTables[requestingProgram][requestedPage] = mainPageTable[chosenPage];
        timestamp++;
        pagesInMainMemory++;
      }
      else
      {
        //Run a replacement algorithm
        replacePage(requestingProgram, requestedPage);

        //If we're using prepaging and requestedPage isn't the last logical page
        if(pagingMethod == "p" && requestedPage+1 < programSizes[requestingProgram])
        {
          //If next page isn't resident
          if(pageTables[requestingProgram][requestedPage+1] == NULL)
          {
            replacePage(requestingProgram, requestedPage + 1);
          }
          else
          {
            //If using lru, update timestamp
            if(replacementAlgorithm == "lru")
            {
              pageTables[requestingProgram][requestedPage+1]->setTimestamp(timestamp);
              timestamp++;
            }
            else if(replacementAlgorithm == "clock")
            {
              pageTables[requestingProgram][requestedPage+1]->setUseBit(true);
            }
          }
        }
      }
    }
    else
    {
      //Page is resident

      //If using lru, update timestamp
      if(replacementAlgorithm == "lru")
      {
        pageTables[requestingProgram][requestedPage]->setTimestamp(timestamp);
        timestamp++;
      }
      else if(replacementAlgorithm == "clock")
      {
        pageTables[requestingProgram][requestedPage]->setUseBit(true);
      }
    }
  }

  //Close filestreams and delete data
  programListIn.close();
  programTraceIn.close();
  for(int i = 0; i < NUM_PROGRAMS; i++)
  {
    delete[] pageTables[i];
  }

  for(int i = 0; i < numFrames; i++)
  {
    delete mainPageTable[i];
  }
  delete[] mainPageTable;

  cout << "There were " << pageFaults << " page faults" << endl;

  return 0;
}

bool checkParameters(int argc, char* argv[], ifstream& programListIn, ifstream& programTraceIn, int& pageSize, string& replacementAlgorithm, string& pagingMethod)
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

void replacePage(int requestingProgram, int requestedPage)
{
  int indexOfPageToBeReplaced = 0;

  //LRU and FIFO replace pages the same way. The only difference is in how they update timestamps
  if(replacementAlgorithm == "lru" || replacementAlgorithm == "fifo")
  {
    indexOfPageToBeReplaced = getIndexOfOldestPage();
  }
  else if(replacementAlgorithm == "clock")
  {
    //clockIndex = getIndexOfOldestPage();
    while(mainPageTable[clockIndex]->getUseBit())
    {
      //Clear the bit
      mainPageTable[clockIndex]->setUseBit(false);
      clockIndex++;
      clockIndex %= numFrames;
    }
    indexOfPageToBeReplaced = clockIndex;
  }

  //Replace page at index lruIndex
  Page p = *mainPageTable[indexOfPageToBeReplaced];

  //Page p is no longer resident
  pageTables[p.getOwnerProgram()][p.getLogicalPageNum()] = NULL;

  delete mainPageTable[indexOfPageToBeReplaced];
  mainPageTable[indexOfPageToBeReplaced] = new Page(requestingProgram, requestedPage, timestamp);
  pageTables[requestingProgram][requestedPage] = mainPageTable[indexOfPageToBeReplaced];
  timestamp++;
}

int getIndexOfOldestPage()
{
  int index = 0;

  //Find index of page with oldest timestamp
  for(int i = 0; i < numFrames; i++)
  {
    if(mainPageTable[i]->getTimestamp() < mainPageTable[index]->getTimestamp())
    {
      index = i;
    }
  }
  
  return index;
}