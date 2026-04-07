# βen 语言

## βen 的含义
- **β(beta)**
- **e(embeda)**
- **n(nested)**

## 编译
```bash
gcc -Wall -Wextra -std=c11 -o ben.exe Main.c Grammar.c DataProcess.c StrMath.c Compiler.c
```

## 运行
基础运行 (默认显示Help):
```bash
./ben.exe
```

指定源文件:
```bash
./ben.exe <your_code>.ben
```

## 示例:嵌入式汇编
```ben
func void HelloWorld() [
    asm [
        movq $1, %rax
        ret
    ];
]
```

## 关于std里的文件
- 测试用
- 以后的标准库
- 没弄完

## 如何入门:

### 类型:
- char: 8位数字 .byte
- long: 64位数字 .quad
- num: 任意数字 (要省内存的话不建议使用,因为默认double) .double
- int: 32位数字 .long
- double: 64位双精度浮点数 .double
- float: 32位单精度浮点数 .float

### 修饰符:
- unsigned: 无符号数
- const: 只读
- void: 函数/任意类型

- func: 定义函数

### 返回:
- ret: 只设置返回值
- return: 立刻返回

### 退出:
- exit

```ben
exit 0;
```

### 定义:
- type : typedef
- indef : define

``` ben
type u64 EFI_STATUS;
indef EFIAPI __attribute__((ms_abi));
```

### 空:
- NULL

```ben
void a = NULL;
```

### 外部定义:
- extern

```ben
extern WriteConsoleA;
```

### 导入:
- import

```ben
import std/Stdio.ben 
```

*(结尾加不加';'都可以)*

### 内联汇编:
- asm

```ben
asm[
    <汇编代码>
    <汇编代码>
    <汇编代码>
    <汇编代码>
    <汇编代码>
];
```

