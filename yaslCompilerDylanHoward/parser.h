//Dylan Howard Revised October 2011
//This file contains class definitions for the parser class 
//

//The definitions of these routines are found in parser.cpp

#ifndef _parser
#define _parser

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>    //to allow exit
#include "scanner.h"
#include "pstack.h"

using namespace std;
using std::ifstream;

class parserClass
{	public:
		parserClass();
		void parseProgram();
		int numLinesProcessed();
		string finalType;

	private:
		int currParams;
		int globals;
		string getNextTemp();
		fstream outfile;
		scannerClass theScanner;
		pstackClass theStack;
		tokenClass theArray[5];
		TableList *ptrArray[5];
		int arrayItems;
		tokenClass theToken;
		string precedence(tokenClass row, tokenClass column);
		bool terminalPopped();
		tokenClass lastTermPopped;
		bool isValidRHS(tokenClass array1[], TableList *array2[]);
		void parseBlock(string bodyLabel);
		void parseStatement();
		void parseExpression();
		void varDecs();
		void varDecs(string bodyLabel);
		void funcDecs();
		void funcDecTail();
		void typeTail(string type);
		void paramList();
		void paramTail();
		void identList(string type);
		void identTail(string type);
		void progBody();
		void followBegin();
		void statementTail();
		void followIf(string after, string otherwise);
		void followIdent(string theLexeme);
		void followExpression(TableList* tempPtr);
		void followCIN();
		void followCOUT();
		void quitParse(string expected, int x);
		void undecIdent(string theLexeme);
		int parCount;
		void wrongParams(string theLexeme, int needed, int received);
		void wrongType(string got, string expected);
		void otherError(string message);
		void printInstruction(string command, TableList* ptr1, string str1, TableList* ptr2, string str2);
};

#endif
