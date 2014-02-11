

//Dave A. Berque, Revised August 2011 by Dylan Howard
//This driver program should be used to test the first part of the YASL
//compiler.

//This version tested in Visual Studio .NET 2008


#include "stdafx.h"  // A visual studio requirement
#include "parser.h"

#include <iostream>

int main(int argc, char* argv[])
{   
	//fileManagerClass theFile;     //Define the sourceprogram object
	parserClass theParser;
	//tokenClass token1, token2;
	/*scannerClass theScanner;
    char ch;
	int count=0;
	tokenClass theToken;*/
	theParser.parseProgram();

	cout << "YASL-DH compiled " << theParser.numLinesProcessed() << " lines of code successfully." << endl;
	//theFile.setPrintStatus(true);
    /*while ((ch = theFile.getNextChar()) != EOF)
    {  
		if(count > 20)
			theFile.setPrintStatus(true);
		cout << ch;
		count++;
    }
	if(!(theToken.isLexError()))
		cout << "YASLC-DH has just compiled " << theScanner.numLinesProcessed() << " lines of code.";
	theScanner.closeSourceProgram();*/
    cin.get();

    return (0);
}

