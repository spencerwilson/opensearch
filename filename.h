// Name: File Name functions
// Author: mcpancakes
// Date: 23 Feb 2009

// Pass these functions an array that is a file name or path, and they will return the name
// with the extension, the name without the extension, or the extension, respectively.

char *fileName(char *filePath);        // Returns the name of the file, with the extension.
char *fileNameNoExt(char *filePath);   // Returns the name of the file, without the extension.
char *fileExt(char *filePath);         // Returns the file extension, without the '.'. Returns null if there is no file extension.

