#ifndef MSGCOLOR_H
#define MSGCOLOR_H

#include <stdio.h>
#include <stdlib.h>
#include "DataProcess.h"

#define Error(s)   "\033[1;31m" s "\033[0m"
#define Success(s) "\033[1;32m" s "\033[0m"
#define Warning(s) "\033[1;34m" s "\033[0m"
#define Text(s) "\033[1;37m]" s "\033[0m"

static inline void PrintError(char* Code, char* ErrorMsg, int x, int y) {
    printf(Error("Error"));
    printf(" at line %d, column %d: %s\n", y, x, ErrorMsg);
    
    // 獲取錯誤的那一行
    char* line = GetLine(Code, y);
    if (line) {
        printf("  %s\n", line);
        printf("  ");
        for (int i = 0; i < x - 1; i++) printf(" ");
        printf(Error("^\n"));
        free(line);
    }
    exit(1);
}

static inline void PrintWarning(char* Code, char* ErrorMsg, int x, int y) {
    printf(Warning("Warning"));
    printf(" at line %d, column %d: %s\n", y, x, ErrorMsg);
    
    char* line = GetLine(Code, y);
    if (line) {
        printf("  %s\n", line);
        printf("  ");
        for (int i = 0; i < x - 1; i++) printf(" ");
        printf(Warning("^\n"));
        free(line);
    }
}

#endif