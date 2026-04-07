#include "DataProcess.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

size_t StrlenList(char** list){
    size_t s = 0;
    while (list[s]) s++;
    return s;
}

_Bool CharInList(char* arg, char** list) {
    for (int i = 0; list[i] != NULL; i++) {
        if (strcmp(list[i], arg) == 0) {
            return 1;
        }
    }
    return 0;
}

char* GetStr(char* str, int from, int to) {
    if (!str || from < 0 || to <= from) return NULL;
    
    int len = to - from;
    char* result = malloc(len + 1);
    if (!result) return NULL;
    
    strncpy(result, str + from, len);
    result[len] = '\0';
    
    return result;
}

// 獲取第 line 行的內容（從 1 開始）
char* GetLine(char* str, int line) {
    if (!str || line < 1) return NULL;
    
    int current_line = 1;
    int start = 0;
    int pos = 0;
    
    while (str[pos] != '\0') {
        if (current_line == line) {
            start = pos;
            // 找到行尾
            while (str[pos] != '\n' && str[pos] != '\0') pos++;
            return GetStr(str, start, pos);
        }
        if (str[pos] == '\n') {
            current_line++;
        }
        pos++;
    }
    
    return NULL;
}

// 一次遍历完成分割，O(n)
char** ListCodeChar(char Char, char *FileCode) {
    if (!FileCode) return NULL;
    
    // 第一遍：数行数
    size_t lines = 1;  // 至少一行
    for (char* p = FileCode; *p; p++) {
        if (*p == Char) lines++;  // 假设 Char 是 '\n'
    }
    
    char** list = malloc(sizeof(char*) * (lines + 1));
    if (!list) return NULL;
    
    // 第二遍：复制每行
    size_t idx = 0;
    char* start = FileCode;
    for (char* p = FileCode; ; p++) {
        if (*p == Char || *p == '\0') {
            size_t len = p - start;
            list[idx] = malloc(len + 1);
            if (!list[idx]) { /* 错误处理，释放已分配的 */ }
            strncpy(list[idx], start, len);
            list[idx][len] = '\0';
            idx++;
            start = p + 1;
            if (*p == '\0') break;
        }
    }
    list[idx] = NULL;
    return list;
}