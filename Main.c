#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "DataProcess.h"
#include "MsgError.h"
#include "Grammar.h"
#include "Compiler.h"

#ifdef _WIN32
    #include <io.h>
    #define access _access
    #define F_OK 0
#else
    #include <unistd.h>
#endif

#ifdef _WIN32
    #define NASM_FORMAT "win32"
    #define LD_EMULATION "i386pe"
    #define GCC_MARCH ""
    #define OBJ_EXT ".o"
#elif __linux__
    #ifdef __x86_64__
        #define NASM_FORMAT "elf64"
        #define LD_EMULATION "elf_x86_64"
        #define GCC_MARCH "-m64"
        #define OBJ_EXT ".o"
    #else
        #define NASM_FORMAT "elf32"
        #define LD_EMULATION "elf_i386"
        #define GCC_MARCH "-m32"
        #define OBJ_EXT ".o"
    #endif
#elif __APPLE__
    #define NASM_FORMAT "macho32"
    #define LD_EMULATION ""
    #define GCC_MARCH "-m32"
    #define OBJ_EXT ".o"
#endif

char* BArgs[] = {"-S","-o [files]","-i [files]","-nl","-mi","-h",NULL};
char* BArgsHelp[] = {"To Assembly","Output","Input","No link","More info in compile","Show Help",NULL};
size_t BArgc = (sizeof(BArgs) / sizeof(BArgs[0]) - 1);

_Bool ToASM = 0;
_Bool MoreInfo = 0;
_Bool Nolink = 0;

void PrintHelp(){
    for (int idx=0; idx<BArgc; idx++){
        printf("%s : %s\n", BArgs[idx], BArgsHelp[idx]);
    }
}

_Bool file_exists(const char* filename) {
    return access(filename, F_OK) == 0;
}

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb"); // 建议用 "rb" 防止 Windows 换行符干扰长度计算
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* content = malloc(size + 1);
    if (!content) { fclose(f); return NULL; }

    size_t read_size = fread(content, 1, size, f);
    content[read_size] = '\0'; // 👈 必须用实际读取的大小来封口！
    
    fclose(f);
    return content;
}

static char* expand_imports(const char* source, const char* current_file, int depth) {
    if (depth > 10) {
        PrintError("", "Import nesting too deep (>10)", 0, 0);
        return NULL;
    }

    size_t capacity = 1024 * 64; // 初始 64KB
    size_t current_size = 0;
    char* result = malloc(capacity);
    if (!result) return NULL;
    result[0] = '\0';

    const char* pos = source;
    while (*pos) {
        // 检测到 import 关键字
        if (strncmp(pos, "import", 6) == 0 && (*(pos + 6) == ' ' || *(pos + 6) == '\t' || *(pos + 6) == '"')) {
            pos += 6;
            while (*pos == ' ' || *pos == '\t') pos++;
            const char* filename_start = (*pos == '"') ? (++pos) : pos;
            while (*pos && *pos != '"' && *pos != ' ' && *pos != '\t' && *pos != ';' && *pos != '\n') pos++;
            
            int len = pos - filename_start;
            char* filename = malloc(len + 1);
            strncpy(filename, filename_start, len);
            filename[len] = '\0';
            
            if (*pos == '"') pos++;
            while (*pos == ' ' || *pos == '\t') pos++;
            if (*pos == ';') pos++;

            // 递归读取被导入的文件
            char* imported = read_file(filename);
            if (!imported) {
                PrintError("", "Cannot open import file", 0, 0);
                printf(Error("  File: %s\n"), filename);
                free(filename); free(result); return NULL;
            }

            char* expanded = expand_imports(imported, filename, depth + 1);
            if (!expanded) { free(filename); free(imported); free(result); return NULL; }

            // 🚀 动态扩容检查
            size_t expanded_len = strlen(expanded);
            while (current_size + expanded_len >= capacity) {
                capacity *= 2;
                char* new_res = realloc(result, capacity);
                if (!new_res) { /* 处理错误 */ exit(3); }
                result = new_res;
            }

            // 拼接
            memcpy(result + current_size, expanded, expanded_len);
            current_size += expanded_len;
            result[current_size] = '\0';

            free(expanded); free(imported); free(filename);
        } 
        else {
            // 🚀 普通字符的动态扩容检查
            if (current_size + 1 >= capacity) {
                capacity *= 2;
                result = realloc(result, capacity);
            }
            result[current_size++] = *pos++;
            result[current_size] = '\0';
        }
    }
    return result;
}

int main(int argc,char** argv){
    if (argc == 1){
        PrintHelp();
        exit(0);
    }

    if (strcmp(argv[1],"NyanNyan") == 0){
        printf("NyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\n");
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=2yJgwwDcgV8");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=2yJgwwDcgV8");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=2yJgwwDcgV8");
        #endif
        exit(0);
    }else if(strcmp(argv[1],"FUCK") == 0){
        printf("Never Gonna Give You Up~\nNever Gonna Give You Up~\nNever Gonna Give You Up~\nNever Gonna Give You Up~\nNever Gonna Give You Up~\n");
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=dQw4w9WgXcQ");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=dQw4w9WgXcQ");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=dQw4w9WgXcQ");
        #endif
        exit(0);
    }else if(strcmp(argv[1], "Study") == 0){
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=nr3GOE_AJNQ");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=nr3GOE_AJNQ");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=nr3GOE_AJNQ");
        #endif
        exit(0);
    }else if(strcmp(argv[1], "III") == 0){
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=aX4v5XUQtnw");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=aX4v5XUQtnw");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=aX4v5XUQtnw");
        #endif
        exit(0);
    }else if(strcmp(argv[1], "Rei") == 0){
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=S4wuQWHguV8");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=S4wuQWHguV8");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=S4wuQWHguV8");
        #endif
        exit(0);
    }else if(strcmp(argv[1], "WhatIsLove") == 0){
        printf("I will teach you to be real\n");
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=bVqGnaE2-wM");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=bVqGnaE2-wM");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=bVqGnaE2-wM");
        #endif
        exit(0);
    }else if(strcmp(argv[1], "Love") == 0){
        printf("Teto I love you!\nTeto I love you!\nTeto I love you!\nTeto I love you!\nTeto I love you!\nTeto I love you!\nTeto I love you!\n");
        #ifdef _WIN32
            system("start https://www.youtube.com/watch?v=sqK-jh4TDXo");
        #elif __linux__
            system("xdg-open https://www.youtube.com/watch?v=sqK-jh4TDXo");
        #elif __APPLE__
            system("open https://www.youtube.com/watch?v=sqK-jh4TDXo");
        #endif
        exit(0);
    }else if(strcmp(argv[1], "Engineer") == 0) {
        printf("I don't want to be an engineer~\n");
        printf("But I already am.\n");
        printf("Suka blyat.\n");
        exit(0);
    }else if(strcmp(argv[1], "Suka") == 0 || strcmp(argv[1], "Blyat") == 0) {
        printf("SUKA BLYAT!!!\n");
        printf("Your GPU is crying.\n");
        printf("Your CPU is crying.\n");
        printf("Your GPU is runing.\n");
        printf("Your CPU is runing.\n");
        printf("Your PC is runing.\n");
        printf("You need to try it.\n");
        exit(0);
    }

    FILE* Input=NULL;
    FILE* Output=NULL;
    char* input_filename=NULL;
    char* output_filename=NULL;

    for (int ArgIdx=1;ArgIdx<argc;ArgIdx++){
        int ArgLen=strlen(argv[ArgIdx]);
        if (ArgLen>=4 && strcmp(argv[ArgIdx]+ArgLen-4,".ben")==0 && file_exists(argv[ArgIdx])){
            Input=fopen(argv[ArgIdx],"r");
            input_filename=argv[ArgIdx];
            continue;
        }
        if (strcmp(argv[ArgIdx],"-i")==0){
            if (!(ArgIdx+1<argc)) { printf("-i no file specified\n"); PrintHelp(); exit(EINVAL); }
            int file_len=strlen(argv[ArgIdx+1]);
            if (file_len>=4 && strcmp(argv[ArgIdx+1]+file_len-4,".ben")==0 && file_exists(argv[ArgIdx+1])){
                Input=fopen(argv[ArgIdx+1],"r");
                input_filename=argv[ArgIdx+1];
                if (!Input){ printf(Error("Error: Cannot open file '%s'\n"), argv[ArgIdx+1]); exit(EIO); }
                ArgIdx++;
                continue;
            } else { printf("No file specified\n"); PrintHelp(); exit(EINVAL); }
        }
        if (strcmp(argv[ArgIdx],"-o")==0){
            if (ArgIdx+1>=argc){ fprintf(stderr,Error("Error: -o requires a filename\n")); PrintHelp(); exit(EINVAL); }
            ArgIdx++;
            Output=fopen(argv[ArgIdx],"wb");
            output_filename=argv[ArgIdx];
            if (!Output){ fprintf(stderr,Error("Error: Cannot create output file '%s'\n"), argv[ArgIdx]); exit(EIO); }
            continue;
        }
        if (strcmp(argv[ArgIdx],"-S")==0) ToASM=1;
        if (strcmp(argv[ArgIdx],"-nl")==0) Nolink=1;
        if (strcmp(argv[ArgIdx],"-mi")==0) MoreInfo=1;
    }

    if (!Input){ printf(Error("Error: No input file specified\n")); PrintHelp(); exit(EINVAL); }
    
    fseek(Input, 0, SEEK_END);
    long size = ftell(Input);
    rewind(Input);
    
    char* content = malloc(size + 1);
    // 关键修复：用 read_actual 接收实际读到的长度
    size_t read_actual = fread(content, 1, size, Input); 
    content[read_actual] = '\0'; // 👈 必须用这个变量封口！
    fclose(Input);

    char* expanded=expand_imports(content,input_filename,0);
    if (!expanded){ free(content); exit(1); }
    free(content);
    
    if (MoreInfo) {
        printf("Debug: Total Source Length = %llu\n", (unsigned long long)strlen(expanded));
    }
    // 如果这个长度远大于你的代码实际字符数，那说明残留就在这一步发生的
    GrammarToken** tokens = CodeToGrammar(expanded);
    if (!tokens){ free(expanded); exit(1); }
    free(expanded);

    FILE* output_file = Output;
    if (!Output){
        char* output_name = malloc(strlen(input_filename)+3);
        strcpy(output_name,input_filename);
        char* dot=strrchr(output_name,'.'); if(dot) *dot='\0';
        strcat(output_name,".s");
        output_file=fopen(output_name,"w");
    }

    Compiler* compiler=init_compiler(tokens,output_file);
    if(!compiler){ fprintf(stderr,Error("Error: Failed to initialize compiler\n")); FreeToken(tokens); if(Output) fclose(Output); exit(1);}
    compile(compiler);
    free_compiler(compiler);
    FreeToken(tokens);
    if(Output) fclose(Output); else if(output_file) fclose(output_file);
    

    if(!Nolink && !ToASM){
        char* exe_name = malloc(strlen(input_filename)+5);
        strcpy(exe_name,input_filename);
        char* dot=strrchr(exe_name,'.'); if(dot) *dot='\0'; strcat(exe_name,".exe");

        char* asm_name = malloc(strlen(input_filename)+3);
        strcpy(asm_name,input_filename);
        dot=strrchr(asm_name,'.'); if(dot) *dot='\0'; strcat(asm_name,".s");

        char* obj_name = malloc(strlen(input_filename)+3);
        strcpy(obj_name,input_filename);
        dot=strrchr(obj_name,'.'); if(dot) *dot='\0'; strcat(obj_name,".o");

        // assemble
        char cmd_asm[1024];
        sprintf(cmd_asm, "as --64 -o %s %s", obj_name, asm_name);
        printf("DEBUG: cmd = [%s]\n", cmd_asm);  // 加这行
        printf("DEBUG: file exists? %d\n", file_exists(asm_name));  // 确认文件存在
        printf("Assembling with as...\n");
        if(system(cmd_asm)!=0){ fprintf(stderr,"Error: Assembling failed\n"); exit(1); }

        // link
        char cmd_link[1024];
        // 修改 main.c
        sprintf(cmd_link, "gcc -m64 %s -o %s -Wl,-e,_start -nostdlib -lkernel32 -luser32 -lmsvcrt", obj_name, exe_name);
        printf("Linking with gcc...\n");
        if(system(cmd_link)!=0){ fprintf(stderr,"Error: Linking failed\n"); exit(1); }

        // 删除临时文件
        remove(asm_name);
        remove(obj_name);
        free(exe_name);
        free(asm_name);
        free(obj_name);
    }

    printf(Success("\nCompilation completed successfully!\nBye! Have a good day.\n"));
    exit(0);
}