//Dylan Howard Revised November 2011
//This file contains class definitions for the table class, TableCell class and
//the TableList class, which will comprise the symbol table

//The definitions of these routines are found in table.cpp

#ifndef _table
#define _table

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>    //to allow exit

using namespace std;

class TableList
{
	public:
		TableList();
		string lexeme;
		string kind;
		string type;
		int offset;
		int nesting;
		TableList *next;
		TableList *params;
		string label;
		string bodyLabel;

	private:

};

class TableCell
{
	public:
		TableCell();
		TableList *theList;
		TableCell *next;
		string scope;
		int nesting;
		int offset;

	private:

};

class tableClass
{
	public:
		tableClass();
		void addLevel(string theScope);
		void deleteLevel();
		bool addEntry(string theLexeme, string theKind, string theType);
		TableList* tableLookup(string theLexeme);
		void tableDump();
		void deleteTemps();
		void deleteSpecialTemps();
		void assignParameters();
		bool countParameters(string lexeme, int &num);
		void addFunctionLabel(string theTemp);
		void addBodyLabel(string theTemp);
		int getCurrentNest();
		

	private:
		TableCell *top;
};

#endif