//Dave A. Berque, Revised August 2011 by Dylan Howard
//This file contains method implementations for the
//fileManagerClass

//The class header is found in filemngr.h

#include "stdafx.h"  // Required for visual studio

#include "filemngr.h"


fileManagerClass::fileManagerClass()
//Precondition: None.
//Postcondition: The constructor has opened the file which is to be
//               processed and has inialized the current line number to 0
{  char filename[MAXCELLSPERSTRING];
	autoPrintStatus = false;

   cout << "Enter file name to compile: ";
   cin.getline (filename, MAXCELLSPERSTRING);
   fileToCompile.open(filename);

   if (fileToCompile.fail())
   {  cout << "Error, the file: " << filename << " was not opened." << endl;
      cout << "Press return to terminate program." << endl;
      cin.get();
      exit(1);
   }  
  currentLineNumber = 0;
}

int fileManagerClass::getNextChar()
//Precondition:  The source file associated with the owning object has
//               been prepared for reading.
//Postcondition: The next character from the input file has been read and
//               returned by the function.  If another chacter could not
//               be read because END OF FILE was reached then EOF is
//               returned.
{  if ((currentLineNumber == 0) ||
      (charToReadNext == strlen(currentLine)))
   {  if (fileToCompile.peek() == EOF) 
         return (EOF);
	  fileToCompile.getline(currentLine, MAXCELLSPERSTRING);
      strcat(currentLine, "\n");
      currentLineNumber++;
      charToReadNext = 0;
	  if(autoPrintStatus == true)
		  printCurrentLine();
   }
   return(currentLine[charToReadNext++]);
}


void fileManagerClass::closeSourceProgram()
//Precondition:  The file belonging to the object owning this routine has
//               been opened.
//Postcondition: The file belonging to the object owning this routine has
//               been closed.
{  fileToCompile.close();
}

void fileManagerClass::pushBack()
{
	if(charToReadNext != 0)
	{
		charToReadNext--;
	}
}

void fileManagerClass::printCurrentLine()
{
	cout << currentLineNumber << " - " << currentLine;
}

void fileManagerClass::setPrintStatus(bool newStatus)
{
	autoPrintStatus = newStatus;
}

int fileManagerClass::numLinesProcessed()
{
	return currentLineNumber;
}


