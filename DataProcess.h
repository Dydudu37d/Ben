#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <stddef.h>

size_t StrlenList(char** list);
_Bool CharInList(char* arg, char** list);
char* GetStr(char* str, int from, int to);
char* GetLine(char* str, int line);
char** ListCodeChar(char Char,char *FileCode);

#endif