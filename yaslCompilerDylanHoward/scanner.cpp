//Dylan Howard, Revised September 2011
//This file contains method implementations for the
//scannerClass

//The class header is found in scanner.h

#include "stdafx.h"  // Required for visual studio

#include "scanner.h"
#include <string>
#include <ctype.h>
#include <iostream>

using namespace std;

tokenClass::tokenClass()
{	type = "";
	subtype = "NONE";
	lexeme = "";
	lexError = false;
}

void tokenClass::display()
{	
	if(lexError == false)
	{	cout << lexeme << '\t';
		cout << type << '\t';
		cout << subtype << '\t';
		cout << endl;	}
}

string tokenClass::getLexeme()
{
	return lexeme;
}

string tokenClass::getType()
{
	return type;
}

string tokenClass::getSubtype()
{
	return subtype;
}

void tokenClass::setType(string newType)
{
	type = newType;
}

void tokenClass::setSubtype(string newSubtype)
{
	subtype = newSubtype;
}

void tokenClass::setLexeme(string newLexeme)
{
	lexeme = newLexeme;
}

void tokenClass::addToLexeme(char ch)
{
	lexeme = lexeme + ch;
}

void tokenClass::clearToken()
{
	type = "";
	subtype = "NONE";
	lexeme = "";
}

bool tokenClass::isEOF()
{
	if (type == "EOF_T")
		return true;
	return false;
}

void tokenClass::pushBackLexeme()
{	
	lexeme.erase(lexeme.length()-1, 1);
}

bool tokenClass::isLexError()
{
	return lexError;
}

void tokenClass::foundLexError() //call when a  lexical error is found
{
	lexError = true;
}

scannerClass::scannerClass()
{
	lexemeLength = 0;
	state = 0;
	expDeb = false;
	// possibly do other stuff as needed
}

void scannerClass::addTableLevel(string theScope)
{
	theTable.addLevel(theScope);
}

void scannerClass::deleteTableLevel()
{
	theTable.deleteLevel();
}

void scannerClass::deleteTemps()
{
	theTable.deleteTemps();
}

void scannerClass::deleteSpecialTemps()
{
	theTable.deleteSpecialTemps();
}

void scannerClass::addTableEntry(string theLexeme, string theKind, string theType)
{
	if(theTable.addEntry(theLexeme, theKind, theType))
		return;
	else
	{
		printCurrentLine();
		cin.get();
		exit(20);
	}
}

void scannerClass::assignParameters()
{
	theTable.assignParameters();
}

bool scannerClass::countParameters(string lexeme, int &num)
{
	return theTable.countParameters(lexeme, num);
}

TableList* scannerClass::tableLookup(string theLexeme)
{
	return theTable.tableLookup(theLexeme);
}
void scannerClass::tableDump()
{
	theTable.tableDump();
}

void scannerClass::addFunctionLabel(string theTemp)
{
	theTable.addFunctionLabel(theTemp);
}

void scannerClass::addBodyLabel(string theTemp)
{
	theTable.addBodyLabel(theTemp);
}

int scannerClass::getCurrentNest()
{
	return theTable.getCurrentNest();
}

tokenClass scannerClass::getNextToken()
{
	lexemeLength = 0;
	state = 0;
	theToken.clearToken();
	while(state < 90)
	{
		char ch = theFile.getNextChar();
		theToken.addToLexeme(ch);
		lexemeLength++;
		state = getNextState(ch);
		if(state == 0)
		{	theToken.clearToken();
			lexemeLength = 0;	}
		if (state == 1)
		{	if(lexemeLength > 12)
			{	
				theToken.clearToken();
				lexemeLength = 0;
				theToken.foundLexError();
				cout << "Error: Identifier longer than 12 characters" << endl;
				theFile.printCurrentLine();
				state = 90;
				closeSourceProgram();
				cin.get();
				exit(0);	
			}
		}
		else if (state == 2)
		{	if(lexemeLength > 4)
			{	
				theToken.clearToken();
				lexemeLength = 0;
				theToken.foundLexError();
				cout << "Error: Integer longer than 4 digits" << endl;
				theFile.printCurrentLine();
				state = 90;
				closeSourceProgram();
				cin.get();
				exit(0);
			}
		}
		else if (state == 3)
		{	if(lexemeLength > 51)
			{	
				theToken.clearToken();
				lexemeLength = 0;
				theToken.foundLexError();
				cout << "Error: String longer than 50 characters" << endl;
				theFile.printCurrentLine();
				state = 90;
				closeSourceProgram();
				cin.get();
				exit(0);	
			}
		}
		else if (state == 11)
		{	theToken.clearToken();
			lexemeLength = 0;	}
		else if (state == 13)
		{	theToken.clearToken();
			lexemeLength = 0;	}
		else if(state == 90) //Extra state to route lexical Errors into
		{	/*Project3 update - now redundant with exit commands in errors, still reasonable for future debugging*/	}
		else if (state == 93) //not actually an error, if EOF read in //-style comment, push it back and end comment
		{	theToken.pushBackLexeme();
			theFile.pushBack();
			lexemeLength = 0;
			state = 0;	}
		else if (state == 94)
		{	theToken.clearToken();
			lexemeLength = 0;
			theToken.foundLexError();
			cout << "Error: Single / not followed by another /" << endl;
			theFile.printCurrentLine();
			closeSourceProgram();
			cin.get();
			exit(0);	}
		else if (state == 95)
		{	theToken.clearToken();
			lexemeLength = 0;
			theToken.foundLexError();
			cout << "Error: EOF inside of string" << endl;
			theFile.printCurrentLine();
			closeSourceProgram();
			cin.get();
			exit(0);	}
		else if (state == 96)
		{	theToken.clearToken();
			lexemeLength = 0;
			theToken.foundLexError();
			cout << "Error: Carriage return inside of string" << endl;
			theFile.printCurrentLine();
			closeSourceProgram();
			cin.get();
			exit(0);	}
		else if (state == 97) //state for receiving a compiler directive
		{	processDirective();
			theToken.clearToken();
			lexemeLength = 0;
			state = 0;	}
		else if (state == 98)
		{	theToken.clearToken();
			lexemeLength = 0;
			theToken.foundLexError();
			cout << "Error: EOF found in comment" << endl;
			theFile.printCurrentLine();
			closeSourceProgram();
			cin.get();
			exit(0);	}
		else if (state == 99)
		{	string badChar = theToken.getLexeme();
			theToken.clearToken();
			lexemeLength = 0;
			theToken.foundLexError();
			cout << "Error: Invalid start of expression: " << badChar << endl;
			theFile.printCurrentLine();
			closeSourceProgram();
			cin.get();
			exit(0);	}
		/*else if (state >= 93 && state < 100) //generic error message
		{	theToken.clearToken();
			lexemeLength = 0;
			theToken.foundLexError();
			cout << "A lexical error has been identified.  Ceasing compilation." << endl;
			theFile.printCurrentLine();
			closeSourceProgram();	} */
		else if (state == 100)
		{	theToken.setType("EOF_T");	}
		else if (state == 101)
		{	theToken.pushBackLexeme();
			string ident = theToken.getLexeme();
			if(_stricmp(ident.c_str(), "program") == 0)
				theToken.setType("PROGRAM_T");
			else if(_stricmp(ident.c_str(), "function") == 0)
				theToken.setType("FUNCTION_T");
			else if(_stricmp(ident.c_str(), "begin") == 0)
				theToken.setType("BEGIN_T");
			else if(_stricmp(ident.c_str(), "end") == 0)
				theToken.setType("END_T");
			else if(_stricmp(ident.c_str(), "if") == 0)
				theToken.setType("IF_T");
			else if(_stricmp(ident.c_str(), "then") == 0)
				theToken.setType("THEN_T");
			else if(_stricmp(ident.c_str(), "else") == 0)
				theToken.setType("ELSE_T");
			else if(_stricmp(ident.c_str(), "while") == 0)
				theToken.setType("WHILE_T");
			else if(_stricmp(ident.c_str(), "do") == 0)
				theToken.setType("DO_T");
			else if(_stricmp(ident.c_str(), "cout") == 0)
				theToken.setType("COUT_T");
			else if(_stricmp(ident.c_str(), "cin") == 0)
				theToken.setType("CIN_T");
			else if(_stricmp(ident.c_str(), "endl") == 0)
				theToken.setType("ENDL_T");
			else if(_stricmp(ident.c_str(), "or") == 0)
			{	theToken.setType("ADDOP_T");
				theToken.setSubtype("OR_ST");	}
			else if(_stricmp(ident.c_str(), "and") == 0)
			{	theToken.setType("MULOP_T");
				theToken.setSubtype("AND_ST");	}
			else if(_stricmp(ident.c_str(), "div") == 0)
			{	theToken.setType("MULOP_T");
				theToken.setSubtype("DIV_ST");	}
			else if(_stricmp(ident.c_str(), "mod") == 0)
			{	theToken.setType("MULOP_T");
				theToken.setSubtype("MOD_ST");	}
			else if(_stricmp(ident.c_str(), "int") == 0)
				theToken.setType("INT_T");
			else if(_stricmp(ident.c_str(), "boolean") == 0)
				theToken.setType("BOOLEAN_T");
			else if(_stricmp(ident.c_str(), "true") == 0)
				theToken.setType("TRUE_T");
			else if(_stricmp(ident.c_str(), "false") == 0)
				theToken.setType("FALSE_T");
			//checks for YASL keywords
			else
				theToken.setType("IDENT_T");
			theFile.pushBack();	}
		else if (state == 102)
		{	theToken.setType("INTEGER_T");
			theToken.pushBackLexeme();
			theFile.pushBack();	}
		else if (state == 103)
		{	theToken.setType("STRING_T");	}
		else if (state == 104)
		{	theToken.setType("PERIOD_T");	}
		else if (state == 105)
		{	theToken.setType("ASSIGN_T");
			theToken.setSubtype("EQUALS_ST");
			theToken.pushBackLexeme();
			theFile.pushBack();	}
		else if (state == 106)
		{	theToken.setType("RELOP_T");
			theToken.setSubtype("EQUEQU_ST");	}
		else if (state == 107)
		{	theToken.setType("RELOP_T");
			theToken.setSubtype("GRTH_ST");
			theToken.pushBackLexeme();
			theFile.pushBack();	}
		else if (state == 108)
		{	theToken.setType("RELOP_T");
			theToken.setSubtype("GREQ_ST");	}
		else if (state == 109)
		{	theToken.setType("ASSIGN_T");
			theToken.setSubtype("SWAP_ST");	}
		else if (state == 110)
		{	theToken.setType("STREXT_T");	}
		else if (state == 111)
		{	theToken.setType("RELOP_T");
			theToken.setSubtype("LSTH_ST");
			theToken.pushBackLexeme();
			theFile.pushBack();	}
		else if (state == 112)
		{	theToken.setType("RELOP_T");
			theToken.setSubtype("NTEQ_ST");	}
		else if (state == 113)
		{	theToken.setType("STRINS_T");	}
		else if (state == 114)
		{	theToken.setType("RELOP_T");
			theToken.setSubtype("LSEQ_ST");	}
		else if (state == 115)
		{	theToken.setType("ADDOP_T");
			theToken.setSubtype("MINUS_ST");	}
		else if (state == 116)
		{	theToken.setType("ADDOP_T");
			theToken.setSubtype("PLUS_ST");	}
		else if (state == 117)
		{	theToken.setType("MULOP_T");
			theToken.setSubtype("MULT_ST");	}
		else if (state == 118)
		{	theToken.setType("AMPERS_T");	}
		else if (state == 119)
		{	theToken.setType("RIGHTPAREN_T");	}
		else if (state == 120)
		{	theToken.setType("SEMICOL_T");	}
		else if (state == 121)
		{	theToken.setType("LEFTPAREN_T");	}
		else if (state == 122)
		{	theToken.setType("COMMA_T");	}
			
			
	}
	state = 0;
	return theToken;
}

void scannerClass::processDirective() //Invoked when a compiler directive is identified
{
	string directive = theToken.getLexeme();
	if(directive[2] == 'p')
	{	
		if(directive[3] == '+')
			theFile.setPrintStatus(true);
		else
			theFile.setPrintStatus(false);
	}
	else if(directive[2] == 'e')
	{
		if(directive[3] == '+')
			expDeb = true;
		else
			expDeb = false;
	}
	else if(directive[2] == 's')
	{
		if(directive[3] == '+') // move this to the else if clause if needed
			tableDump();
	}
	else
	{	cout << "Lexical Warning: Invalid compiler directive" << endl;
		theFile.printCurrentLine();		}
}

void scannerClass::closeSourceProgram()
{
	theFile.closeSourceProgram();
}

int scannerClass::numLinesProcessed()
{
	return theFile.numLinesProcessed();
}

void scannerClass::printCurrentLine()
{
	theFile.printCurrentLine();
}

bool scannerClass::expDebugging()
{
	return expDeb;
}

int scannerClass::getNextState(char ch)
{
	static int table[14][24] = {{120, 1, 2, 99, 0, 0, 7, 99, 6, 116, 115, 4, 5, 104, 122, 118, 99,	121, 119, 3, 12, 117, 100, 99},
								{101, 1, 1,	1,	101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101},
								{102, 102, 2,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
								{3,	3,	3,	3,	96,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	103,3,	3,	95,	3},
								{111,111,111,111,111,111,111,111,114,111,111,113,112,111,111,111,111,111,111,111,111,111,111,111},
								{107,107,107,107,107,107,107,107,108,107,107,109,110,107,107,107,107,107,107,107,107,107,107,107},
								{105,105,105,105,105,105,105,105,106,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105},
								{11,11,	11,	11,	11,	11,	11,	0,	11,	11,	11,	11,	11,	11,	11,	11,	8,	11,	11,	11,	11,	11,	98,	11},
								{11,9,	11,	11,	11,	11,	11,	0,  11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	98,	11},
								{11,11,	11,	11,	11,	11,	11,	0,	11,	10,	10,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	98,	11},
								{11,11,	11,	11,	11,	11,	11,	97,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	98,	11},
								{11,11,	11,	11,	11,	11,	11,	0,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	98,	11},
								{94,94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94,	94, 94,	94,	13,	94,	94,	94},
								{13,13,13,13,0,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	13,	93,	13	}};

	int input;
	if (ch == ';')
		input = 0;
	else if(isalpha(ch))
		input = 1;
	else if(isdigit(ch))
		input = 2;
	else if(ch == '_')
		input = 3;
	else if(ch == '\n')
		input = 4;
	else if(isspace(ch))
		input = 5;
	else if(ch == '{')
		input = 6;
	else if(ch == '}')
		input = 7;
	else if(ch == '=')
		input = 8;
	else if(ch == '+')
		input = 9;
	else if(ch == '-')
		input = 10;
	else if(ch == '<')
		input = 11;
	else if(ch == '>')
		input = 12;
	else if(ch == '.')
		input = 13;
	else if(ch == ',')
		input = 14;
	else if(ch == '&')
		input = 15;
	else if(ch == '$')
		input = 16;
	else if(ch == '(')
		input = 17;
	else if(ch == ')')
		input = 18;
	else if(ch == '\'')
		input = 19;
	else if(ch == '/')
		input = 20;
	else if(ch == '*')
		input = 21;
	else if(ch == EOF)
		input = 22;
	else
		input = 23;
	//uses series of if statements to determine value of input
	state = table[state][input];
	return state;
}


