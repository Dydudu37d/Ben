#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Grammar.h"
#include "StrMath.h"
#include "DataProcess.h"

_Bool is_type(const char* token) {
    if (!token) return 0;
    return (strcmp(token, "num") == 0 ||
            strcmp(token, "char") == 0 ||
            strcmp(token, "str") == 0 ||
            strcmp(token, "int") == 0 ||    // 新增
            strcmp(token, "ptr") == 0 ||    // 为你的 OS 指针预留
            strcmp(token, "void") == 0);
}

char* process_escape_sequences(const char* str) {
    if (!str) return NULL;
    int len = strlen(str);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] == '\\' && i + 1 < len) {
            switch (str[i + 1]) {
                case 'n': result[j++] = '\n'; i++; break;
                case 't': result[j++] = '\t'; i++; break;
                case 'r': result[j++] = '\r'; i++; break;
                case '\\': result[j++] = '\\'; i++; break;
                case '"': result[j++] = '"'; i++; break;
                default: result[j++] = str[i]; break;
            }
        } else {
            result[j++] = str[i];
        }
    }
    result[j] = '\0';
    return result;
}

typedef struct {
    GrammarToken** tokens;
    int current_token;
    FILE* output;
} Compiler;

Compiler* init_compiler(GrammarToken** tokens, FILE* output) {
    Compiler* compiler = malloc(sizeof(Compiler));
    if (!compiler) return NULL;
    compiler->tokens = tokens;
    compiler->current_token = 0;
    compiler->output = output;
    return compiler;
}

void free_compiler(Compiler* compiler) {
    if (compiler) free(compiler);
}

GrammarToken* current_token(Compiler* compiler) {
    if (!compiler || !compiler->tokens) return NULL;
    return compiler->tokens[compiler->current_token];
}

void next_token(Compiler* compiler) {
    if (compiler->tokens[compiler->current_token]) {
        compiler->current_token++;
    }
}

// 1. 数据段生成器 (扫描全局变量)
void compile_data_section(Compiler* compiler) {
    fprintf(compiler->output, ".section .data\n");
    
    int i = 0;
    while (compiler->tokens[i] != NULL) {
        GrammarToken* tok = compiler->tokens[i];

        // 处理类型定义: num a = 1.2;
        if (strcmp(tok->Category, "Keyword") == 0 && is_type(tok->Code)) {
            char* type_name = tok->Code;
            
            if (compiler->tokens[i+1] && strcmp(compiler->tokens[i+1]->Category, "Identifier") == 0) {
                char* name = compiler->tokens[i+1]->Code;

                // 如果后面跟着 '('，说明是函数声明，不是变量定义，跳过
                if (compiler->tokens[i+2] && strcmp(compiler->tokens[i+2]->Code, "(") == 0) {
                    i++; 
                    continue;
                }

                const char* asm_directive = ".quad"; 
                if (strcmp(type_name, "num") == 0) asm_directive = ".double";
                else if (strcmp(type_name, "int") == 0) asm_directive = ".long";
                else if (strcmp(type_name, "char") == 0) asm_directive = ".byte";

                // 处理初始化赋值
                if (compiler->tokens[i+2] && strcmp(compiler->tokens[i+2]->Code, "=") == 0) {
                    int j = i + 3;
                    char expr_buf[1024] = {0};

                    while (compiler->tokens[j] && strcmp(compiler->tokens[j]->Code, ";") != 0) {
                        strcat(expr_buf, compiler->tokens[j]->Code);
                        j++;
                    }

                    if (strcmp(type_name, "str") == 0) {
                        fprintf(compiler->output, "var_%s: .asciz %s\n", name, compiler->tokens[i+3]->Code);
                    } else if (strcmp(type_name, "num") == 0) {
                        fprintf(compiler->output, "var_%s: .double %g\n", name, EvaluateConstant(expr_buf));
                    } else {
                        fprintf(compiler->output, "var_%s: %s %ld\n", name, asm_directive, (long)EvaluateConstant(expr_buf));
                    }
                    i = j; // 移动到分号处
                } else {
                    // 无初始化定义
                    fprintf(compiler->output, "var_%s: %s 0\n", name, asm_directive);
                    i += 1; 
                }
            }
        }
        
        // 跳过函数体 [ ... ] 以防内部局部变量被误当做全局变量
        if (strcmp(tok->Code, "func") == 0) {
            while (compiler->tokens[i] && strcmp(compiler->tokens[i]->Code, "[") != 0) i++;
            int depth = 1; i++;
            while (compiler->tokens[i] && depth > 0) {
                if (strcmp(compiler->tokens[i]->Code, "[") == 0) depth++;
                if (strcmp(compiler->tokens[i]->Code, "]") == 0) depth--;
                i++;
            }
            continue;
        }
        i++;
    }
}

_Bool match(Compiler* compiler, const char* category, const char* value) {
    GrammarToken* token = current_token(compiler);
    if (!token) return 0;
    if (category && strcmp(token->Category, category) != 0) return 0;
    if (value != NULL && strcmp(token->Code, value) != 0) return 0;
    return 1;
}

void expect(Compiler* compiler, const char* category, const char* value) {
    if (!match(compiler, category, value)) {
        GrammarToken* token = current_token(compiler);
        fprintf(stderr, "Error: Expected %s %s, got %s %s\n", 
                category, value,
                token ? token->Category : "NULL",
                token ? token->Code : "NULL");
        exit(1);
    }
    next_token(compiler);
}

// 处理函数定义
void process_function(Compiler* compiler) {
    expect(compiler, "Keyword", "func");
    
    // 跳过返回类型
    if (match(compiler, "Keyword", "num") || 
        match(compiler, "Keyword", "int") || 
        match(compiler, "Keyword", "char") ||
        match(compiler, "Keyword", "void")) {
        next_token(compiler);
    }
    
    // 函数名
    if (match(compiler, "Identifier", NULL)) {
        const char* func_name = current_token(compiler)->Code;
        next_token(compiler);
        
        // 生成函数标签
        fprintf(compiler->output, "_%s:\n", func_name);
        
        // 跳过参数列表 ( ... )
        expect(compiler, "Punctuation", "(");
        int paren_depth = 1;
        while (paren_depth > 0 && current_token(compiler)) {
            if (match(compiler, "Punctuation", "(")) paren_depth++;
            if (match(compiler, "Punctuation", ")")) paren_depth--;
            next_token(compiler);
        }
        
        // 找到函数体开始的 [
        while (current_token(compiler) && !match(compiler, "Punctuation", "[")) {
            next_token(compiler);
        }
        
        if (match(compiler, "Punctuation", "[")) {
            next_token(compiler);  // 跳过 [
            
            // 处理函数体
            while (current_token(compiler) && !match(compiler, "Punctuation", "]")) {

                if (match(compiler, "Keyword", "asm")) {
                    next_token(compiler);

                    if (match(compiler, "Punctuation", "[")) next_token(compiler);

                    GrammarToken* tok = current_token(compiler);
                    if (tok && strcmp(tok->Category, "Assembly") == 0) {
                        fprintf(compiler->output, "%s\n", tok->Code);
                        next_token(compiler);
                    }

                    if (match(compiler, "Punctuation", "]")) next_token(compiler);
                }
                else {
                    // 🔥 必须加
                    next_token(compiler);
                }
            }
            
            // 跳过函数体结束的 ]
            if (match(compiler, "Punctuation", "]")) {
                next_token(compiler);
            }
        }
        fprintf(compiler->output, "\n");
    }
}

// 编译主函数
void compile(Compiler* compiler) {
    compile_data_section(compiler);

    fprintf(compiler->output, ".code64\n");
    fprintf(compiler->output, ".section .text\n");
    fprintf(compiler->output, ".global _start\n\n");
    
    int original_pos = compiler->current_token;

    // 第一遍：处理 extern 声明
    while (current_token(compiler)) {
        if (match(compiler, "Keyword", "extern")) {
            next_token(compiler);
            if (current_token(compiler) && strcmp(current_token(compiler)->Category, "Identifier") == 0) {
                fprintf(compiler->output, ".extern %s\n", current_token(compiler)->Code);
                next_token(compiler);
            }
            while (current_token(compiler) && !match(compiler, "Punctuation", ";")) next_token(compiler);
            if (match(compiler, "Punctuation", ";")) next_token(compiler);
        } else {
            next_token(compiler);
        }
    }
    
    // 第二遍：处理自定义函数定义
    compiler->current_token = original_pos;
    while (current_token(compiler)) {
        if (match(compiler, "Keyword", "func")) {
            process_function(compiler);
        } else {
            next_token(compiler);
        }
    }
    
    // 第三遍：生成 _start 入口（main 逻辑）
    compiler->current_token = original_pos;
    fprintf(compiler->output, "_start:\n");
    fprintf(compiler->output, "    subq $48, %%rsp\n"); 
    
    int string_counter = 0;
    
    while (current_token(compiler)) {
        GrammarToken* token = current_token(compiler);
        if (token == NULL || token->Category == NULL) break;
        // --- 重点：生成变量赋值指令 ---
        if (strcmp(token->Category, "Keyword") == 0 && strcmp(token->Code, "num") == 0) {
            next_token(compiler); // 跳过 "num"
            
            GrammarToken* name_tok = current_token(compiler);
            if (name_tok && strcmp(name_tok->Category, "Identifier") == 0) {
                next_token(compiler); // 跳过 变量名
                
                // 检查是否有 '='
                if (match(compiler, "Punctuation", "=")) {
                    next_token(compiler); // 跳过 "="
                    
                    GrammarToken* val_tok = current_token(compiler);
                    if (val_tok) {
                        // 🚀 再次融合 bpp 的常量折叠
                        if (ConstantExpression(val_tok->Code)) {
                            long final_val = EvaluateConstant(val_tok->Code);
                            // 生成：movq $值, var_变量名(%rip)
                            fprintf(compiler->output, "    movq $%ld, var_%s(%%rip)\n", final_val, name_tok->Code);
                        } else {
                            // 如果右边也是变量：movq var_b(%rip), %rax -> movq %rax, var_a(%rip)
                            fprintf(compiler->output, "    movq var_%s(%%rip), %%rax\n", val_tok->Code);
                            fprintf(compiler->output, "    movq %%rax, var_%s(%%rip)\n", name_tok->Code);
                        }
                        next_token(compiler);
                    }
                }
            }
            
            // 吃掉分号
            while (current_token(compiler) && !match(compiler, "Punctuation", ";")) {
                next_token(compiler);
            }
            if (match(compiler, "Punctuation", ";")) next_token(compiler);
            
            continue; 
        }
        // --- 处理 关键字 ---
        if (strcmp(token->Category, "Keyword") == 0) {
            if (strcmp(token->Code, "extern") == 0 || strcmp(token->Code, "func") == 0) {
                // 跳过已经处理过的声明和定义
                next_token(compiler);
                while (current_token(compiler) && !match(compiler, "Punctuation", ";") && !match(compiler, "Punctuation", "]")) {
                    next_token(compiler);
                }
                if (match(compiler, "Punctuation", ";") || match(compiler, "Punctuation", "]")) next_token(compiler);
            } 
            else if (strcmp(token->Code, "ret") == 0) {
                next_token(compiler); // 跳过 ret
                
                char expr_buf[1024] = {0};
                while (current_token(compiler) && !match(compiler, "Punctuation", ";")) {
                    strcat(expr_buf, current_token(compiler)->Code);
                    next_token(compiler);
                }
                
                // 🚀 融合 bpp 的常量折叠
                if (ConstantExpression(expr_buf)) {
                    long final_val = EvaluateConstant(expr_buf);
                    fprintf(compiler->output, "    movq $%ld, %%rax\n", final_val);
                } 
                else {
                    // 变量读取：var_name(%rip)
                    fprintf(compiler->output, "    movq var_%s(%%rip), %%rax\n", expr_buf);
                }

                fprintf(compiler->output, "    addq $48, %%rsp\n");
                fprintf(compiler->output, "    ret\n");
                if (match(compiler, "Punctuation", ";")) next_token(compiler);
            } 
            else {
                next_token(compiler);
            }
        }
        // --- 处理 标识符（函数调用） ---
        else if (strcmp(token->Category, "Identifier") == 0) {
            const char* func_name = token->Code;
            next_token(compiler);
            _Bool is_call = 0;

            while (current_token(compiler) && !match(compiler, "Punctuation", ";")) {
                is_call = 1;
                if (match(compiler, "String", NULL)) {
                    fprintf(compiler->output, ".section .data\n");
                    fprintf(compiler->output, "str_%d: .asciz %s\n", string_counter, current_token(compiler)->Code);
                    fprintf(compiler->output, ".section .text\n");
                    fprintf(compiler->output, "    leaq str_%d(%%rip), %%rcx\n", string_counter++);
                    next_token(compiler);
                } 
                else if (match(compiler, "Character", NULL)) {
                    // 你的增强版转义逻辑
                    char c = 0;
                    const char* code = current_token(compiler)->Code;
                    if (code[1] == '\\') {
                        switch (code[2]) {
                            case 'n':  c = '\n'; break;
                            case 't':  c = '\t'; break;
                            case 'r':  c = '\r'; break;
                            case '\\': c = '\\'; break;
                            case '0':  c = '\0'; break;
                            default:   c = code[2]; break;
                        }
                    } else { c = code[1]; }
                    fprintf(compiler->output, "    movq $%d, %%rcx\n", (unsigned char)c);
                    next_token(compiler);
                }
                else { next_token(compiler); }
            }
            
            if (is_call) fprintf(compiler->output, "    call _%s\n", func_name);
            
            if (match(compiler, "Punctuation", ";")) next_token(compiler);
            else { exit(1); } // 缺少分号报错
        }
        // --- 处理 直接汇编 ---
        else if (strcmp(token->Category, "Assembly") == 0) {
            fprintf(compiler->output, "%s\n", token->Code);
            next_token(compiler);
        }
        else {
            next_token(compiler);
        }
    }
    
    // 默认退出逻辑
    fprintf(compiler->output, "    xorq %%rax, %%rax\n");
    fprintf(compiler->output, "    addq $48, %%rsp\n");
    fprintf(compiler->output, "    ret\n");
}