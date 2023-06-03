#include "../include/optimizer.hpp"
#include "../include/translateCommand.hpp"

#define MATH_COMMAND(operator)                                              \
    irArr[i].argument = irArr[i].argument operator irArr[i+1].argument;     \
    irArr[i+1].cmd = irArr[i+2].cmd = CMD_TRASH;

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
            irArr[i].x86size        = SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM;
            irArr[i+1].cmd          = CMD_TRASH;
            irArr[i+1].name         = "trash";
            i++;
        }

        if ((irArr[i].cmd   == CMD_PUSH && irArr[i].argument_type   == NUMBER) &&
            (irArr[i+1].cmd == CMD_PUSH && irArr[i+1].argument_type == NUMBER))
        {
            switch (irArr[i+2].cmd)
            {
                case CMD_ADD:
                    irArr[i].argument = irArr[i].argument + irArr[i+1].argument;
                    irArr[i+1].cmd = irArr[i+2].cmd   = CMD_TRASH;
                    irArr[i+1].name = irArr[i+2].name = "trash";
                    break;

                case CMD_SUB:
                    irArr[i].argument = irArr[i].argument - irArr[i+1].argument;  
                    irArr[i+1].cmd = irArr[i+2].cmd   = CMD_TRASH;
                    irArr[i+1].name = irArr[i+2].name = "trash";
                
                case CMD_MUL:
                    irArr[i].argument = (int64_t) ((((double)irArr[i].argument/1000) * ((double)irArr[i+1].argument/1000))*1000);  
                    irArr[i+1].cmd = irArr[i+2].cmd   = CMD_TRASH;
                    irArr[i+1].name = irArr[i+2].name = "trash";
                
                case CMD_DIV:
                    irArr[i].argument = (int64_t) (((double)irArr[i].argument / (double)irArr[i+1].argument)*1000);  
                    irArr[i+1].cmd = irArr[i+2].cmd   = CMD_TRASH;
                    irArr[i+1].name = irArr[i+2].name = "trash";
                    break;
                
                default:
                    break;
            }
        }
    }
    return ;
}