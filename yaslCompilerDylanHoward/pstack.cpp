//Dylan Howard, Revised October 2011
//This file contains method implementations for the
//pstack and StackCell classes

//The class header is found in pstack.h

#include "stdafx.h"  // Required for visual studio

#include "pstack.h"
#include <string>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>    //to allow exit

using namespace std;

pstackClass::pstackClass()
{
	top = NULL;
}

void pstackClass::push(tokenClass theToken, TableList *ptr)
{
	StackCell *tempCell = new StackCell;
	tempCell->next = top;
	top = tempCell;
	tempCell->type = theToken.getType();
	if(ptr == NULL)
		tempCell->subtype = theToken.getSubtype();
	else
		tempCell->subtype = ptr->type;
	tempCell->lexeme = theToken.getLexeme();
	tempCell->ptr = ptr;
}

TableList* pstackClass::topPtr()
{
	return top->ptr;
}

tokenClass pstackClass::pop()
{
	tokenClass theToken;
	theToken.setType(top->type);
	theToken.setSubtype(top->subtype);
	theToken.setLexeme(top->lexeme);
	top = top->next;
	return theToken;
}

tokenClass pstackClass::getTopTerminal()
{
	StackCell *tempCell = new StackCell;
	tempCell = top;
	tokenClass temp;
	while (tempCell != NULL)
	{
		if(tempCell->type == "E_T")
			tempCell = tempCell->next;		
		else
		{	temp.setType(tempCell->type);
			temp.setSubtype(tempCell->subtype);
			return temp;	}
	}
	return temp;
}

bool pstackClass::topIsTerm()
{
	if(top->type == "E_T")
		return false;
	else
		return true;
}

StackCell::StackCell()
{
	subtype = "NONE";
}