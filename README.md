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

### 类型与内存映射:
- **char**:   8位数字  `[.byte]`
- **int**:    32位数字 `[.long]`
- **long**:   64位数字 `[.quad]`
- **float**:  32位单精度浮点数 `[.float]`
- **double**: 64位双精度浮点数 `[.double]`
- **num**:    通用数字 (默认 double, 不建议在内核高频使用) `[.double]`

### 修饰符:
- **unsigned**: 无符号正数。解放最高位符号位，扩大正数表达范围 (0~2^n-1)。处理内存地址 (ptr) 和硬件寄存器时的首选。
- **const**: 只读标记。防止变量在编译期被重新赋值。
- **void**: 占位符。表示无类型、无返回值，或配合 `ptr` 指向任意内存。
- **func**: 函数声明关键字。

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

