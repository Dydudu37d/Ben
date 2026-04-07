#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Grammar.h"
#include "DataProcess.h"
#include "MsgError.h"

void FreeToken(GrammarToken** Tokens){
    if (!Tokens) return;
    
    for (int i = 0; Tokens[i] != NULL; i++) {
        if (Tokens[i]->Category) free(Tokens[i]->Category);
        if (Tokens[i]->Code) free(Tokens[i]->Code);
        free(Tokens[i]);
    }
    free(Tokens);
}

GrammarToken* CreateTokenWithCode(const char* code) {
    if (!code) return NULL; // 加个保险
    GrammarToken* Token = malloc(sizeof(GrammarToken));
    if (!Token) return NULL;
    
    Token->Code = strdup(code);
    Token->Category = NULL; // 初始化
    return Token;
}

GrammarToken** CodeToGrammar(const char* FileCode){
    GrammarToken** Grammar = malloc(sizeof(GrammarToken*) * 1024);
    if (Grammar == NULL) goto InitGrammarNULL;

    int LenTokens = 1024;
    int TokenCount = 0;

    const char* pos = FileCode;

    while (*pos) {
        while (*pos==' '||*pos=='\t'||*pos=='\n'||*pos=='\r') pos++;

        while (*pos && (unsigned char)*pos <= 32) {
            pos++;
        }
        if (!*pos) break; // 如果跳完之后到头了，直接结束
        if (*pos == '\0') break;

        // ===== IDENTIFIER / KEYWORD =====
        if ((*pos >= 'a' && *pos <= 'z') || (*pos >= 'A' && *pos <= 'Z') || *pos == '_') {

            const char* start = pos;
            while (*pos && (
                (*pos >= 'a' && *pos <= 'z') ||
                (*pos >= 'A' && *pos <= 'Z') ||
                (*pos >= '0' && *pos <= '9') ||
                *pos == '_'
            )) pos++;

            int len = pos - start;
            char* token_str = malloc(len + 1);
            strncpy(token_str, start, len);
            token_str[len] = '\0';

            GrammarToken* Token = CreateTokenWithCode(token_str);
            if (!Token) goto NewTokenNULL;

            const char* keywords[] = {
                "func","return","ret","exit","asm","type","indef",
                "num","int","float","double","void","extern","import",
                "const","unsigned","long","char",NULL
            };

            _Bool is_keyword = 0;
            for (int i=0; keywords[i]; i++) {
                if (strcmp(token_str, keywords[i]) == 0) {
                    is_keyword = 1;
                    break;
                }
            }

            Token->Category = strdup(is_keyword ? "Keyword" : "Identifier");
            Grammar[TokenCount++] = Token;

            // ===== FIX：asm block =====
            if (is_keyword && strcmp(token_str, "asm") == 0) {

                while (*pos==' '||*pos=='\t'||*pos=='\n'||*pos=='\r') pos++;

                if (*pos == '[') {

                    // [
                    GrammarToken* l = CreateTokenWithCode("[");
                    l->Category = strdup("Punctuation");
                    Grammar[TokenCount++] = l;
                    pos++;

                    const char* asm_start = pos;
                    int depth = 1;

                    while (*pos && depth > 0) {
                        if (*pos == '[') depth++;
                        else if (*pos == ']') {
                            depth--;
                            if (depth == 0) break;
                        }
                        pos++;
                    }

                    int asm_len = pos - asm_start;
                    char* asm_code = malloc(asm_len + 1);
                    strncpy(asm_code, asm_start, asm_len);
                    asm_code[asm_len] = '\0';

                    GrammarToken* a = CreateTokenWithCode(asm_code);
                    a->Category = strdup("Assembly");
                    Grammar[TokenCount++] = a;

                    free(asm_code);

                    // ✅ 修复：吃掉 ]
                    if (*pos == ']') {
                        GrammarToken* r = CreateTokenWithCode("]");
                        r->Category = strdup("Punctuation");
                        Grammar[TokenCount++] = r;
                        pos++;
                    }
                }
            }

            free(token_str);
        }

        // ===== NUMBER =====
        else if (*pos >= '0' && *pos <= '9') {
            const char* start = pos;
            while (*pos && ((*pos >= '0' && *pos <= '9') || *pos=='.')) pos++;

            int len = pos - start;
            char* token_str = malloc(len+1);
            strncpy(token_str, start, len);
            token_str[len] = '\0';

            GrammarToken* t = CreateTokenWithCode(token_str);
            t->Category = strdup("Number");
            Grammar[TokenCount++] = t;

            free(token_str);
        }

        // ===== STRING =====
        else if (*pos == '"') {
            const char* start = pos++;
            while (*pos && *pos != '"') pos++;
            if (*pos == '"') pos++;

            int len = pos - start;
            char* token_str = malloc(len+1);
            strncpy(token_str, start, len);
            token_str[len] = '\0';

            GrammarToken* t = CreateTokenWithCode(token_str);
            t->Category = strdup("String");
            Grammar[TokenCount++] = t;

            free(token_str);
        }

        // 在 CodeToGrammar 循环中
        else if (*pos == '\'') {
            const char* start = pos++;
            if (*pos == '\\') pos += 2; // 处理转义
            else pos++;
            if (*pos == '\'') pos++;

            int len = pos - start;
            char* token_str = malloc(len + 1);
            strncpy(token_str, start, len);
            token_str[len] = '\0';

            GrammarToken* t = CreateTokenWithCode(token_str);
            t->Category = strdup("Character"); // 必须是这个 Category
            Grammar[TokenCount++] = t;
            free(token_str);
        }

        // ===== SYMBOL =====
        else {
            if ((unsigned char)*pos > 32 && (unsigned char)*pos < 127) {
                char tmp[2] = {*pos++, '\0'};
                GrammarToken* t = CreateTokenWithCode(tmp);

                if (strchr("()[];:,", tmp[0]))
                    t->Category = strdup("Punctuation");
                else
                    t->Category = strdup("Symbol");

                Grammar[TokenCount++] = t;
            } else {
                // 发现非法垃圾字符，直接跳过并闭嘴
                pos++; 
            }
        }

        if (TokenCount >= LenTokens - 1) {
            int OldLen = LenTokens;
            LenTokens *= 2;
            GrammarToken** NewGrammar = realloc(Grammar, sizeof(GrammarToken*) * LenTokens);
            if (!NewGrammar) {
                fprintf(stderr, "Out of memory!\n");
                goto NewGrammarNULL;
                exit(3);
            }
            Grammar = NewGrammar;
            // 确保新空间全部初始化为 NULL
            memset(Grammar + OldLen, 0, (LenTokens - OldLen) * sizeof(GrammarToken*));
        }
    }

    Grammar[TokenCount] = NULL;
    return Grammar;

NewGrammarNULL:
    printf("Memory reallocation failed\n");
    exit(3);

InitGrammarNULL:
    printf("Memory malloc failed\n");
    exit(3);

NewTokenNULL:
    printf("Failed to create token\n");
    exit(4);
}