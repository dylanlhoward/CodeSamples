//Dylan Howard Revised October 2011
//This file contains class definitions for the pstack class 
//

//The definitions of these routines are found in pstack.cpp

#ifndef _pstack
#define _pstack

#include <string>
#include <iostream>
#include <fstream>
#include "scanner.h"

using namespace std;
using std::ifstream;


class StackCell
{	public:
		StackCell();
		string type;
		string subtype;
		string lexeme;
		StackCell *next;
		TableList *ptr;
		
	private:
		
};

class pstackClass
{	public:
		pstackClass();
		void push(tokenClass theToken, TableList *ptr);
		TableList* topPtr();
		tokenClass pop();
		bool topIsTerm();
		tokenClass getTopTerminal();

	private:
		StackCell *top;
		
};

#endif