#ifndef GRAMMAR_H
#define GRAMMAR_H

typedef struct
{
    char *Code;
    char *Category;
} GrammarToken;

void FreeToken(GrammarToken** Token);
GrammarToken** CodeToGrammar(const char* FileCode);

#endif