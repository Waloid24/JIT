#include "../include/lexer.hpp"
#include "../include/translateCommand.hpp"
#include "../include/dumpIr.hpp"

const int MASK = (1<<4) + (1<<3) + (1<<2) + (1<<1) + 1; //0001|1111

const int RAM = 7; //0000 | 0001  -> 1000 | 0000
const int REG = 6; //0000 | 0001  -> 0100 | 0000
const int NUM = 5; //0000 | 0001  -> 0010 | 0000

const int NUM_REGISTERS = 4;  
const size_t SIZE_x86_BUF = 1000;

const size_t MAX_RAM = 100;
const int NOT_PTR = -1;

#define JUMP_FORM(command)                              \
    compilerInfo->irInfo.irArray[numCmds] = {           \
        .name           = #command,                     \
        .cmd            = CMD_##command,                \
        .nativeSize     = 2,                            \
        .nativeIP       = i-1,                          \
        .argument_type  = LABEL,                        \
        .argument       = compilerInfo->byteCode.buf[i], \
        .x86size        = SIZE_POP_REG + SIZE_POP_REG + SIZE_CMP_REG_REG +   \
                            SIZE_COND_JMP + SIZE_REL_PTR                 \
    };

#define BOOL_EXPR(command)                          \
    compilerInfo->irInfo.irArray[numCmds] = {       \
        .name           = #command,                 \
        .cmd            = CMD_##command,            \
        .nativeSize     = 2,                        \
        .nativeIP       = i,                        \
        .argument_type  = LABEL,                    \
        .x86size        = SIZE_POP_REG + SIZE_POP_REG + SIZE_CMP_REG_REG +              \
                            SIZE_COND_JMP + SIZE_REL_PTR + SIZE_MOV_REG_IMMED +     \
                            SIZE_8BYTE_NUM + SIZE_PUSH_REG + SIZE_JMP + SIZE_REL_PTR +    \
                            SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM + SIZE_PUSH_REG               \
    };

static int checkBit         (const int value, const int position);
static int * createArrRegs  (size_t numRegs);
static bool isJump          (ir_t irCommand);
static int64_t findJmpx86Ip (compilerInfo_t * compilerInfo, ir_t * irCommand);
static void fillCallJumpArg (compilerInfo_t * compilerInfo);

void createIRArray (compilerInfo_t * compilerInfo)
{
    compilerInfo->irInfo.irArray = (ir_t *) calloc (compilerInfo->byteCode.sizeBuf/sizeof(int), sizeof(ir_t));
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "Unable to allocate memory");
}

void JITConstructor (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure")

    compilerInfo->machineCode.buf = (char *) aligned_alloc (PAGE_SIZE, 4096*sizeof(char));
    MY_ASSERT (compilerInfo->machineCode.buf == nullptr, "Unable to allocate new memory")
    memset ((void*) compilerInfo->machineCode.buf, 0, 4096*sizeof(char));

    compilerInfo->x86_memory_buf = (char *) aligned_alloc (MEMORY_ALIGNMENT, MEMORY_ALIGNMENT*sizeof(char));
    MY_ASSERT (compilerInfo->x86_memory_buf == nullptr, "Unable to allocate new memory")
    memset ((void*) compilerInfo->x86_memory_buf, 0, MEMORY_ALIGNMENT*sizeof(char));

    compilerInfo->x86StackBuf = (char *) aligned_alloc (MEMORY_ALIGNMENT, MEMORY_ALIGNMENT*sizeof(char));
    MY_ASSERT (compilerInfo->x86StackBuf == nullptr, "Unable to allocate new memory")
    memset ((void*) compilerInfo->x86StackBuf, 0, MEMORY_ALIGNMENT*sizeof(char));
}

void JITDestructor (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure")
    free (compilerInfo->x86_memory_buf);
    free (compilerInfo->machineCode.buf);
    free (compilerInfo->byteCode.buf);
    free (compilerInfo->irInfo.irArray);
}

static bool isJump (ir_t irCommand)
{
    if (irCommand.cmd == CMD_JMP || irCommand.cmd == CMD_JE || 
        irCommand.cmd == CMD_JBE || irCommand.cmd == CMD_JB || 
        irCommand.cmd == CMD_JGE || irCommand.cmd == CMD_JA ||
        irCommand.cmd == CMD_JMP || irCommand.cmd == CMD_JNE)
    {
        return true;
    }
    return false;
}

void fillJmpsCalls (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure")

    for (size_t i = 0; i < compilerInfo->irInfo.sizeArray; i++)
    {
        if (isJump (compilerInfo->irInfo.irArray[i]) || compilerInfo->irInfo.irArray[i].cmd == CMD_CALL)
        {
            compilerInfo->irInfo.irArray[i].argument = findJmpx86Ip (compilerInfo, &(compilerInfo->irInfo.irArray[i]));
            // graphvizDumpIR (*compilerInfo);
        }
    }
}

static int64_t findJmpx86Ip (compilerInfo_t * compilerInfo, ir_t * irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure")

    for (size_t i = 0; i < compilerInfo->irInfo.sizeArray; i++)
    {
        if (compilerInfo->irInfo.irArray[i].nativeIP == irCommand->argument)
        {
            return compilerInfo->irInfo.irArray[i].x86ip;
        }
    }

    return NOT_PTR;
}

void setIp (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure")

    size_t x86ip = SIZE_MOV_RNUM_IMMED + sizeof(u_int64_t) + SIZE_MOV_RNUM_IMMED + sizeof (u_int64_t);;

    for (size_t i = 0; i < compilerInfo->irInfo.sizeArray; i++)
    {
        if (compilerInfo->irInfo.irArray[i].cmd != CMD_TRASH)
        {
            compilerInfo->irInfo.irArray[i].x86ip = x86ip;
            x86ip += compilerInfo->irInfo.irArray[i].x86size;
        }
        
    }
}

void fillIRArray (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "There is no access to the array with code");

    int * regs = createArrRegs (NUM_REGISTERS);
    int cmd = -1;

    size_t numCmds  = 0;

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
                    .argument_type  = NUMBER,
                    .argument       = compilerInfo->byteCode.buf[i],
                    .x86size        = SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM + SIZE_PUSH_REG
                };
            }
            else
            {
                MY_ASSERT (1, "Wrong command (section push_num)");
            }
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
                    .argument_type  = REGISTER,
                    .reg_type       = nReg,
                    .x86size        = SIZE_PUSH_REG
                };
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "pop_reg",
                    .cmd            = CMD_POP,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .argument_type  = REGISTER,
                    .reg_type       = nReg,
                    .x86size        = SIZE_POP_REG
                };
            }
            else
            {
                MY_ASSERT (1, "Wrong command");
            }
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
                .argument_type  = NUM_REG,
                .reg_type       = nReg,
                .argument       = numIndex,
                .x86size        = SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM + SIZE_ADD_REG_REG + SIZE_PUSH_REG
            };
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
                    .argument_type  = MEM_NUM,
                    .argument       = ramIndex,
                    .x86size        = SIZE_PUSH_R15_OFFSET + SIZE_REL_PTR
                }; 
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds] = {
                    .name           = "pop_num",
                    .cmd            = CMD_POP,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .argument_type  = MEM_NUM,
                    .argument       = ramIndex,
                    .x86size        = SIZE_PUSH_R15_OFFSET + SIZE_REL_PTR
                }; 
            }
            else 
            {
                MY_ASSERT (1, "Wrong command");
            }
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
                    .argument_type  = MEM_REG,
                    .reg_type       = nReg,
                    .x86size        = SIZE_MOV_RNUM_RNUM + SIZE_ADD_RNUM_REG + 
                                        SIZE_PUSH_R15_OFFSET + SIZE_REL_PTR + SIZE_MOV_RNUM_RNUM    
                };
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds]  = {
                    .name           = "pop_reg",
                    .cmd            = CMD_POP,
                    .nativeSize     = 2,
                    .nativeIP       = i-1,
                    .argument_type  = MEM_REG,
                    .reg_type       = nReg,
                    .x86size        = SIZE_MOV_RNUM_RNUM + SIZE_ADD_RNUM_REG + 
                                        SIZE_PUSH_R15_OFFSET + SIZE_REL_PTR + SIZE_MOV_RNUM_RNUM    
                }; 
            }
            else 
            {
                MY_ASSERT (1, "Wrong command");
            }
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
                    .argument_type  = MEM_NUM_REG,
                    .reg_type       = nReg,
                    .argument       = ramIndex*8,
                    .x86size        = SIZE_MOV_RNUM_RNUM + SIZE_ADD_REG_REG + 
                                        SIZE_POP_R15_OFFSET + SIZE_REL_PTR + SIZE_MOV_RNUM_RNUM
                };
            }
            else if (cmd == CMD_POP)
            {
                compilerInfo->irInfo.irArray[numCmds]  = {
                    .name           = "pop_mem_num_reg",
                    .cmd            = CMD_POP,
                    .nativeSize     = 3,
                    .nativeIP       = i-2,
                    .argument_type  = MEM_NUM_REG,
                    .reg_type       = nReg,
                    .argument       = ramIndex*8,
                    .x86size        = SIZE_MOV_RNUM_RNUM + SIZE_ADD_REG_REG + 
                                        SIZE_POP_R15_OFFSET + SIZE_REL_PTR + SIZE_MOV_RNUM_RNUM
                };
            }
            else 
            {
                MY_ASSERT (1, "Wrong command");
            }
        }
        else


        #include "../include/cmd.hpp"
        
    
        {
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


