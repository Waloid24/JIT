#ifndef DUMP_IR_HPP
#define DUMP_IR_HPP

#include <stdio.h>
#include "lexer.hpp"

void graphvizDumpIR (compilerInfo_t compilerInfo);
void dumpx86MachineCode (compilerInfo_t compilerInfo);
void dumpCode       (compilerInfo_t * compilerInfo);

#endif