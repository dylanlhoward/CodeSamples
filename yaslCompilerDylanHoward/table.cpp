//Dylan Howard, Revised November 2011
//This file contains method implementations for the
//table and TableCell and TableList classes

//The class header is found in table.h

#include "stdafx.h"  // Required for visual studio

#include "table.h"
#include <string>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>    //to allow exit

using namespace std;

tableClass::tableClass()
{
	top = NULL;
}

void tableClass::addLevel(string theScope)
{
	TableCell *tempCell = new TableCell();
	tempCell->offset = 0;
	if (top == NULL)
	{
		tempCell->nesting = 0;
		tempCell->scope = theScope;
	}
	else
	{
		tempCell->nesting = top->nesting + 1;
		tempCell->scope = top->scope + "." + theScope;
	}
	tempCell->theList = NULL;
	tempCell->next = top;
	top = tempCell;
}

void tableClass::deleteLevel()
{
	TableCell *tempCell = new TableCell();
	if(top == NULL)
		cout << "Error: No table levels to delete" << endl;
	else
	{
		tempCell = top->next;
		top = tempCell;
	}
}

bool tableClass::addEntry(string theLexeme, string theKind, string theType)
{
	TableList *tempList = new TableList();
	tempList->lexeme = theLexeme;
	tempList->kind = theKind;
	tempList->type = theType;
	if(top == NULL)
	{
		cout << "Error: Cannot add entry to table with no levels" << endl;
		return false;
	}
	else 
	{
		TableList *checkList = new TableList();
		checkList = top->theList;
		while(checkList != NULL)
		{
			if(checkList->lexeme == theLexeme)
			{
				cout << "Error (Duplicate Identifier): " << theLexeme << " is already declared in " << top->scope << endl;
				return false;
			}
			checkList = checkList->next;
		} 
		tempList->params = NULL;
		tempList->nesting = top->nesting;
		tempList->offset = top->offset;
		if(tempList->kind == "VAR-ID" || tempList->kind == "REF-PARAM" || tempList->kind == "VAL-PARAM" || tempList->kind == "CONST" || tempList->kind == "SPEC")
			top->offset++;
		tempList->next = top->theList;
		top->theList = tempList;
		return true;
	}
}

void tableClass::assignParameters()  //call when done parsing parameter declarations
{
	TableList *tempList = new TableList();
	TableList *paramList = new TableList();
	TableList *addOn = new TableList();
	tempList = top->theList;
	paramList = (top->next)->theList;
	while(tempList != NULL)
	{
		TableList *addOn = new TableList();
		addOn->lexeme = tempList->lexeme;
		addOn->kind = tempList->kind;
		addOn->type = tempList->type;
		addOn->offset = tempList->offset;
		addOn->nesting = tempList->nesting;
		addOn->params = NULL;

		addOn->next = paramList->params;
		paramList->params = addOn;
		
		tempList = tempList->next;
	}
	((top->next)->theList)->params = paramList->params;
}

TableList* tableClass::tableLookup(string theLexeme)
{
	TableList *tempList = new TableList();
	TableCell *tempCell = new TableCell();
	TableList *holder = new TableList();
	tempCell = top;
	while(tempCell != NULL)
	{
		tempList = tempCell->theList;
		holder = tempCell->theList;
		while(tempList != NULL)
		{
			if(tempList->lexeme == theLexeme)
			{
				if(holder != tempList)
				{
					holder->next = tempList->next;
					tempList->next = tempCell->theList;
					tempCell->theList = tempList;
				}
				return tempList;
			}
			if(holder != tempList)
				holder = holder->next;
			tempList = tempList->next;
		}
		tempCell = tempCell->next;
	}
	return NULL;
}

bool tableClass::countParameters(string lexeme, int &num)
{
	TableList *tempList = tableLookup(lexeme);
	tempList = tempList->params;
	while(tempList != NULL)
	{
		num--;
		tempList = tempList->next;
	}
	if(num == 0)
		return true;
	else
		return false;
}

void tableClass::tableDump()
{
	TableList *tempList = new TableList();
	TableCell *tempCell = new TableCell();
	TableList *paramList = new TableList();
	tempCell = top;
	cout << "******************************************************************" << endl;
	while(tempCell != NULL)
	{
		tempList = tempCell->theList;
		cout << "Name = " << tempCell->scope << " Nesting level = " << tempCell->nesting << endl;
		while(tempList != NULL)
		{
			paramList = tempList->params;
			cout << "lexeme = " << tempList->lexeme << ", kind = " << tempList->kind << ", type = " << tempList->type;
			cout << ", offset = " << tempList->offset << ", nesting level = " << tempList->nesting << endl;
			if(tempList->kind == "FUNC-ID")
				cout << "\tparam-list:" << endl;
			while(paramList != NULL)
			{
				cout << "\t\tlexeme = " << paramList->lexeme << ", kind = " << paramList->kind << ", type = " << paramList->type;
				cout << ", offset = " << paramList->offset << endl;
				paramList = paramList->next;
			}
			tempList = tempList->next;
		}
		tempCell = tempCell->next;	
	}
	cout << "******************************************************************" << endl;
}

void tableClass::deleteTemps()
{
	TableList *tempList = new TableList();
	TableList *tempList2 = new TableList();
	bool notDoneYet = true;
	int numRemoved = 0;
	while(notDoneYet)
	{
		tempList = top->theList;
		if(tempList == NULL)
		{
			notDoneYet = false;
		}
		else if(tempList->lexeme[0] == '$' && tempList->lexeme[1] != 's')
		{
			tempList2 = tempList->next;
			numRemoved++;
			while(tempList2 != NULL && tempList2->lexeme[0] == '$' && tempList2->lexeme[1] != 's')
			{
				tempList2 = tempList2->next;
				numRemoved++;
			}
			if(tempList2 == NULL)
				notDoneYet = false;
			top->theList = tempList2;
		}
		else
		{
			tempList2 = tempList->next;
			while(tempList2 != NULL && (tempList2->lexeme[0] != '$' || tempList2->lexeme[1] == 's'))
			{
				tempList = tempList->next;
				tempList2 = tempList2->next;
			}
			if(tempList2 == NULL)
			{
				notDoneYet = false;
			}
			else
			{
				tempList2 = tempList->next;
				numRemoved++;
				while(tempList2 != NULL && tempList2->lexeme[0] == '$' && tempList2->lexeme[1] != 's')
				{
					tempList2 = tempList2->next;
					numRemoved++;
				}
				if(tempList2 == NULL)
					notDoneYet = false;
				tempList->next = tempList2;
			}
		}
	}
	top->offset -= numRemoved;
}

void tableClass::deleteSpecialTemps()
{
	TableList *tempList = new TableList();
	TableList *tempList2 = new TableList();
	bool notDoneYet = true;
	int numRemoved = 0;
	while(notDoneYet)
	{
		tempList = top->theList;
		if(tempList == NULL)
		{
			notDoneYet = false;
		}
		else if(tempList->lexeme[0] == '$' && tempList->lexeme[1] == 's')
		{
			tempList2 = tempList->next;
			//numRemoved++;
			while(tempList2 != NULL && tempList2->lexeme[0] == '$' && tempList2->lexeme[1] == 's')
			{
				tempList2 = tempList2->next;
				numRemoved++;
			}
			if(tempList2 == NULL)
				notDoneYet = false;
			top->theList = tempList2;
		}
		else
		{
			tempList2 = tempList->next;
			while(tempList2 != NULL && (tempList2->lexeme[0] != '$' || tempList2->lexeme[1] != 's'))
			{
				tempList = tempList->next;
				tempList2 = tempList2->next;
			}
			if(tempList2 == NULL)
			{
				notDoneYet = false;
			}
			else
			{
				tempList2 = tempList->next;
				//numRemoved++;
				while(tempList2 != NULL && tempList2->lexeme[0] == '$' && tempList2->lexeme[1] == 's')
				{
					tempList2 = tempList2->next;
					numRemoved++;
				}
				if(tempList2 == NULL)
					notDoneYet = false;
				tempList->next = tempList2;
			}
		}
	}
	top->offset -= numRemoved;
}

void tableClass::addFunctionLabel(string theTemp) //Should only be used immediately after adding a function entry to the table (before adding the new scope level)
{
	top->theList->label = theTemp;
}

void tableClass::addBodyLabel(string theTemp)
{
	top->theList->bodyLabel = theTemp;
}

int tableClass::getCurrentNest()
{
	return top->nesting;
}

TableCell::TableCell()
{

}

TableList::TableList()
{
	label = "";
	bodyLabel = "";
}
