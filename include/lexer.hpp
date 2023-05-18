#ifndef LEXER_HPP
#define LEXER_HPP

#include "processFileWithCode.hpp"
#include <stdlib.h>

#define DEF_CMD(name, numCmd, ...)    \
    CMD_##name = numCmd,

typedef enum {
    BAD_CMD = 0,
    #include "../include/cmd.hpp"
    CMD_MOV,
    CMD_TRASH
} COMMANDS;

#undef DEF_CMD

typedef enum argument {
    NUMBER      = 1,
    REGISTER    = 2,
    NUM_REG     = 3,
    REG_NUM     = 4, 
    MEM_NUM     = 5,
    MEM_REG     = 6,
    MEM_NUM_REG = 7,
    MEM_REG_NUM = 8,
    LABEL       = 9
} argument_t;

typedef struct command {

    const char* name;
    int         cmd;
    int         nativeSize;
    size_t      nativeIP;
    size_t      x86ip;
    argument_t  argument_type;
    char        reg_type;
    int64_t     argument;

} ir_t;

typedef struct irInfo {

    ir_t * irArray;
    size_t sizeArray;

} irInfo_t;

typedef struct JIT_CompilerInfo {

    code_t byteCode;
    x86code_t machineCode;

    irInfo_t irInfo;

    char * x86_memory_buf;

} compilerInfo_t;

void createIRArray  (compilerInfo_t * compilerInfo);
void fillIRArray    (compilerInfo_t * compilerInfo);

#endif