#include "StrMath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// 1. 将返回值和内部计算统一改为 double
double EvaluateConstant(char* expr) {
    char temp[1024];
    char clean_expr[1024] = {0};
    int j = 0;
    
    // 去除空白字符
    for (int i = 0; expr[i]; i++) {
        if (!isspace(expr[i])) {
            clean_expr[j++] = expr[i];
        }
    }
    strcpy(temp, clean_expr);
    
    char* ops[] = {"*/", "+-"}; 

    for (int p = 0; p < 2; p++) {
        for (int i = 0; temp[i] != '\0'; i++) {
            if (strchr(ops[p], temp[i])) {
                if (i == 0 && temp[i] == '-') continue;

                // --- 1. 寻找左侧数字起点 (支持小数点) ---
                int left_start = i - 1;
                while (left_start >= 0 && (isdigit(temp[left_start]) || temp[left_start] == '.')) {
                    left_start--;
                }
                left_start++;

                // --- 2. 寻找右侧数字终点 (支持小数点) ---
                int right_end = i + 1;
                while (temp[right_end] != '\0' && (isdigit(temp[right_end]) || temp[right_end] == '.')) {
                    right_end++;
                }

                // --- 3. 提取数字并计算 (换成 atof 和 double) ---
                char left_str[100] = {0}, right_str[100] = {0};
                strncpy(left_str, temp + left_start, i - left_start);
                strncpy(right_str, temp + i + 1, right_end - (i + 1));

                double left_val = atof(left_str);   // 改用 atof
                double right_val = atof(right_str); // 改用 atof
                double result = 0;                  // 改用 double

                if (temp[i] == '*') result = left_val * right_val;
                else if (temp[i] == '/') result = (right_val != 0) ? (left_val / right_val) : 0;
                else if (temp[i] == '+') result = left_val + right_val;
                else if (temp[i] == '-') result = left_val - right_val;

                // --- 4. 组装新字符串 (换成 %f) ---
                char new_temp[1024] = {0};
                char res_s[64];
                // 使用 %g 可以自动去掉多余的 0，比如 2.0000 -> 2
                sprintf(res_s, "%g", result); 

                strncat(new_temp, temp, left_start);
                strcat(new_temp, res_s);
                strcat(new_temp, temp + right_end);

                strcpy(temp, new_temp);
                i = -1; // 重新扫描
            }
        }
    }

    // 最终返回也要支持浮点
    double final_ans = atof(temp);
    printf("evaluate_constant returning: %s -> %g\n", temp, final_ans);
    return final_ans;
}

// 2. 判定函数也要放行小数点
_Bool ConstantExpression(char* expr) {
    if (!expr) return 0;
    for (char* p = expr; *p != '\0'; p++) {
        if (!isdigit(*p) && 
            *p != '+' && *p != '-' && *p != '*' && *p != '/' &&
            *p != ' ' && *p != '.' && *p != '\r' && *p != '\n') {
            return 0; 
        }
    }
    return 1;
}