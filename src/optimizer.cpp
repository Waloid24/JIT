#include "../include/optimizer.hpp"

void optimizeIR (compilerInfo_t * compilerInfo)
{
    size_t sizeArr  = compilerInfo->irInfo.sizeArray;
    ir_t * irArr    = compilerInfo->irInfo.irArray;

    for (size_t i = 0; i < sizeArr; i++)
    {
        if ((irArr[i].cmd   == CMD_PUSH && irArr[i].argument_type   == NUMBER  ) && 
            (irArr[i+1].cmd == CMD_POP  && irArr[i+1].argument_type == REGISTER))
        {
            irArr[i].cmd            = CMD_MOV;
            irArr[i].argument_type  = REG_NUM;
            irArr[i].reg_type       = irArr[i+1].reg_type;
            irArr[i].name           = "mov";
            irArr[i+1].cmd          = CMD_TRASH;
            irArr[i+1].name         = "trash";
            i++;
        }
    }
    return ;
}