//Dylan Howard, Revised October 2011
//This file contains method implementations for the
//parserClass

//The class header is found in parser.h

#include "stdafx.h"  // Required for visual studio

#include "parser.h"
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <sstream>

using namespace std;

int currParams;

parserClass::parserClass()
{
	arrayItems = 0;
	outfile.open("out.pal", ios::out);
	globals = 0;
}

void parserClass::parseProgram()
{
	theToken = theScanner.getNextToken();
	if (theToken.getType() == "PROGRAM_T"){
		theToken = theScanner.getNextToken();
		if(theToken.getType() == "IDENT_T"){
			string temp = theToken.getLexeme();
			theToken = theScanner.getNextToken();
			if(theToken.getType() == "SEMICOL_T"){
				theScanner.addTableLevel(temp);
				outfile << "$main movw SP R0" << endl;
				theToken = theScanner.getNextToken();
				parseBlock("");
				if(theToken.getType() == "PERIOD_T"){
					//program parsed with no errors
					theScanner.deleteTableLevel();
					outfile << "end" << endl;
				}
				else
					quitParse(".", 90);
			}
			else
				quitParse(";", 2);
		}
		else
			quitParse("Valid Identifier", 1);
	}
	else
		quitParse("PROGRAM", 0);
}
void parserClass::parseBlock(string bodyLabel)
{
	globals = 0;
	if(bodyLabel == "")
		varDecs();
	else
		varDecs(bodyLabel);
	funcDecs();
	if(bodyLabel != "")
		printInstruction(bodyLabel + " movw", NULL, "R0", NULL, "R0");
	progBody();
}
void parserClass::parseStatement()
{
	if(theToken.getType() == "IDENT_T"){
		if(theScanner.tableLookup(theToken.getLexeme()) == NULL){
			undecIdent(theToken.getLexeme());
		}
		string temp = theToken.getLexeme();
		theToken = theScanner.getNextToken();
		followIdent(temp);
	}
	else if(theToken.getType() == "COUT_T"){
		theToken = theScanner.getNextToken();
		followCOUT();
	}
	else if(theToken.getType() == "CIN_T"){
		theToken = theScanner.getNextToken();
		followCIN();
	}
	else if(theToken.getType() == "BEGIN_T"){
		theToken = theScanner.getNextToken();
		followBegin();
	}
	else if(theToken.getType() == "WHILE_T"){
		theToken = theScanner.getNextToken();
		string top = getNextTemp();
		printInstruction(top + " movw", NULL, "R0", NULL, "R0");
		parseExpression();
		if (finalType != "BOOLEAN_T")
				wrongType("Something else", "BOOLEAN");
		printInstruction("cmpw", NULL, "#0", NULL, "-4@SP");
		printInstruction("subw", NULL, "#4", NULL, "SP");
		string after = getNextTemp();
		printInstruction("beq", NULL, after, NULL, "");
		if(theToken.getType() == "DO_T"){
			theToken = theScanner.getNextToken();
			parseStatement();
			printInstruction("jmp", NULL, top, NULL, "");
			printInstruction(after + " movw", NULL, "R0", NULL, "R0");
		}
		else{
			quitParse("DO", 3);
		}
	}
	else if(theToken.getType() == "IF_T"){
		theToken = theScanner.getNextToken();
		parseExpression();
		if (finalType != "BOOLEAN_T")
				wrongType("Something else", "BOOLEAN");
		string afterElse = getNextTemp();
		string otherwise = getNextTemp();
		printInstruction("cmpw", NULL, "#0", NULL, "-4@SP");
		printInstruction("subw", NULL, "#4", NULL, "SP");
		printInstruction("beq", NULL, otherwise, NULL, "");
		if(theToken.getType() == "THEN_T"){
			theToken = theScanner.getNextToken();
			parseStatement();
			followIf(afterElse, otherwise);
		}
		else
			quitParse("THEN", 6);
	}
	else{
		if(theToken.getType() != "EOF_T")
		{
			quitParse("WHILE, IF, Identifier, COUT, CIN, BEGIN", 5);
		}
	}
}

void parserClass::parseExpression()
{
	tokenClass first;
	first.setType("SEMICOL_T");
	theStack.push(first, NULL);
	printInstruction("movw", NULL, "SP", NULL, "R1");
	//theToken = theScanner.getNextToken();  //-removed for Project 4
	if(theToken.getType() != "LEFTPAREN_T" && theToken.getType() != "IDENT_T" && theToken.getType() != "INTEGER_T" && theToken.getType() != "TRUE_T" && theToken.getType() != "FALSE_T"){
		cout << "Syntax Error: Invalid Expression" << endl;
		theScanner.printCurrentLine();
		cin.get();
		exit(4);
	}
	while(true)
	{
		if(theStack.getTopTerminal().getType() == "SEMICOL_T" && (theToken.getType() == "SEMICOL_T" || theToken.getType() == "DO_T" || theToken.getType() == "THEN_T" || theToken.getType() == "RIGHTPAREN_T" || theToken.getType() == "COMMA_T" || theToken.getType() == "ELSE_T" || theToken.getType() == "END_T" || theToken.getType() == "STRINS_T"))
		{
			TableList *theAnswer = theStack.topPtr();
			printInstruction("movw", theAnswer, "", NULL, "@R1");
			printInstruction("movw", NULL, "R1", NULL, "SP");
			printInstruction("addw", NULL, "#4", NULL, "SP");
			finalType = theAnswer->type;
			theScanner.deleteTemps();
			return;
		}
		else
		{	tokenClass topTerm = theStack.getTopTerminal();
			if(precedence(topTerm, theToken) == "LT" || precedence(topTerm, theToken) == "EQ")
			{
				if(theToken.getType() == "IDENT_T")
				{
					TableList *id = theScanner.tableLookup(theToken.getLexeme());
					if(id == NULL)
						undecIdent(theToken.getLexeme());
					else
						theStack.push(theToken, id);
				}
				else
					theStack.push(theToken, NULL);
				theToken = theScanner.getNextToken();
			}
			else if(precedence(topTerm, theToken) == "GT")
			{
				while(arrayItems == 0 || (terminalPopped() == false) || (theStack.topIsTerm() == false) || (precedence(theStack.getTopTerminal(), lastTermPopped) != "LT"))
				{	
					ptrArray[arrayItems] = theStack.topPtr();
					theArray[arrayItems] = theStack.pop();
					if(theArray[arrayItems].getType() != "E_T")
						lastTermPopped = theArray[arrayItems];
					arrayItems++;
				}
				if(isValidRHS(theArray, ptrArray))
				{
					//took the push statement into the function
				}
				else //not a valid right hand side
				{
					cout << "Syntax Error: Invalid Expression" << endl;
					theScanner.printCurrentLine();
					cin.get();
					exit(1);
				}	
			}
			else //precedence() returned error
			{
				cout << "Syntax Error: Invalid Expression" << endl;
				theScanner.printCurrentLine();
				cin.get();
				exit(2);
			}
		}
	}
}

bool parserClass::isValidRHS(tokenClass array1[], TableList *array2[])
{
	arrayItems--;
	if(array1[arrayItems].getType() == "IDENT_T")
	{
		tokenClass e;
		e.setType("E_T");
		theStack.push(e, array2[arrayItems]);
		if(theScanner.expDebugging())
			cout << "Did E -> identifier" << endl;
		arrayItems = 0;
		return true;
	}
	else if(array1[arrayItems].getType() == "INTEGER_T")
	{
		tokenClass e;
		e.setType("E_T");
		string newTemp = getNextTemp();
		theScanner.addTableEntry(newTemp, "CONST", "INT_T");
		TableList *temp = theScanner.tableLookup(newTemp);
		theStack.push(e, temp);
		printInstruction("addw", NULL, "#4", NULL, "SP");
		printInstruction("movw", NULL, "#" + array1[arrayItems].getLexeme(), temp, "");
		if(theScanner.expDebugging())
			cout << "Did E -> integer-constant" << endl;
		arrayItems = 0;
		return true;
	}
	else if(array1[arrayItems].getType() == "TRUE_T")
	{
		tokenClass e;
		e.setType("E_T");
		string newTemp = getNextTemp();
		theScanner.addTableEntry(newTemp, "CONST", "BOOLEAN_T");
		TableList *temp = theScanner.tableLookup(newTemp);
		theStack.push(e, temp);
		printInstruction("addw", NULL, "#4", NULL, "SP");
		printInstruction("movw", NULL, "#1", temp, "");
		if(theScanner.expDebugging())
			cout << "Did E -> true" << endl;
		arrayItems = 0;
		return true;
	}
	else if(array1[arrayItems].getType() == "FALSE_T")
	{
		tokenClass e;
		e.setType("E_T");
		string newTemp = getNextTemp();
		theScanner.addTableEntry(newTemp, "CONST", "BOOLEAN_T");
		TableList *temp = theScanner.tableLookup(newTemp);
		theStack.push(e, temp);
		printInstruction("addw", NULL, "#4", NULL, "SP");
		printInstruction("movw", NULL, "#0", temp, "");
		if(theScanner.expDebugging())
			cout << "Did E -> false" << endl;
		arrayItems = 0;
		return true;
	}
	else if(array1[arrayItems].getType() == "LEFTPAREN_T")
	{
		if(array1[arrayItems-1].getType() == "E_T"){
			if(array1[arrayItems-2].getType() == "RIGHTPAREN_T")
			{
				theStack.push(array1[arrayItems-1], array2[arrayItems-1]);
				if(theScanner.expDebugging())
					cout << "Did E -> (E)" << endl;
				arrayItems = 0;
				return true;
			}
			else
				return false;	
		}
		else
			return false;
	}
	else if(array1[arrayItems].getType() == "E_T")
	{
		if(array1[arrayItems-2].getType() == "E_T")
		{
			if(array1[arrayItems-1].getSubtype() == "PLUS_ST")
			{
				if(array2[arrayItems]->type != "INT_T" || array2[arrayItems-2]->type != "INT_T")
					wrongType("BOOLEAN or FUCNTION_ID", "INT");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", thisTemp, "");
				printInstruction("addw", array2[arrayItems-2], "", thisTemp, "");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E + E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "MULT_ST")
			{
				if(array2[arrayItems]->type != "INT_T" || array2[arrayItems-2]->type != "INT_T")
					wrongType("BOOLEAN or FUCNTION_ID", "INT");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", thisTemp, "");
				printInstruction("mulw", array2[arrayItems-2], "", thisTemp, "");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E * E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "MINUS_ST")
			{
				if(array2[arrayItems]->type != "INT_T" || array2[arrayItems-2]->type != "INT_T")
					wrongType("BOOLEAN or FUCNTION_ID", "INT");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", thisTemp, "");
				printInstruction("subw", array2[arrayItems-2], "", thisTemp, "");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E - E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "DIV_ST")
			{
				if(array2[arrayItems]->type != "INT_T" || array2[arrayItems-2]->type != "INT_T")
					wrongType("BOOLEAN or FUCNTION_ID", "INT");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", thisTemp, "");
				printInstruction("divw", array2[arrayItems-2], "", thisTemp, "");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E div E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "MOD_ST")
			{
				if(array2[arrayItems]->type != "INT_T" || array2[arrayItems-2]->type != "INT_T")
					wrongType("BOOLEAN or FUCNTION_ID", "INT");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *temp1 = theScanner.tableLookup(newTemp);
				newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *temp2 = theScanner.tableLookup(newTemp);
				newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "INT_T");
				TableList *temp3 = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#12", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", temp1, "");
				printInstruction("divw", array2[arrayItems-2], "", temp1, "");
				printInstruction("movw", temp1, "", temp2, "");
				printInstruction("mulw", array2[arrayItems-2], "", temp2, "");
				printInstruction("movw", array2[arrayItems], "", temp3, "");
				printInstruction("subw", temp2, "", temp3, "");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , temp3);
				if(theScanner.expDebugging())
					cout << "Did E -> E mod E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "OR_ST")
			{
				if(array2[arrayItems]->type != "BOOLEAN_T" || array2[arrayItems-2]->type != "BOOLEAN_T")
					wrongType("INT or FUCNTION_ID", "BOOLEAN");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", thisTemp, "");
				printInstruction("addw", array2[arrayItems-2], "", thisTemp, "");
				printInstruction("cmpw", thisTemp, "", NULL, "#0");
				newTemp = getNextTemp();
				printInstruction("beq", NULL, newTemp, NULL, "");
				printInstruction("movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E or E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "AND_ST")
			{
				if(array2[arrayItems]->type != "BOOLEAN_T" || array2[arrayItems-2]->type != "BOOLEAN_T")
					wrongType("INT or FUCNTION_ID", "BOOLEAN");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("movw", array2[arrayItems], "", thisTemp, "");
				printInstruction("mulw", array2[arrayItems-2], "", thisTemp, "");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E and E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "EQUEQU_ST")
			{
				if(array2[arrayItems]->type != array2[arrayItems-2]->type)
					wrongType("different from the other side of the expression", "same on both sides");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("cmpw", array2[arrayItems], "", array2[arrayItems-2], "");
				string trueTemp = getNextTemp();
				printInstruction("beq", NULL, trueTemp, NULL, "");
				printInstruction("movw", NULL, "#0", thisTemp, "");
				newTemp = getNextTemp();
				printInstruction("jmp", NULL, newTemp, NULL, "");
				printInstruction(trueTemp + " movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E == E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "LSTH_ST")
			{
				if(array2[arrayItems]->type != array2[arrayItems-2]->type)
					wrongType("different from the other side of the expression", "same on both sides");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("cmpw", array2[arrayItems], "", array2[arrayItems-2], "");
				string trueTemp = getNextTemp();
				printInstruction("blss", NULL, trueTemp, NULL, "");
				printInstruction("movw", NULL, "#0", thisTemp, "");
				newTemp = getNextTemp();
				printInstruction("jmp", NULL, newTemp, NULL, "");
				printInstruction(trueTemp + " movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E < E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "LSEQ_ST")
			{
				if(array2[arrayItems]->type != array2[arrayItems-2]->type)
					wrongType("different from the other side of the expression", "same on both sides");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("cmpw", array2[arrayItems], "", array2[arrayItems-2], "");
				string trueTemp = getNextTemp();
				printInstruction("bleq", NULL, trueTemp, NULL, "");
				printInstruction("movw", NULL, "#0", thisTemp, "");
				newTemp = getNextTemp();
				printInstruction("jmp", NULL, newTemp, NULL, "");
				printInstruction(trueTemp + " movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E <= E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "GRTH_ST")
			{
				if(array2[arrayItems]->type != array2[arrayItems-2]->type)
					wrongType("different from the other side of the expression", "same on both sides");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("cmpw", array2[arrayItems], "", array2[arrayItems-2], "");
				string trueTemp = getNextTemp();
				printInstruction("bgtr", NULL, trueTemp, NULL, "");
				printInstruction("movw", NULL, "#0", thisTemp, "");
				newTemp = getNextTemp();
				printInstruction("jmp", NULL, newTemp, NULL, "");
				printInstruction(trueTemp + " movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E > E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "GREQ_ST")
			{
				if(array2[arrayItems]->type != array2[arrayItems-2]->type)
					wrongType("different from the other side of the expression", "same on both sides");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("cmpw", array2[arrayItems], "", array2[arrayItems-2], "");
				string trueTemp = getNextTemp();
				printInstruction("bgeq", NULL, trueTemp, NULL, "");
				printInstruction("movw", NULL, "#0", thisTemp, "");
				newTemp = getNextTemp();
				printInstruction("jmp", NULL, newTemp, NULL, "");
				printInstruction(trueTemp + " movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E >= E" << endl;
				arrayItems = 0;
				return true;
			}
			else if(array1[arrayItems-1].getSubtype() == "NTEQ_ST")
			{
				if(array2[arrayItems]->type != array2[arrayItems-2]->type)
					wrongType("different from the other side of the expression", "same on both sides");
				string newTemp = getNextTemp();
				theScanner.addTableEntry(newTemp, "VAR-ID", "BOOLEAN_T");
				TableList *thisTemp = theScanner.tableLookup(newTemp);
				printInstruction("addw", NULL, "#4", NULL, "SP");
				printInstruction("cmpw", array2[arrayItems], "", array2[arrayItems-2], "");
				string trueTemp = getNextTemp();
				printInstruction("bneq", NULL, trueTemp, NULL, "");
				printInstruction("movw", NULL, "#0", thisTemp, "");
				newTemp = getNextTemp();
				printInstruction("jmp", NULL, newTemp, NULL, "");
				printInstruction(trueTemp + " movw", NULL, "#1", thisTemp, "");
				printInstruction(newTemp + " movw", NULL, "R0", NULL, "R0");
				tokenClass e;
				e.setType("E_T");
				theStack.push(e , thisTemp);
				if(theScanner.expDebugging())
					cout << "Did E -> E <> E" << endl;
				arrayItems = 0;
				return true;
			}
			else
				return false;	
		}
		else
			return false;
	}
	else
		return false;
}

bool parserClass::terminalPopped()
{
	int i = 0;
	while(i < arrayItems)
	{
		if(theArray[i].getType() != "E_T")
			return true; 
		i++;
	}
	return false;
}

string parserClass::precedence(tokenClass rowtok, tokenClass columntok)
{
	static string table[11][11] = {{"GT",	"LT",	"LT",	"LT",	"GT",	"LT",	"LT",	"LT",	"LT",	"GT",	"ER"},
							       {"GT",	"GT",	"LT",	"LT",	"GT",	"LT",	"LT",	"LT",	"LT",	"GT",	"ER"},
								   {"GT",	"GT",	"GT",	"LT",	"GT",	"LT",	"LT",	"LT",	"LT",	"GT",	"ER"},
								   {"LT",	"LT",	"LT",	"LT",	"EQ",	"LT",	"LT",	"LT",	"LT",	"ER",	"ER"},
								   {"GT",	"GT",	"GT",	"ER",	"GT",	"ER",	"ER",	"ER",	"ER",	"GT",	"ER"},
								   {"GT",	"GT",	"GT",	"ER",	"GT",	"ER",	"ER",	"ER",	"ER",	"GT",	"ER"},
							       {"GT",	"GT",	"GT",	"ER",	"GT",	"ER",	"ER",	"ER",	"ER",	"GT",	"ER"},
								   {"GT",	"GT",	"GT",	"ER",	"GT",	"ER",	"ER",	"ER",	"ER",	"GT",	"ER"},
								   {"GT",	"GT",	"GT",	"ER",	"GT",	"ER",	"ER",	"ER",	"ER",	"GT",	"ER"},
								   {"LT",	"LT",	"LT",	"LT",	"ER",	"LT",	"LT",	"LT",	"LT",	"ER",	"ER"},
								   {"ER",	"ER",	"ER",	"ER",	"ER",	"ER",	"ER",	"ER",	"ER",	"ER",	"ER"}};

	int row, col;
	if(rowtok.getType() == "RELOP_T")
		row = 0;
	else if(rowtok.getType() == "ADDOP_T")
		row = 1;
	else if(rowtok.getType() == "MULOP_T")
		row = 2;
	else if(rowtok.getType() == "LEFTPAREN_T")
		row = 3;
	else if(rowtok.getType() == "RIGHTPAREN_T")
		row = 4;
	else if(rowtok.getType() == "IDENT_T")
		row = 5;
	else if(rowtok.getType() == "INTEGER_T")
		row = 6;
	else if(rowtok.getType() == "TRUE_T")
		row = 7;
	else if(rowtok.getType() == "FALSE_T")
		row = 8;
	else if(rowtok.getType() == "SEMICOL_T")
		row = 9;
	else
		row = 10;

	if(columntok.getType() == "RELOP_T")
		col = 0;
	else if(columntok.getType() == "ADDOP_T")
		col = 1;
	else if(columntok.getType() == "MULOP_T")
		col = 2;
	else if(columntok.getType() == "LEFTPAREN_T")
		col = 3;
	else if(columntok.getType() == "RIGHTPAREN_T")
		col = 4;
	else if(columntok.getType() == "IDENT_T")
		col = 5;
	else if(columntok.getType() == "INTEGER_T")
		col = 6;
	else if(columntok.getType() == "TRUE_T")
		col = 7;
	else if(columntok.getType() == "FALSE_T")
		col = 8;
	else if(columntok.getType() == "SEMICOL_T" || columntok.getType() == "DO_T" || columntok.getType() == "THEN_T" || columntok.getType() == "RIGHTPAREN_T" || columntok.getType() == "COMMA_T" || columntok.getType() == "ELSE_T" || columntok.getType() == "END_T" || columntok.getType() == "STRINS_T" )
		col = 9;
	else
		col = 10;

	return table[row][col];
}

void parserClass::varDecs()
{
	if(theToken.getType() == "INT_T" || theToken.getType() == "BOOLEAN_T"){
		string temp = theToken.getType();
		theToken = theScanner.getNextToken();
		identList(temp);
		if(theToken.getType() == "SEMICOL_T") //Don't need else here since it will verify the ; in identTail()
		{
			theToken = theScanner.getNextToken();
			varDecs();
		}
	}
	else if(theToken.getType() != "FUNCTION_T" && theToken.getType() != "BEGIN_T"){
		quitParse("Type(INT or BOOLEAN), FUNCTION, or BEGIN", 0);
	}
	//else let go to epsilon
	else{
		int change = globals * 4;
		stringstream enter;
		enter << "#" << change;
		printInstruction("addw", NULL, enter.str(), NULL, "SP"); //ask Dave about this.
		if(theScanner.getCurrentNest() == 0)
			printInstruction("jmp", NULL, getNextTemp(), NULL, "");
		else
		{}	//not doing this yet, should be fine for case 3
	}
}
void parserClass::varDecs(string bodyLabel)
{
	if(theToken.getType() == "INT_T" || theToken.getType() == "BOOLEAN_T"){
		string temp = theToken.getType();
		theToken = theScanner.getNextToken();
		identList(temp);
		if(theToken.getType() == "SEMICOL_T") //Don't need else here since it will verify the ; in identTail()
		{
			theToken = theScanner.getNextToken();
			varDecs(bodyLabel);
		}
	}
	else if(theToken.getType() != "FUNCTION_T" && theToken.getType() != "BEGIN_T"){
		quitParse("Type(INT or BOOLEAN), FUNCTION, or BEGIN", 0);
	}
	//else let go to epsilon
	else{
		int change = globals * 4;
		stringstream enter;
		enter << "#" << change;
		printInstruction("addw", NULL, enter.str(), NULL, "SP"); //ask Dave about this.
		if(theScanner.getCurrentNest() == 0)
			printInstruction("jmp", NULL, getNextTemp(), NULL, "");
		else
		{
			printInstruction("jmp", NULL, bodyLabel, NULL, "");
		}	//not doing this yet, should be fine for case 3
	}
}

void parserClass::funcDecs()
{
	if(theToken.getType() == "FUNCTION_T"){
		theToken = theScanner.getNextToken();
		if(theToken.getType() == "IDENT_T"){
			theScanner.addTableEntry(theToken.getLexeme(), "FUNC-ID", "FUNC-ID");
			string label = getNextTemp();
			string bodyLabel = getNextTemp();
			theScanner.addFunctionLabel(label);
			theScanner.addBodyLabel(bodyLabel);
			outfile << ":this begins the function " << theToken.getLexeme() << endl;
			printInstruction(label + " movw", NULL, "R0", NULL, "R0");
			theScanner.addTableLevel(theToken.getLexeme());
			theToken = theScanner.getNextToken();
			funcDecTail();
			if(theToken.getType() == "SEMICOL_T"){
				theScanner.assignParameters();
				theToken = theScanner.getNextToken();
				if (currParams != 0){
					stringstream adjustment;
					adjustment << "#" << ((currParams * 4)-4);
					printInstruction("subw", NULL, adjustment.str(), NULL, "AP");
					for(int i = 1; i <= currParams; i++){
						printInstruction("movw", NULL, "@AP", NULL, "@SP");
						printInstruction("addw", NULL, "#4", NULL, "SP");
						if(i != currParams)
							printInstruction("addw", NULL, "#4", NULL, "AP");
					}
				}
				parseBlock(bodyLabel);
				printInstruction("movw", NULL, "FP", NULL, "SP");
				outfile << "RET" << endl;
				if(theToken.getType() == "SEMICOL_T"){
					theToken = theScanner.getNextToken();
					theScanner.deleteTableLevel();
					funcDecs();
				}
				else
					quitParse(";", 2);
			}
			else //shouldn't ever trigger this
				quitParse(";", 1);
		}
		else
			quitParse("Valid Identifier for function name", 0);
	}
	else if(theToken.getType() != "BEGIN_T"){
		quitParse("FUNCTION or BEGIN",0);
	}
	//else let go to epsilon
}
void parserClass::funcDecTail()
{
	currParams = 0;
	if(theToken.getType() == "LEFTPAREN_T"){
		currParams++;
		theToken = theScanner.getNextToken();
		paramList();
		if(theToken.getType() == "RIGHTPAREN_T"){
			theToken = theScanner.getNextToken();
		}
		else //Shouldn't ever trigger this
			quitParse(")", 1);
	}
	else if(theToken.getType() != "SEMICOL_T"){
		quitParse("; or (", 0);
	}
	//else let go to epsilon
}
void parserClass::paramList()
{
	if(theToken.getType() == "INT_T" || theToken.getType() == "BOOLEAN_T"){
		string temp = theToken.getType();
		theToken = theScanner.getNextToken();
		typeTail(temp);
	}
	else
		quitParse("Type(INT or BOOLEAN)", 0);

}
void parserClass::paramTail()
{
	if(theToken.getType() == "COMMA_T"){
		currParams++;
		theToken = theScanner.getNextToken();
		paramList();
	}
	else if(theToken.getType() != "RIGHTPAREN_T"){
		quitParse(") or ,", 1);
	}
	//else let go to epsilon
}
void parserClass::typeTail(string type)
{
	if(theToken.getType() == "AMPERS_T"){
		theToken = theScanner.getNextToken();
		if(theToken.getType() == "IDENT_T"){
			string temp = theToken.getLexeme();
			theScanner.addTableEntry(temp, "REF-PARAM", type);
			theToken = theScanner.getNextToken();
			paramTail();
		}
		else
			quitParse("Identifier or &Identifier", 0);
	}
	else if(theToken.getType() == "IDENT_T"){
		string temp = theToken.getLexeme();
		theScanner.addTableEntry(temp, "VAL-PARAM", type);
		theToken = theScanner.getNextToken();
		paramTail();
	}
	else
		quitParse("Identifier or &Identifier", 0);

}
void parserClass::identList(string type)
{
	if(theToken.getType() == "IDENT_T"){
		string temp = theToken.getLexeme();
		globals++;
		theScanner.addTableEntry(temp, "VAR-ID", type);
		theToken = theScanner.getNextToken();
		identTail(type);
	}
	else
		quitParse("a valid Identifier", 0);

}
void parserClass::identTail(string type)
{
	if(theToken.getType() == "COMMA_T"){
		theToken = theScanner.getNextToken();
		identList(type);
	}
	else if(theToken.getType() != "SEMICOL_T"){
		quitParse(", or ;", 0);
	}
	//else let go to epsilon

}
void parserClass::progBody()
{
	if(theToken.getType() == "BEGIN_T"){
		if(theScanner.getCurrentNest() == 0)
			printInstruction("$0 movw", NULL, "R0", NULL, "R0");
		else
		{}	//not doing this yet, should be fine for case 3
		theToken = theScanner.getNextToken();
		followBegin();
	}
	else{
		quitParse("BEGIN as start of program body", 0);
	}
}
void parserClass::followIdent(string theLexeme)
{
	if(theToken.getSubtype() == "EQUALS_ST"){
		theToken = theScanner.getNextToken();
		parseExpression();
		TableList *temp = theScanner.tableLookup(theLexeme);
		if(temp->type != finalType)
			wrongType(finalType, temp->type);
		else
			printInstruction("movw", NULL, "-4@SP", temp, "");
		printInstruction("subw", NULL, "#4", NULL, "SP");
	}
	else if(theToken.getSubtype() == "SWAP_ST"){
		theToken = theScanner.getNextToken();
		if(theToken.getType() == "IDENT_T"){
			TableList *first = theScanner.tableLookup(theLexeme);
			TableList *second = theScanner.tableLookup(theToken.getLexeme());
			if(second == NULL)
						undecIdent(theToken.getLexeme());
			if(first->type != second->type){ //Didn't feel like writing a new error module just for this case
				cout << "ERROR: operands of swap statement are not the same type" << endl;
				theScanner.printCurrentLine();
				cin.get();
				exit(0);
			}
			printInstruction("movw", second, "", NULL, "@SP");
			printInstruction("addw", NULL, "#4", NULL, "SP");
			printInstruction("movw", first, "", second, "");
			printInstruction("movw", NULL, "-4@SP", first, "");
			printInstruction("subw", NULL, "#4", NULL, "SP");
			theToken = theScanner.getNextToken();
		}
		else
			quitParse("Identifier", 1);
	}
	else if(theToken.getType() == "LEFTPAREN_T"){
		parCount = 1;
		TableList *func = theScanner.tableLookup(theLexeme);
		string funcName = func->label;
		TableList *temp = func->params;
		theToken = theScanner.getNextToken();
		if(temp == NULL){
			otherError("Error: Too many parameters entered for this function");
		}
		else if(temp->kind == "VAL-PARAM"){
			parseExpression();
		}
		else{ //is a reference parameter
			if(theToken.getType() == "IDENT_T"){
				TableList *refParam = theScanner.tableLookup(theToken.getLexeme());
				printInstruction("mova", refParam, "", NULL, "@SP");
				printInstruction("addw", NULL, "#4", NULL, "SP");
				finalType = refParam->type;
				theToken = theScanner.getNextToken();
			}
			else
				quitParse("Identifier for reference parameter", 2);
		}
		string specTemp = getNextTemp();
		specTemp = "$s" + specTemp;
		theScanner.addTableEntry(specTemp, "SPEC", "SPEC");
		for(int i = parCount - 1; i < parCount; i++)
		{
			if(temp == NULL){
				otherError("Error: Too many parameters entered for this function");
			}
			else if(temp->type != finalType){
				wrongType(finalType, temp->type);
			}
			temp=temp->next;
		}
		followExpression(temp);
		if(theToken.getType() == "RIGHTPAREN_T"){
			int entered = parCount;
			if(theScanner.countParameters(theLexeme, parCount) == false){ 
				wrongParams(theLexeme, (entered - parCount), entered);
			}
			stringstream enter;
			enter << "#" << entered;
			printInstruction("CALL", NULL, enter.str(), NULL, funcName);
			theScanner.deleteSpecialTemps();
			theToken = theScanner.getNextToken();
		}
		else{ //probably not necessary since followExpression should throw a fit before this even gets checked
			quitParse(")", 1); 
		}
	}
	else if(theToken.getType() != "ELSE_T" && theToken.getType() != "SEMICOL_T" && theToken.getType() != "END_T"){
		quitParse("=, ><, (, ELSE, semicolon, or END", 0);
	}
	//else let go to epsilon
	else{
		parCount = 0;
		int entered = parCount;
		if(theScanner.countParameters(theLexeme, parCount) == false){
			wrongParams(theLexeme, (entered - parCount), entered);
		}
		TableList *func = theScanner.tableLookup(theLexeme);
		string funcName = func->label;
		printInstruction("CALL", NULL, "#0", NULL, funcName);
	}	
}
void parserClass::followIf(string after, string otherwise)
{
	if(theToken.getType() == "ELSE_T"){
		printInstruction("jmp", NULL, after, NULL, "");
		printInstruction(otherwise + " movw", NULL, "R0", NULL, "R0");
		theToken = theScanner.getNextToken();
		parseStatement();
		printInstruction(after + " movw", NULL, "R0", NULL, "R0");
	}
	else if(theToken.getType() != "SEMICOL_T" && theToken.getType() != "END_T")
	{
		quitParse("ELSE, END, or semicolon", 0);
	}
	//else let go to epsilon
	else
		printInstruction(otherwise + " movw", NULL, "R0", NULL, "R0");
}
void parserClass::followCOUT()
{
	if(theToken.getType() == "STRINS_T")
	{
		theToken = theScanner.getNextToken();
		if(theToken.getType() == "ENDL_T"){
			theToken = theScanner.getNextToken();
			outfile << "outb #10" << endl;
		}
		else if(theToken.getType() == "STRING_T"){
			string temp = theToken.getLexeme();
			theToken = theScanner.getNextToken();
			for(int i = 1; i < temp.length() - 1; i++)
			{
				if(temp[i] == ' ')
					outfile << "outb #32" << endl;
				else
					outfile << "outb ^" << temp[i] << endl;
			}

		}
		else if(theToken.getType() != "LEFTPAREN_T" && theToken.getType() != "IDENT_T" && theToken.getType() != "INTEGER_T" && theToken.getType() != "TRUE_T" && theToken.getType() != "FALSE_T"){
			quitParse("ENDL, string, or Expression", 10); //as far as I can tell, this will pick out things like cout << do
		}
		else{
			parseExpression();
			printInstruction("outw", NULL, "-4@SP", NULL, "");
			printInstruction("subw", NULL, "#4", NULL, "SP");
		}
		if(theToken.getType() == "STRINS_T")
			followCOUT();
	}
	else{
		quitParse("<<", 0);
	}
}

void parserClass::followCIN()
{
	if(theToken.getType() == "STREXT_T"){
		theToken = theScanner.getNextToken();
		if(theToken.getType() == "IDENT_T"){
			TableList *temp = theScanner.tableLookup(theToken.getLexeme());
			if(temp == NULL){
				undecIdent(theToken.getLexeme());
			}
			else if(temp->type == "BOOLEAN_T"){
				wrongType("BOOLEAN", "INT");
			}
			else{
				printInstruction("inw", temp, "", NULL, "");
			}
			theToken = theScanner.getNextToken();
		}
		else{
			quitParse("identifier", 0);	
		}
	}
	else if(theToken.getType() != "ELSE_T" && theToken.getType() != "SEMICOL_T" && theToken.getType() != "END_T"){
		quitParse(">>, semicolon, ELSE, or END", 0);
	}
	//else let go to epsilon
	else
		outfile << "inb @sp" << endl;
}
void parserClass::followBegin() //still needs <statement-tail>
{
	if(theToken.getType() == "END_T"){
		theToken = theScanner.getNextToken();
	}
	else if(theToken.getType() != "WHILE_T" && theToken.getType() != "IF_T" && theToken.getType() != "IDENT_T" && theToken.getType() != "COUT_T" && theToken.getType() != "CIN_T" && theToken.getType() != "BEGIN_T"){
		quitParse("END or Statement", 0);  //guarding?
	}
	else{
		parseStatement();
		statementTail();
		if(theToken.getType() == "END_T"){
			theToken = theScanner.getNextToken();
		}	
	}
}
void parserClass::statementTail()
{
	if(theToken.getType() == "SEMICOL_T"){
		theToken = theScanner.getNextToken();
		parseStatement();
		statementTail();
	}
	else if (theToken.getType() != "END_T"){
		quitParse("END or semicolon", 0);
	}
	//else let go to epsilon
}
void parserClass::followExpression(TableList* tempPointer)
{
	if(theToken.getType() == "COMMA_T"){
		parCount++;
		theToken = theScanner.getNextToken();
		if(tempPointer == NULL){
			otherError("Error: Too many parameters entered for this function");
		}
		else if(tempPointer->kind == "VAL-PARAM"){
			parseExpression();
		}
		else{ //is a reference parameter
			if(theToken.getType() == "IDENT_T"){
				TableList *refParam = theScanner.tableLookup(theToken.getLexeme());
				printInstruction("mova", refParam, "", NULL, "@SP");
				printInstruction("addw", NULL, "#4", NULL, "SP");
				finalType = refParam->type;
				theToken = theScanner.getNextToken();
			}
			else
				quitParse("Identifier for reference parameter", 2);
		}
		string specTemp = getNextTemp();
		specTemp = "$s" + specTemp;
		theScanner.addTableEntry(specTemp, "SPEC", "SPEC");
		for(int i = parCount - 1; i < parCount; i++)
		{
			if(tempPointer == NULL){
				otherError("Error: Too many parameters entered for this function");
			}
			else if(tempPointer->type != finalType){
				wrongType(finalType, tempPointer->type);
			}
			tempPointer=tempPointer->next;
		}
		followExpression(tempPointer);
	}
	else if(theToken.getType() != "RIGHTPAREN_T"){
		quitParse(") or ,", 0);
	}
	//else let go to epsilon
}
int parserClass::numLinesProcessed()
{
	return theScanner.numLinesProcessed();
}

void parserClass::quitParse(string expected, int x)
{
	if(theToken.getType() == "EOF_T")
		cout << "Error: Found End of File when expecting " << expected << endl;
	else
		cout << "Error: Found " << theToken.getLexeme() << " when expecting " << expected << endl;
	theScanner.printCurrentLine();
	cin.get();
	exit(x);
}

void parserClass::undecIdent(string theLexeme)
{
	cout << "Error (Undeclared Identifier): " << theLexeme << endl;
	theScanner.printCurrentLine();
	cin.get();
	exit(49);
}

void parserClass::wrongParams(string theLexeme, int needed, int received)
{
	cout << "Error: Function " << theLexeme << " takes " << needed << " parameters but was passed " << received << endl;
	theScanner.printCurrentLine();
	cin.get();
	exit(51);
}

void parserClass::wrongType(string got, string expected)
{
	cout << "Error: statement/expression was given identifier of type " << got << " when it needed type " << expected << endl;
	theScanner.printCurrentLine();
	cin.get();
	exit(0);
}

void parserClass::printInstruction(string command, TableList* ptr1, string str1, TableList* ptr2, string str2)
{
	outfile << command << " ";
	if(ptr1 == NULL)
		outfile << str1 << " ";
	else if(ptr1->nesting == 0)
		outfile << "+" << ptr1->offset * 4 << "@R0 ";
	else if(ptr1->kind == "REF-PARAM")
		outfile << "+" << ptr1->offset * 4 << "@@FP ";
	else
		outfile << "+" << ptr1->offset * 4 << "@FP ";
	if(ptr2 != NULL){
		if(ptr2->nesting == 0)
			outfile << "+" << ptr2->offset * 4 << "@R0" << endl;
		else if(ptr2->kind == "REF-PARAM")
			outfile << "+" << ptr2->offset * 4 << "@@FP " << endl;
		else
			outfile << "+" << ptr2->offset * 4 << "@FP" << endl;
	}
	else
		outfile << str2 << endl;
}

void parserClass::otherError(string message)
{
	cout << message << endl;
	theScanner.printCurrentLine();
	cin.get();
	exit(0);
}

string parserClass::getNextTemp()
{
	static int nextCt = 0;
	char tempName[10];

	sprintf(tempName, "$%d", nextCt++);
	return tempName;
}

