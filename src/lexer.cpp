#include "../include/lexer.hpp"
#include "../include/translateCommand.hpp"

const int MASK = (1<<4) + (1<<3) + (1<<2) + (1<<1) + 1; //0001|1111

const int RAM = 7; //0000 | 0001  -> 1000 | 0000
const int REG = 6; //0000 | 0001  -> 0100 | 0000
const int NUM = 5; //0000 | 0001  -> 0010 | 0000

const int NUM_REGISTERS = 4;  

const size_t MAX_RAM = 100;

#define JUMP_FORM(command)                              \
    compilerInfo->irInfo.irArray[numCmds] = {                   \
        .name           = #command,                     \
        .cmd            = CMD_##command,                \
        .nativeSize     = 2,                            \
        .nativeIP       = i-1,                          \
        .argument_type  = LABEL,                        \
        .argument       = compilerInfo->byteCode.buf[i]  \
    };

#define BOOL_EXPR(command)                          \
    compilerInfo->irInfo.irArray[numCmds] = {               \
        .name           = #command,                 \
        .cmd            = CMD_##command,            \
        .nativeSize     = 2,                        \
        .nativeIP       = i,                        \
        .argument_type  = LABEL                     \
    };

static int checkBit(const int value, const int position);
static int * createArrRegs (size_t numRegs);

void createIRArray (compilerInfo_t * compilerInfo)
{
    compilerInfo->irInfo.irArray = (ir_t *) calloc (compilerInfo->byteCode.sizeBuf/sizeof(int), sizeof(ir_t));
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "Unable to allocate memory");
}

void fillIRArray (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "There is no access to the array with code");

    int * regs = createArrRegs (NUM_REGISTERS);
    int cmd = -1;

    size_t numCmds  = 0;
    size_t x86ip    = 0;

    #define DEF_CMD(nameCmd, numCmd, isArg, ...)            \
    if (cmd == CMD_##nameCmd)                               \
        __VA_ARGS__                                         \
    else

    for (size_t i = 0; ; i++, numCmds++)
    {
        cmd = (compilerInfo->byteCode.buf[i] & MASK);

        if ( (checkBit(compilerInfo->byteCode.buf[i], NUM) == 1) && 
             (checkBit(compilerInfo->byteCode.buf[i], REG) == 0) && 
             (checkBit(compilerInfo->byteCode.buf[i], RAM) == 0)) //push 7 / pop
        {
            i++;
            if (cmd == CMD_PUSH)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "push_num",
                    .cmd            = CMD_PUSH,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = NUMBER,
                    .argument       = compilerInfo->byteCode.buf[i]
                };
            }
            else if (cmd == CMD_POP)
            {
                // i++;
                // compilerInfo->irInfo.irArray[numCmds] = {
                //     .name           = "pop_empty",
                //     .cmd            = CMD_POP,
                //     .nativeSize     = 2,
                //     .nativeIP       = i-1,
                //     .argument_type  = NUMBER,
                //     .argument       = compilerInfo->byteCode.buf[i]
                // };
                MY_ASSERT (1, "Incorrect command pop num")
            }
            else
            {
                MY_ASSERT (1, "Wrong command (section pushORpop_num)");
            }
            x86ip += SIZE_MOV_REG_IMMED + SIZE_NUM + SIZE_PUSH_REG;
        }
        else if ((checkBit(compilerInfo->byteCode.buf[i], NUM) == 0) && 
                 (checkBit(compilerInfo->byteCode.buf[i], REG) == 1) && 
                 (checkBit(compilerInfo->byteCode.buf[i], RAM) == 0)) //push/pop rax
        {     
            i++;
            char nReg = compilerInfo->byteCode.buf[i];    
            MY_ASSERT (nReg > NUM_REGISTERS-1, "You are out of register memory");

            if (cmd == CMD_PUSH)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "push_reg",
                    .cmd            = CMD_PUSH,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = REGISTER,
                    .reg_type       = nReg
                };
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "pop_reg",
                    .cmd            = CMD_POP,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = REGISTER,
                    .reg_type       = nReg
                };
            }
            else
            {
                MY_ASSERT (1, "Wrong command");
            }
            x86ip += SIZE_PUSH_REG;
        }
        else if ((cmd == CMD_PUSH) && 
                 (checkBit(compilerInfo->byteCode.buf[i], NUM) == 1) && 
                 (checkBit(compilerInfo->byteCode.buf[i], REG) == 1) && 
                 (checkBit(compilerInfo->byteCode.buf[i], RAM) == 0)) //push 5 + rax
        {
            i++;
            char nReg = compilerInfo->byteCode.buf[i];
            printf ("nReg = %d, i = %zu\n", nReg, i);
            MY_ASSERT (nReg > NUM_REGISTERS-1, "You are out of register memory");
            i++;
            int numIndex = compilerInfo->byteCode.buf[i];
            compilerInfo->irInfo.irArray[numCmds] = {
                .name           = "push_num_reg",
                .cmd            = CMD_PUSH,
                .nativeSize     = 3, 
                .nativeIP       = i-2,
                .x86ip          = x86ip,
                .argument_type  = NUM_REG,
                .reg_type       = nReg,
                .argument       = numIndex
            };
            x86ip += SIZE_MOV_REG_IMMED + SIZE_NUM + SIZE_ADD_REG_REG + SIZE_PUSH_REG;
        }
        else if ((checkBit(compilerInfo->byteCode.buf[i], NUM) == 1) && 
                 (checkBit(compilerInfo->byteCode.buf[i], REG) == 0) && 
                 (checkBit(compilerInfo->byteCode.buf[i], RAM) == 1)) // push/pop[10]
        {
            i++;
            int ramIndex = compilerInfo->byteCode.buf[i];
            MY_ASSERT (ramIndex > MAX_RAM-1, "You are out of RAM");
            MY_ASSERT (ramIndex < 0, "ramIndex < 0")

            if (cmd == CMD_PUSH)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "push_num",
                    .cmd            = CMD_PUSH,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = MEM_NUM,
                    .argument       = ramIndex
                }; 
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "pop_num",
                    .cmd            = CMD_POP,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = MEM_NUM,
                    .argument       = ramIndex
                }; 
            }
            else 
            {
                MY_ASSERT (1, "Wrong command");
            }
            x86ip += SIZE_PUSH_R15_OFFSET + SIZE_REL_PTR;
        }
        else if ((checkBit(compilerInfo->byteCode.buf[i], NUM) == 0) && 
                 (checkBit(compilerInfo->byteCode.buf[i], REG) == 1) && 
                 (checkBit(compilerInfo->byteCode.buf[i], RAM) == 1)) //push/pop[rcx]
        {
            i++;
            char nReg = compilerInfo->byteCode.buf[i];
            MY_ASSERT (nReg > NUM_REGISTERS-1, "You are out of register memory");

            if (cmd == CMD_PUSH)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "push_reg",
                    .cmd            = CMD_PUSH,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = MEM_REG,
                    .reg_type       = nReg
                };
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds]  = {
                    .name           = "pop_reg",
                    .cmd            = CMD_POP,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .x86ip          = x86ip,
                    .argument_type  = MEM_REG,
                    .reg_type       = nReg
                }; 
            }
            else 
            {
                MY_ASSERT (1, "Wrong command");
            }
            x86ip += SIZE_PUSH_REG + SIZE_ADD_REG_REG + SIZE_PUSH_R15_OFFSET + SIZE_REL_PTR + SIZE_POP_REG;
        }
        else if ((checkBit(compilerInfo->byteCode.buf[i], NUM) == 1) && 
                (checkBit(compilerInfo->byteCode.buf[i], REG) == 1) && 
                (checkBit(compilerInfo->byteCode.buf[i], RAM) == 1)) //push/pop [5 + rcx]
        {
            i++;
            char nReg = compilerInfo->byteCode.buf[i];
            MY_ASSERT (nReg > NUM_REGISTERS-1, "You are out of register memory");
            i++;
            int ramIndex = compilerInfo->byteCode.buf[i];
            MY_ASSERT ((size_t) ramIndex > MAX_RAM-1, "You are out of RAM");

            if (cmd == CMD_PUSH)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "push_mem_num_reg",
                    .cmd            = CMD_PUSH,
                    .nativeSize     = 3,
                    .nativeIP       = i-2,
                    .x86ip          = x86ip,
                    .argument_type  = MEM_NUM_REG,
                    .reg_type       = nReg,
                    .argument       = ramIndex
                };
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds]  = {
                    .name           = "pop_mem_num_reg",
                    .cmd            = CMD_POP,
                    .nativeSize     = 3,
                    .nativeIP       = i-2,
                    .x86ip          = x86ip,
                    .argument_type  = MEM_NUM_REG,
                    .reg_type       = nReg,
                    .argument       = ramIndex
                };
            }
            else 
            {
                MY_ASSERT (1, "Wrong command");
            }
            x86ip += SIZE_PUSH_REG + SIZE_ADD_REG_REG + SIZE_POP_R15_OFFSET + SIZE_REL_PTR + SIZE_POP_REG;
        }
        else


        #include "../include/cmd.hpp"
        
    
        {
            printf ("cmd = %d, i = %zu, prevCmd = %s\n", cmd, i, compilerInfo->irInfo.irArray[i-2].name);
            break;
        }
    }

    free (regs);

    compilerInfo->irInfo.sizeArray = numCmds;

    return ;
}

static int checkBit(const int value, const int position) 
{
    return ((value & (1 << position)) != 0);
}

static int * createArrRegs (size_t numRegs)
{
    int * regs = (int *) calloc (numRegs, sizeof(int));
    MY_ASSERT (regs == nullptr, "Unable to allocate new memory");

    return regs;
}


