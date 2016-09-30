// Name: OpenSearch
// Author: mcpancakes
// Version: 0.1
// Date: 12 April 2009
//
// Copyright (C) 2009 mcpancakes
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "filename.h"

bool checkRelativity(char strChar1, char strChar2, unsigned char romChar1, unsigned char romChar2);   // Returns true (1) if the two chars in the string and in the ROM are relatively the same.
void searchComplete(void);            // If the string was found, control passes to this function.
void charComparison(void);            // Prints the hex values of the string and of the portion of the ROM that passed the relativity check.
void makeTable(void);                 // Outputs a table file of the ROM's English alphabet.


bool mktbl = 0;                   // Command line argument to produce a table (.tbl) file.
bool autocaps = 0;                // Command line argument to generate the set of hex values for capital letters.
bool tblOverwrite = 0;            // Command line argument to make certain it's alright to overwrite an existing table file.
int disp = 1;                     // Command line argument to specify which occurrence of the string is to be reported.
char *filePath;                   // The path of the file (argv[1]).
char autoCapsAsciiChar;           // AutoCaps ASCII letter ("-autocaps %c").
char *autoCapsStrPtr;             // AutoCaps pointer to the first wildcard that directly precedes the first letter of the string.
int autoCapsPtrDist;              // AutoCaps distance from beginning of string to wildcard directly before the first letter of the string.
unsigned long romSize;            // Size of the ROM in bytes.
char string[31];                  // String to search for in the ROM.
unsigned char *romInMem;          // Pointer to the ROM once it is read into memory.
unsigned char *beginningOfRom;    // Pointer to the first byte of the ROM in memory.
unsigned char *endOfRom;          // Pointer to the last byte of the ROM in memory.
char *strChar1;                   // The first of two chars from the string used in relative searching.
char *strChar2;                   // The second of two chars from the string used in relative searching.
unsigned char *romChar1;          // The first of two chars from the ROM used in relative searching.
unsigned char *romChar2;          // The second of two chars from the ROM used in relative searching.
long int byteCount;               // A marker of how far we are into the ROM.


int main(int argc, char *argv[])
{
	if (argv[1] != NULL && !strcmp(argv[1], "-help"))
	{
		printf("OpenSearch 0.1 - optional arguments:\n");
		printf("  -mktbl              Create the table file for the English alphabet in the\n");
		printf("                      case that was searched for.\n");
		printf("  -autocaps <letter>  Automatically generate the uppercase alphabet of the ROM\n");
		printf("                      and include it in your table file.\n");
		printf("  -disp <number>      Specify which occurrence of the string you want to\n");
		printf("                      be returned (and subsequently used to make a table file).\n");
		printf("  -f                  Allows for overwriting of an existing table file.\n");
		exit(EXIT_SUCCESS);
	}
	
	if (argc < 3)    // At least three arguments are required.
	{
		printf("Usage: %s <filename> <string>\n", argv[0]);
		printf("For an explanation of optional arguments, type \"%s -help\".\n", argv[0]);
		exit(EXIT_FAILURE);
	
	}
	
	printf("OpenSearch 0.1  Copyright (C) 2009 mcpancakes\n");
	
	while (argc--)    // Parse the optional arguments.
	{
		if (!strcmp(argv[argc], "-mktbl")) mktbl = 1;
		else if (!strcmp(argv[argc], "-autocaps"))
		{
			if (autocaps) printf("Warning: Redundant \"-autocaps\" calling.\n");
			else if (argv[argc + 1] == NULL) printf("Error: No AutoCaps letter specified.\n");
			else if (argv[argc + 1][0] < 'A' || argv[argc + 1][0] > 'Z') printf("Error: AutoCaps letter must be A-Z.\n");
			else
			{
				autoCapsAsciiChar = argv[argc + 1][0];
				autocaps = 1;
			}
		}
		else if (!strcmp(argv[argc], "-disp"))
		{
			if (disp != 1) printf("Warning: Redundant \"-disp\" calling.\n");
			else if (atoi(argv[argc + 1]) <= 0) printf("Error: Invalid \"-disp\" argument.\n");
			else disp = atoi(argv[argc + 1]);
		}
		else if (!strcmp(argv[argc], "-f")) tblOverwrite = 1;
		else if (argc > 2 && strcmp(argv[argc - 1], "-autocaps") && strcmp(argv[argc - 1], "-disp")) printf("Unrecognized argument: %s\n", argv[argc]);
	}
	
	filePath = (char *) malloc(strlen(argv[1]) + 1);
	strcpy(filePath, argv[1]);
	
	strncpy(string, argv[2], 30);
	
	if (strlen(string) < 2)
	{
		printf("Error: At least two characters are required for a relative search.\n");
		exit(EXIT_FAILURE);
	}
	
	if (strlen(string) < 5)
	{
		printf("Warning: For accuracy, the search string should be at least 5 characters.\n");
	}
	
	strChar1 = &string[0];
	strChar2 = &string[1];
	while (strChar1 != &string[strlen(string) - 1])
	{
		if (*strChar1 >= 'A' && *strChar1 <= 'Z' && *strChar2 >= 'a' && *strChar2 <= 'z')    // This tests for Xx.
		{
			printf("Warning: Mixing cases may yield inaccurate results.\n");
			break;
		}
		
		if (*strChar2 >= 'A' && *strChar2 <= 'Z' && *strChar1 >= 'a' && *strChar1 <= 'z')    // This tests for xX (for when the last char of the string is the only uppercase).
		{
			printf("Warning: Mixing cases may yield inaccurate results.\n");
			break;
		}
		
		strChar1++;
		strChar2++;
	}
	
	autoCapsStrPtr = &string[0];
	if (*autoCapsStrPtr == '*')
	{
		autoCapsPtrDist = 0;
		while (*autoCapsStrPtr == '*')
		{
			autoCapsStrPtr++;
			autoCapsPtrDist++;
		}
		autoCapsPtrDist--;     // Used to set autoCapsHexChar later in makeTable().
	}
	else if (autocaps) printf("Error: No wildcard for AutoCaps was at the start of the string.\n");
	
	FILE *romFile;
	romFile = fopen(argv[1], "rb");
	
	if (romFile == NULL)
	{
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	
	fseek(romFile, 0, SEEK_END);
	romSize = ftell(romFile);
	// printf("ROM size: %lu bytes\n", romSize);
	rewind(romFile);
	
	romInMem = (unsigned char*) malloc(romSize);
	if (romInMem == NULL)
	{
		perror("Allocating space for ROM in RAM");
		exit(EXIT_FAILURE);
	}
	
	if (!fread(romInMem, 1, romSize, romFile))
	{
		perror("Copying ROM into RAM");
		free(romInMem);
		exit(EXIT_FAILURE);
	}
	
	fclose(romFile);
	
	beginningOfRom = romInMem;
	endOfRom = romInMem + romSize - 1;
	
	while (romChar2 <= endOfRom)  // If romChar2 is on the last byte of the file, and it failed the previous test, the string isn't in the ROM.
	{
		// printf("byte count: %d (0x%X)\n", byteCount, byteCount);
		strChar1 = &string[0];
		strChar2 = &string[1];
		// printf("strChars: %02X %02X\n", *strChar1, *strChar2);
		romChar1 = beginningOfRom + byteCount;
		romChar2 = romChar1 + 1;
		// printf("romChars: %02X %02X\n", *romChar1, *romChar2);
		
		bool tryNextChars = 1;    // Should we continue testing for relativity from a certain offset or move on?
		while (tryNextChars)
		{
			if (romChar2 > endOfRom) break;
			if (strChar1 == &string[strlen(string) - 1])
			{
				disp--;
				if (disp == 0) searchComplete();
			}
			
			tryNextChars = checkRelativity(*strChar1, *strChar2, *romChar1, *romChar2);
			
			if (*strChar1 == '*' || *strChar2 == '*') tryNextChars = 1;    // Lazy wildcard check (does not check letters before and after the wildcard for matching relativity).
			
			strChar1++;
			strChar2++;
			romChar1++;
			romChar2++;
		}

		byteCount++;
	}
	
	printf("String not found!\n");
	free(romInMem);
	
	return 0;
}

bool checkRelativity(char strChar1, char strChar2, unsigned char romChar1, unsigned char romChar2)
{
	int strCharsRelativity = strChar1 - strChar2;
	int romCharsRelativity = romChar1 - romChar2;
	if (strCharsRelativity == romCharsRelativity) return 1;
	else return 0;
}

void searchComplete(void)
{
	charComparison();
	if (mktbl) makeTable();
	
	free(romInMem);
	exit(EXIT_SUCCESS);
}

void charComparison(void)
{
	printf("Input:");
	int i;
	for (i = 0; i < strlen(string); i++)
	{
		if (string[i] == '*') printf("  *");
		else printf(" %02X", string[i]);
	}
	
	printf("\nROM:  ");
	unsigned char *romChar;
	for (romChar = beginningOfRom + byteCount; romChar < romChar2; romChar++)
	{
		printf(" %02X", *romChar);
	}
	
	printf("\n");
	printf("Offset: 0x%X (%ld)\n", (unsigned int) byteCount, byteCount);
}

void makeTable(void)
{
	unsigned char *hexPtr = (romChar2 - strlen(string));
	char *asciiPtr = &string[0];
	while (*asciiPtr == '*')   // If wildcards were at the beginning of the string, we want to advance to a non-wildcard.
	{
		hexPtr++;
		asciiPtr++;
	}
	
	unsigned char hexChar = *hexPtr;
	char asciiChar = *asciiPtr;
	
	while (asciiChar != 'A' && asciiChar != 'a')    // "Rewind" the characters back to the beginning of the alphabet.
	{
		hexChar--;
		asciiChar--;
	}
	
	FILE *tblExists;
	tblExists = fopen(strcat(fileNameNoExt(filePath), ".tbl"), "r");
	if (tblExists != NULL)                // Check if table already exists, and if so, only allow overwriting if it was specified with "-over".
	{
		if (tblOverwrite == 0)
		{
			printf("Error: Table already exists. Run with \"-f\" to overwrite.\n");
			fclose(tblExists);
			return;
		}
	}
	
	FILE *romTable;
	romTable = fopen(strcat(fileNameNoExt(filePath), ".tbl"), "w");
	
	if (romTable == NULL)
	{
		perror("Creating table");
		return;
	}
	
	printf("Creating table... ");
	
	if (autocaps)  // If "-autocaps" was present when the program was run.
	{
		unsigned char autoCapsHexChar = *(beginningOfRom + byteCount + autoCapsPtrDist);    // Point to the byte directly preceding the first non-wildcarded byte.
		while (autoCapsAsciiChar > 'A')    // "Rewind" uppercase characters back to A.
		{
			autoCapsAsciiChar--;
			autoCapsHexChar--;
		}
		
		if (autoCapsHexChar < hexChar)    // If the uppercase alphabet is before the lowercase in the ROM's text system, output it before the lowercase.
		{
			while (autoCapsAsciiChar != 'Z' + 1)
			{
				fprintf(romTable, "%02X", autoCapsHexChar);
				fprintf(romTable, "=%c\n", autoCapsAsciiChar);
				
				autoCapsHexChar++;
				autoCapsAsciiChar++;
			}
			
			while (asciiChar != 'Z' + 1 && asciiChar != 'z' + 1)
			{
				fprintf(romTable, "%02X", hexChar);
				fprintf(romTable, "=%c\n", asciiChar);
				
				hexChar++;
				asciiChar++;
			}
		}
		
		else    // If the uppercase alphabet is after the lowercase in the ROM's text system, output it after the lowercase.
		{
			while (asciiChar != 'Z' + 1 && asciiChar != 'z' + 1)
			{
				fprintf(romTable, "%02X", hexChar);
				fprintf(romTable, "=%c\n", asciiChar);
				
				hexChar++;
				asciiChar++;
			}
			
			while (autoCapsAsciiChar != 'Z' + 1)
			{
				fprintf(romTable, "%02X", autoCapsHexChar);
				fprintf(romTable, "=%c\n", autoCapsAsciiChar);
				
				autoCapsHexChar++;
				autoCapsAsciiChar++;
			}
		}
	}
	
	else    // If the "-autocaps" option was not enabled.
	{
		while (asciiChar != 'Z' + 1 && asciiChar != 'z' + 1)
		{
			fprintf(romTable, "%02X", hexChar);
			fprintf(romTable, "=%c\n", asciiChar);
			
			hexChar++;
			asciiChar++;
		}
	}
	
	fclose(romTable);
	
	printf("Done!\n");
}
