// Name: File Name functions
// Author: mcpancakes
// Date: 23 Feb 2009

// Pass these functions an array that is a file name or path, and they will return the name
// with the extension, the name without the extension, or the extension, respectively.

#include <stdlib.h>     // For malloc().
#include <string.h>     // For strlen().
#include "filename.h"

char *fileName(char *filePath)
{
	char *ptr = filePath + strlen(filePath) - 1;           // Point ptr to the last non-null character of filePath.
	while (*ptr != '/' && *ptr != '\\' && ptr != filePath - 1)    // Rewind to the \ or / closest to the end of filePath.
	  ptr--;
	
	ptr++;                                                 // Point ptr to the character after the \ or / closest to the end of filePath.
	
	char *fileName = (char *) malloc((sizeof(char) * strlen(filePath)) + 1);    // Allocate space for the original string + a terminating null byte.
	
	int i;
	for (i = 0; *(ptr + i) != '\0'; i++)      // While ptr isn't pointing to the terminating null character of filePath, copy a character to fileName.
	  *(fileName + i) = *(ptr + i);
	
	*(fileName + strlen(fileName)) = '\0';    // Append a terminating null byte to the end of filePath.
	
	return fileName;
}

char *fileNameNoExt(char *filePath)
{
	char *ptr = filePath + strlen(filePath) - 1;           // Point ptr to the last non-null character of filePath.
	while (*ptr != '\\' && *ptr != '/' && ptr != filePath - 1)    // Rewind to the \ or / closest to the end of filePath.
	  ptr--;
	
	ptr++;                                                 // Point ptr to the character after the \ or / closest to the end of filePath.
	
	char *fileName = (char *) malloc((sizeof(char) * strlen(filePath)) + 1);    // Allocate space for the original string + a terminating null byte.
	
	int i;
	for (i = 0; *(ptr + i) != '\0'; i++)       // While ptr isn't pointing to the terminating null character of filePath, copy a character to fileName.
	  *(fileName + i) = *(ptr + i);
	
	ptr = fileName + strlen(fileName);         // Point ptr to the terminating null byte of filePath.
	i = 0;
	while (*ptr != '.')                        // Rewind ptr to the last '.'.
	{
		if (ptr == fileName) break;
		ptr--;
	}
	
	while (*ptr != '\0' && ptr != fileName)                       // Overwrite the file extension with null bytes.
	{
		*ptr = '\0';
		ptr++;
	}
	
	*(fileName + strlen(fileName)) = '\0';
	
	return fileName;
}

char *fileExt(char *filePath)
{
	char *ptr = filePath + strlen(filePath) - 1;    // Point ptr to the last non-null character of filePath.
	while (*ptr != '.')                             // Rewind to the '.' closest to the end of filePath.
	{
		ptr--;
		if (ptr == filePath) return NULL;           // If ptr is pointing to the start of filePath, there is no file extension.
	}
	
	if (*(ptr - 1) == '/' || *(ptr - 1) == '\\') return NULL;    // If the file was contained in a directory, and *ptr is a '.', then there is no file extension.
	if (ptr + 1 == filePath + strlen(filePath)) return NULL;     // If the '.' is the last character of the file path, there is no file extension.
	ptr++;                                                       // Advance ptr to the first letter of the file extension.
	
	char *fileExt = (char *) malloc((sizeof(char) * strlen(filePath)) + 1);    // Allocate space for the original string + a terminating null byte.
	
	int i;
	for (i = 0; *(ptr + i) != '\0'; i++)    // While ptr isn't at the end of filePath, copy a character to fileExt.
	  *(fileExt + i) = *(ptr + i);
	
	*(fileExt + strlen(fileExt)) = '\0';    // Append a terminating null byte to the end of filePath.
	
	return fileExt;
}

