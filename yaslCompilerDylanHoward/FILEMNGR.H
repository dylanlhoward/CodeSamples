//Dave A. Berque, Revised August 2011 by Dylan Howard
//This file contains a class definition for managing the
//file that contains the YASL source program to be compiled.

//The definitions of these routines are found in filemngr.cpp

#ifndef _filemngr
#define _filemngr

#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>    //to allow exit
#include "general.h"

using namespace std;
using std::ifstream;

class fileManagerClass
{ public:
     fileManagerClass();
     int getNextChar();
     void closeSourceProgram();
	 void pushBack();
	 void printCurrentLine();
	 void setPrintStatus(bool newStatus);
	 int numLinesProcessed();

  private:
     char currentLine[MAXCELLSPERSTRING];
     int charToReadNext;
     int currentLineNumber;
     ifstream fileToCompile;
	 bool autoPrintStatus;
}  ;

#endif
