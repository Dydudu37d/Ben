#ifndef COMPILER_H
#define COMPILER_H

#include "Grammar.h"

// 编译器结构
typedef struct {
    GrammarToken** tokens;
    int current_token;
    FILE* output;
} Compiler;

// 初始化编译器
Compiler* init_compiler(GrammarToken** tokens, FILE* output);

// 释放编译器
void free_compiler(Compiler* compiler);

// 编译函数
void compile(Compiler* compiler);

#endif
