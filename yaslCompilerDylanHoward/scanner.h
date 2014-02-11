//Dylan Howard Revised September 2011
//This file contains class definitions for the scanner class 
//and the token class which will be used for lexical analysis.

//The definitions of these routines are found in scanner.cpp

#ifndef _scanner
#define _scanner

#include "FILEMNGR.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>    //to allow exit
#include "general.h"
#include "table.h"

using namespace std;
using std::ifstream;

class tokenClass
{	public:
		tokenClass();
		void display();
		void setType(string newType);
		void setSubtype(string newSubtype);
		void setLexeme(string newLexeme);
		void addToLexeme(char ch);
		void clearToken();
		bool isEOF();
		bool isLexError();
		void foundLexError();
		void pushBackLexeme();
		string getLexeme();
		string getType();
		string getSubtype();

	private:
		string type;
		string subtype;
		string lexeme;
		bool lexError;
} ;

class scannerClass
{	public:
		scannerClass();
		tokenClass getNextToken();
		void closeSourceProgram();
		int getNextState(char ch);
		int numLinesProcessed();
		bool expDebugging();
		void printCurrentLine();
		void addTableLevel(string theScope);
		void deleteTableLevel();
		void deleteTemps();
		void deleteSpecialTemps();
		void addTableEntry(string theLexeme, string theKind, string theType);
		TableList* tableLookup(string theLexeme);
		void tableDump();
		void assignParameters();
		bool countParameters(string lexeme, int &num);
		void addFunctionLabel(string theTemp);
		void addBodyLabel(string theTemp);
		int getCurrentNest();

	private:
		fileManagerClass theFile;
		int lexemeLength;
		int state;
		tokenClass theToken;
		void processDirective();
		bool expDeb;
		tableClass theTable;
		//int getNextState();

} ;


#endif