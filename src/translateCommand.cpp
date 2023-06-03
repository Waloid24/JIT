#include "../include/translateCommand.hpp"

static void x86TranslatePushPop     (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateSimpleMath  (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateMov         (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateSqrt        (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateComp        (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateJmpCall     (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateIn          (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateOut         (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateRet         (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateHlt         (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateCondJmps    (compilerInfo_t * compilerInfo, ir_t irCommand);

static void insert8ByteSignNum      (compilerInfo_t * compilerInfo, int64_t number);
static void insert8ByteUnsignNum    (compilerInfo_t * compilerInfo, u_int64_t ptr);
static void insert4ByteSignNum      (compilerInfo_t * compilerInfo, int32_t ptr);
static void insert4ByteUnsignNum    (compilerInfo_t * compilerInfo, u_int32_t ptr);

static void insertCmd               (compilerInfo_t * compilerInfo, opcode_t cmd);
static void myPrintf                (double num);
static void myScanf                 (int * num);

static void dumpCmd                 (compilerInfo_t * compilerInfo, opcode_t cmdCode, const char * name, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpNum                 (compilerInfo_t * compilerInfo, int64_t number, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpAbsPtr              (compilerInfo_t * compilerInfo, u_int64_t number, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpRelPtr              (compilerInfo_t * compilerInfo, u_int32_t number, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpFunc                (compilerInfo_t * compilerInfo, const char * message, size_t lineSrcFile, const char * nameCallingFunc);

const int64_t BAD_NUMBER            = 123456;
const u_int32_t NUMBER_OF_FUNCTION  = 100;

#define x86dumpFunc(compilerInfo, message)                                              \
    dumpFunc (compilerInfo, message, __LINE__, __PRETTY_FUNCTION__);

#define x86dumpCmd(compilerInfo, cmdOpcode, nameCmd)                                    \
    dumpCmd (compilerInfo, cmdOpcode, #nameCmd, __LINE__, __PRETTY_FUNCTION__);  

#define x86dumpNum(compilerInfo, number)                                                \
    dumpNum (compilerInfo, number, __LINE__, __PRETTY_FUNCTION__);  

#define x86dumpAbsPtr(compilerInfo, pointer)                                            \
    dumpAbsPtr (compilerInfo, pointer, __LINE__, __PRETTY_FUNCTION__);  

#define x86dumpRelPtr(compilerInfo, pointer)                                            \
    dumpRelPtr (compilerInfo, pointer, __LINE__, __PRETTY_FUNCTION__);  

#define x86insertCmd(compilerInfo, cmd)                                                 \
    insertCmd (compilerInfo, cmd);                                                      \
    x86dumpCmd (compilerInfo, cmd, #cmd)

#define x86insert8ByteSignNum(compilerInfo, num)                                        \
    insert8ByteSignNum (compilerInfo, num);                                             \
    x86dumpAbsPtr (compilerInfo, num)

#define x86insert8ByteUnsignNum(compilerInfo, num)                                      \
    insert8ByteUnsignNum (compilerInfo, num);                                           \
    x86dumpAbsPtr (compilerInfo, num)

#define x86insert4ByteSignNum(compilerInfo, num)                                        \
    insert4ByteSignNum (compilerInfo, num);                                             \
    x86dumpRelPtr (compilerInfo, num)

#define x86insert4ByteUnsignNum(compilerInfo, num)                                      \
    insert4ByteUnsignNum (compilerInfo, num);                                           \
    x86dumpRelPtr (compilerInfo, num)

#define logDumpline(message, ...)                                                       \
    fprintf (logfile, message, ##__VA_ARGS__);

//===========================New format with getOffsetCmd function=====================

static u_int64_t getOffsetCmd ( u_int64_t cmd, u_int64_t reg, u_int64_t offset)
{
    return cmd | (reg << offset);
}

static void emitCmd (compilerInfo_t * compilerInfo, u_int64_t cmd, u_int64_t cmd_size)
{
    opcode_t code = {.size = cmd_size, .code = cmd};
    insertCmd (compilerInfo, code);
    x86dumpCmd (compilerInfo, code, cmd)
}

#define EMIT_CVTSD2SI_REG(reg1, reg2)      \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (CVTSD2SI_REG, reg1, 35), reg2, 32), SIZE_CVTSD2SI);

#define EMIT_CVTSI2SD_REG(reg1, reg2)      \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (CVTSI2SD_REG, reg1, 35), reg2, 32), SIZE_CVTSI2SD);

#define EMIT_PUSH_REG(reg)              \
    emitCmd (compilerInfo, getOffsetCmd (PUSH_REG, reg, 0), SIZE_PUSH_REG);

#define EMIT_PUSH_RNUM(reg)             \
    emitCmd (compilerInfo, getOffsetCmd (PUSH_RNUM, reg, 8), SIZE_PUSH_RNUM);

#define EMIT_POP_REG(reg)              \
    emitCmd (compilerInfo, getOffsetCmd (POP_REG, reg, 0), SIZE_POP_REG);

#define EMIT_POP_RNUM(reg)             \
    emitCmd (compilerInfo, getOffsetCmd (POP_RNUM, reg, 8), SIZE_POP_RNUM);

#define EMIT_SUB_REG_REG(reg1, reg2)            \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (SUB_REG_REG, reg1, 16), reg2, 19), SIZE_SUB_REG_REG);

#define EMIT_SUB_RNUM_REG(reg1, reg2)            \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (SUB_RNUM_REG, reg1, 16), reg2, 19), SIZE_SUB_RNUM_REG);

#define EMIT_SUB_REG_RNUM(reg1, reg2)            \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (SUB_REG_RNUM, reg1, 16), reg2, 19), SIZE_SUB_REG_RNUM);

#define EMIT_SUB_RNUM_RNUM(reg1, reg2)            \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (SUB_RNUM_RNUM, reg1, 16), reg2, 19), SIZE_SUB_RNUM_RNUM);

#define EMIT_SUB_REG_IMMED(reg, immed)                                                              \
    emitCmd (compilerInfo, getOffsetCmd (SUB_REG_IMMED, reg, 16), SIZE_SUB_REG_IMMED);              \
    x86insert4ByteUnsignNum    (compilerInfo, immed);

#define EMIT_SUB_RNUM_IMMED(reg, immed)                                                             \
    emitCmd (compilerInfo, getOffsetCmd (SUB_RNUM_IMMED, reg, 16), SIZE_SUB_RNUM_IMMED);            \
    x86insert4ByteUnsignNum    (compilerInfo, immed);

#define EMIT_ADD_REG_IMMED(reg, immed)                                                              \
    emitCmd (compilerInfo, getOffsetCmd (ADD_REG_IMMED, reg, 16), SIZE_ADD_REG_IMMED);              \
    x86insert4ByteUnsignNum    (compilerInfo, immed);

#define EMIT_ADD_RNUM_IMMED(reg, immed)                                                             \
    emitCmd (compilerInfo, getOffsetCmd (ADD_RNUM_IMMED, reg, 16), SIZE_ADD_RNUM_IMMED);            \
    x86insert4ByteUnsignNum    (compilerInfo, immed);

#define EMIT_MOV_RNUM_IMMED(reg, immed)                                                             \
    emitCmd         (compilerInfo, getOffsetCmd (MOV_RNUM_IMMED, reg, 8), SIZE_MOV_RNUM_IMMED);     \
    x86insert8ByteUnsignNum (compilerInfo, immed);

#define EMIT_MOV_REG_IMMED(reg, immed)                                                              \
    emitCmd (compilerInfo, getOffsetCmd (MOV_REG_IMMED, reg, 8), SIZE_MOV_REG_IMMED);               \
    x86insert8ByteUnsignNum (compilerInfo, immed);

#define EMIT_ADD_REG_REG(reg1, reg2)    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (ADD_REG_REG, reg1, 16), reg2, 19), SIZE_ADD_REG_REG);

#define EMIT_ADD_RNUM_REG(reg1, reg2)    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (ADD_RNUM_REG, reg1, 16), reg2, 19), SIZE_ADD_RNUM_REG);

#define EMIT_ADD_REG_RNUM(reg1, reg2)    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (ADD_REG_RNUM, reg1, 16), reg2, 19), SIZE_ADD_REG_RNUM);

#define EMIT_ADD_RNUM_RNUM(reg1, reg2)    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (ADD_RNUM_RNUM, reg1, 16), reg2, 19), SIZE_ADD_RNUM_RNUM);

#define EMIT_MOV_REG_REG(reg1, reg2)                                                                \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (MOV_REG_REG, reg1, 16), reg2, 19), SIZE_MOV_REG_REG);

#define EMIT_MOV_RNUM_REG(reg1, reg2)                                                                \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (MOV_RNUM_REG, reg1, 16), reg2, 19), SIZE_MOV_RNUM_REG);

#define EMIT_MOV_REG_RNUM(reg1, reg2)                                                                \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (MOV_REG_RNUM, reg1, 16), reg2, 19), SIZE_MOV_REG_RNUM);

#define EMIT_MOV_RNUM_RNUM(reg1, reg2)                                                                \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (MOV_RNUM_RNUM, reg1, 16), reg2, 19), SIZE_MOV_RNUM_RNUM);

#define EMIT_JMP_REL_PTR(relPtr)                                                                        \
    emitCmd (compilerInfo, JMP, SIZE_JMP);                                                      \
    x86insert4ByteSignNum (compilerInfo, relPtr)

#define EMIT_CALL_REL_PTR(relPtr)                                                                       \
    emitCmd (compilerInfo, CALL, SIZE_CALL);                                                    \
    x86insert4ByteSignNum (compilerInfo, relPtr)

#define EMIT_CMP_REG_REG(reg1, reg2)                                                                    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (CMP_REG_REG, reg1, 16), reg2, 19), SIZE_CMP_REG_REG);

#define EMIT_CMP_REG_RNUM(reg1, reg2)                                                                    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (CMP_REG_RNUM, reg1, 16), reg2, 19), SIZE_CMP_REG_RNUM);
    
#define EMIT_CMP_RNUM_REG(reg1, reg2)                                                                    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (CMP_RNUM_REG, reg1, 16), reg2, 19), SIZE_CMP_RNUM_REG);

#define EMIT_CMP_RNUM_RNUM(reg1, reg2)                                                                    \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (CMP_RNUM_RNUM, reg1, 16), reg2, 19), SIZE_CMP_RNUM_RNUM);

#define EMIT_MULPD(reg1, reg2)                                                                            \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (MULPD, reg1, 27), reg2, 24), SIZE_MULPD);

#define EMIT_DIVPD(reg1, reg2)                                                                            \
    emitCmd (compilerInfo, getOffsetCmd (getOffsetCmd (DIVPD, reg1, 27), reg2, 24), SIZE_DIVPD);

#define EMIT_CMD(cmd)                                                                                       \
    emitCmd (compilerInfo, cmd, SIZE_##cmd);

//======================================================================================================================


void JITCompile (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "There is no access to commands array");
    MY_ASSERT (compilerInfo->irInfo.irArray[0].cmd == CMD_POP, "First command is pop, but stack is empty");

    size_t sizeArr = compilerInfo->irInfo.sizeArray;
    ir_t * irArr   = compilerInfo->irInfo.irArray;

    FILE * logfile = openFile ("./logs/logTranslation.txt", "w");           // clear old information
    fclose (logfile);

    EMIT_MOV_RNUM_IMMED (R15, (u_int64_t) compilerInfo->x86_memory_buf)
    EMIT_MOV_RNUM_IMMED (R14, (u_int64_t) compilerInfo->x86StackBuf)

    size_t irIndex = 0;
    size_t machineIndex = 0;

    for (; irIndex < sizeArr; irIndex++, machineIndex++)
    {
        switch (irArr[irIndex].cmd)
        {
            case CMD_PUSH:
            case CMD_POP:
                x86TranslatePushPop (compilerInfo, irArr[irIndex]);
                break;

            case CMD_JMP:
            case CMD_CALL:
                x86TranslateJmpCall (compilerInfo, irArr[irIndex]);
                break; 

            case CMD_ADD:
            case CMD_SUB:
            case CMD_MUL:
            case CMD_DIV:
                x86TranslateSimpleMath (compilerInfo, irArr[irIndex]);
                break;

            case CMD_JE:
            case CMD_JBE:
            case CMD_JGE:
            case CMD_JA:
            case CMD_JB:
            case CMD_JNE:
                x86TranslateCondJmps (compilerInfo, irArr[irIndex]);
                break;

            case CMD_BA:
            case CMD_BE:
            case CMD_BGE:
            case CMD_BNE:
            case CMD_BB:
            case CMD_BBE:
                x86TranslateComp (compilerInfo, irArr[irIndex]);
                break;

            case CMD_SQRT:
                x86TranslateSqrt (compilerInfo, irArr[irIndex]);
                break;

            case CMD_HLT:
                x86TranslateHlt (compilerInfo, irArr[irIndex]);
                break;

            case CMD_IN:
                x86TranslateIn (compilerInfo, irArr[irIndex]);
                break;

            case CMD_OUT:
                x86TranslateOut (compilerInfo, irArr[irIndex]);
                break;

            case CMD_RET:
                x86TranslateRet (compilerInfo, irArr[irIndex]);
                break;

            case CMD_TRASH:
                break;

            case CMD_MOV:
                x86TranslateMov (compilerInfo, irArr[irIndex]);
                break;

            default:
                printf ("incorrect command type: %d\n", irArr[irIndex].cmd);
                MY_ASSERT (1, "Incorrect command type");
                break;
        }
    }
}

static void myScanf (int * num)
{
    printf ("Enter an integer number: ");
    scanf ("%d", num);
    *num *= 1000;
}

static void myPrintf (double num)
{
    printf ("OUT: %.3lf\n", num/1000);
}

static void x86TranslateMov (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")
    EMIT_MOV_REG_IMMED (irCommand.reg_type, irCommand.argument)
}

static void x86TranslateOut (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    x86dumpFunc (compilerInfo, "Translate [OUT]")

    size_t addrToContinue = irCommand.x86ip + SIZE_CVTSI2SD + SIZE_PUSH_REG + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + 
             SIZE_MOV_RNUM_REG + SIZE_ALIGN_STACK + SIZE_CALL + SIZE_REL_PTR;

    int32_t relAddrPrintf = (u_int64_t)myPrintf - (u_int64_t)(compilerInfo->machineCode.buf + addrToContinue);

    EMIT_CMD                (CVTSI2SD_XMM0_RSP)

    EMIT_PUSH_REG           (RBX)
    EMIT_PUSH_RNUM          (R15)
    EMIT_PUSH_RNUM          (R14)
    EMIT_PUSH_RNUM          (R12)

    EMIT_MOV_RNUM_REG       (R12, RSP)
    EMIT_CMD                (ALIGN_STACK)

    EMIT_CALL_REL_PTR       (relAddrPrintf)

    EMIT_MOV_REG_RNUM       (RSP, R12)

    EMIT_POP_RNUM           (R12)
    EMIT_POP_RNUM           (R14)
    EMIT_POP_RNUM           (R15)
    EMIT_POP_REG            (RBX)

    EMIT_ADD_REG_IMMED      (RSP, 8)

}

static void x86TranslateIn (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    x86dumpFunc (compilerInfo, "Translate [IN]")

    size_t addrToContinue = irCommand.x86ip + SIZE_SUB_REG_IMMED + SIZE_4BYTE_NUM + SIZE_MOV_REG_REG + SIZE_PUSH_REG + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_MOV_RNUM_REG +
                            SIZE_ALIGN_STACK + SIZE_CALL + SIZE_REL_PTR;

    int32_t relAddr = (u_int64_t)myScanf - (u_int64_t)(compilerInfo->machineCode.buf + addrToContinue);

    EMIT_SUB_REG_IMMED      (RSP, 8)
    
    EMIT_MOV_REG_REG        (RDI, RSP)

    EMIT_PUSH_REG           (RBX)
    EMIT_PUSH_RNUM          (R15)
    EMIT_PUSH_RNUM          (R14)
    EMIT_PUSH_RNUM          (R12)

    EMIT_MOV_RNUM_REG       (R12, RSP)
    EMIT_CMD                (ALIGN_STACK)

    EMIT_CALL_REL_PTR       (relAddr)

    EMIT_MOV_REG_RNUM       (RSP, R12)
    EMIT_POP_RNUM           (R12)
    EMIT_POP_RNUM           (R14)
    EMIT_POP_RNUM           (R15)
    EMIT_POP_REG            (RBX)
}

static void x86TranslatePushPop (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.argument_type)
    {
        case NUMBER:
        {
            EMIT_MOV_REG_IMMED          (irCommand.reg_type, irCommand.argument)                             // mov rax, num
            EMIT_PUSH_REG               (irCommand.reg_type)                                                // push rax
            break;
        }

        case REGISTER:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_PUSH_REG           (irCommand.reg_type)                                                  // push r?x
            }
            else
            {
                EMIT_POP_REG            (irCommand.reg_type)                                                   // pop r?x
            }
            break;
        
        case NUM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_MOV_REG_IMMED      (RDX, irCommand.argument)
                EMIT_ADD_REG_REG        (irCommand.reg_type, RDX)
                EMIT_PUSH_REG           (irCommand.reg_type)
            }
            else
            {
                MY_ASSERT (1, "Only push can have num_reg argument");
            }
            break;
        
        case MEM_NUM:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_CMD                (PUSH_R15_OFFSET)
                x86insert4ByteUnsignNum (compilerInfo, irCommand.argument);
            }
            else
            {
                EMIT_CMD                (POP_R15_OFFSET)
                x86insert4ByteUnsignNum (compilerInfo, irCommand.argument);
            }
            break;

        case MEM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_MOV_RNUM_RNUM      (R12, R15)
                EMIT_ADD_RNUM_REG       (R15, irCommand.reg_type)
                EMIT_CMD                (PUSH_R15_OFFSET)
                x86insert4ByteSignNum   (compilerInfo, 0);
                EMIT_MOV_RNUM_RNUM      (R15, R12)
            }
            else
            {
                EMIT_MOV_RNUM_RNUM      (R12, R15)
                EMIT_ADD_RNUM_REG       (R15, irCommand.reg_type)
                EMIT_CMD                (POP_R15_OFFSET)
                x86insert4ByteSignNum   (compilerInfo, 0);
                EMIT_MOV_RNUM_RNUM      (R15, R12)
            }
            break;

        case MEM_NUM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_MOV_RNUM_RNUM      (R12, R15)
                EMIT_ADD_RNUM_REG       (R15, irCommand.reg_type)
                EMIT_CMD                (PUSH_R15_OFFSET)
                x86insert4ByteUnsignNum (compilerInfo, irCommand.argument);
                EMIT_MOV_RNUM_RNUM      (R15, R12)

            }
            else
            {
                EMIT_MOV_RNUM_RNUM      (R12, R15)
                EMIT_ADD_RNUM_REG       (R15, irCommand.reg_type)
                EMIT_CMD                (POP_R15_OFFSET)
                x86insert4ByteUnsignNum (compilerInfo, irCommand.argument);
                EMIT_MOV_RNUM_RNUM      (R15, R12)
            }
            break;

        default:
            MY_ASSERT (1, "Incorrect variation of push/pop command");
            break;
    }
}

static void x86TranslateCondJmps (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    size_t addrToContinue = irCommand.x86ip + SIZE_COND_JMP + SIZE_REL_PTR + 
                            SIZE_POP_REG + SIZE_POP_REG + SIZE_CMP_REG_REG;

    EMIT_POP_REG        (RAX)
    EMIT_POP_REG        (RDX)

    EMIT_CMP_REG_REG    (RDX, RAX)

    opcode_t typeJmp = {
        .size = SIZE_COND_JMP,
        .code = COND_JMP
    };

    switch (irCommand.cmd)
    {
        case CMD_JE:
        {
            typeJmp.code |= JE_MASK << 8;
            break;
        }
        case CMD_JA:
        {
            typeJmp.code |= JG_MASK << 8;
            break;
        }
        case CMD_JGE:
        {
            typeJmp.code |= JGE_MASK << 8;
            break;
        }
        case CMD_JB:
        {
            typeJmp.code |= JL_MASK << 8;
            break;
        }
        case CMD_JBE:
        {
            typeJmp.code |= JLE_MASK << 8;
            break;
        }
        case CMD_JNE:
        {
            typeJmp.code |= JNE_MASK << 8;
            break;
        }

        default:
        {
            MY_ASSERT (1, "Incorrect situation in bool expression");
            break;
        }
    }

    x86insertCmd (compilerInfo, typeJmp);

    int32_t relPtr = irCommand.argument - addrToContinue;

    x86insert4ByteUnsignNum (compilerInfo, relPtr);
}

static void x86TranslateJmpCall (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.cmd)
    {
        case CMD_JMP:
        {
            int32_t relPtr = irCommand.argument - (irCommand.x86ip + SIZE_JMP + SIZE_REL_PTR);
            EMIT_JMP_REL_PTR (relPtr)
            break;
        }

        case CMD_CALL:
        {
            size_t addrToContinue = irCommand.x86ip + SIZE_MOV_REG_IMMED + SIZE_ABS_PTR + 
                    SIZE_MOV_MEM_R14_RAX + SIZE_ADD_REG_IMMED + SIZE_4BYTE_NUM + SIZE_JMP + SIZE_REL_PTR;

            int32_t relPtr = irCommand.argument - addrToContinue;
            u_int64_t absAddr = (u_int64_t)(compilerInfo->machineCode.buf + addrToContinue);

            EMIT_MOV_REG_IMMED  (RAX, absAddr)
            EMIT_CMD            (MOV_MEM_R14_RAX)
            EMIT_ADD_RNUM_IMMED (R14, 8)
            EMIT_JMP_REL_PTR    (relPtr)

            break;
        }  

        default:        
            MY_ASSERT (1, "Incorrect jmp or call")
            break;
    }

    
}

static void x86TranslateSqrt (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT_POP_REG        (RAX)
    EMIT_CVTSI2SD_REG   (XMM0, RAX)

    EMIT_MOV_REG_IMMED  (RAX, 1000)
    EMIT_CVTSI2SD_REG   (XMM1, RAX)

    EMIT_DIVPD          (XMM0, XMM1)
    EMIT_CMD            (SQRTPD_XMM0_XMM0)
    EMIT_MULPD          (XMM0, XMM1)

    EMIT_CVTSD2SI_REG   (RAX, XMM0)
    EMIT_PUSH_REG       (RAX)
}

static void x86TranslateComp (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT_POP_REG        (RAX)
    EMIT_POP_REG        (RDX)

    EMIT_CMP_REG_REG    (RDX, RAX)

    opcode_t bool_expr = {
        .size = SIZE_COND_JMP,
        .code = COND_JMP
    };

    switch (irCommand.cmd)
    {
        case CMD_BE:
        {
            bool_expr.code |= JE_MASK << 8;
            break;
        }
        case CMD_BA:
        {
            bool_expr.code |= JG_MASK << 8;
            break;
        }
        case CMD_BGE:
        {
            bool_expr.code |= JGE_MASK << 8;
            break;
        }
        case CMD_BB:
        {
            bool_expr.code |= JL_MASK << 8;
            break;
        }
        case CMD_BBE:
        {
            bool_expr.code |= JLE_MASK << 8;
            break;
        }
        case CMD_BNE:
        {
            bool_expr.code |= JNE_MASK << 8;
            break;
        }

        default:
        {
            MY_ASSERT (1, "Incorrect situation in bool expression");
            break;
        }
    }

    x86insertCmd (compilerInfo, bool_expr);
    int32_t relAddr = SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM + SIZE_PUSH_REG + SIZE_JMP +    // -> j? .equal
                    SIZE_REL_PTR;
    x86insert4ByteSignNum (compilerInfo, relAddr);

    EMIT_MOV_REG_IMMED  (RAX, 0)
    EMIT_PUSH_REG       (RAX)

    int32_t relAddrEnd = SIZE_MOV_REG_IMMED+SIZE_8BYTE_NUM+SIZE_PUSH_REG;
    EMIT_JMP_REL_PTR    (relAddrEnd)           //jmp .after_equal

    //.equal
    EMIT_MOV_REG_IMMED  (RAX, 1)
    EMIT_PUSH_REG       (RAX)
    //.after_equal

}

static void x86TranslateSimpleMath  (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.cmd)
    {
        case CMD_ADD:
        {
            EMIT_POP_REG        (RAX)
            EMIT_POP_REG        (RDX)
            EMIT_ADD_REG_REG    (RDX, RAX)
            EMIT_PUSH_REG       (RDX)
            break;
        }
            
        
        case CMD_SUB:
        {
            EMIT_POP_REG        (RAX)
            EMIT_POP_REG        (RDX)
            EMIT_SUB_REG_REG    (RDX, RAX)
            EMIT_PUSH_REG       (RDX)
            break;
        }
            
        case CMD_MUL:
        {
            EMIT_POP_REG        (RAX)
            EMIT_CVTSI2SD_REG   (XMM1, RAX)

            EMIT_POP_REG        (RAX)
            EMIT_CVTSI2SD_REG   (XMM0, RAX)

            EMIT_MOV_REG_IMMED  (RAX, 1000)
            EMIT_CVTSI2SD_REG   (XMM2, RAX)

            EMIT_DIVPD          (XMM0, XMM2)
            EMIT_DIVPD          (XMM1, XMM2)

            EMIT_MULPD          (XMM0, XMM1)
            EMIT_MULPD          (XMM0, XMM2)

            EMIT_CVTSD2SI_REG   (RAX, XMM0)
            EMIT_PUSH_REG       (RAX)
            break;
        }
            

        case CMD_DIV:
        {
            EMIT_POP_REG        (RAX)
            EMIT_CVTSI2SD_REG   (XMM1, RAX)

            EMIT_POP_REG        (RAX)
            EMIT_CVTSI2SD_REG   (XMM0, RAX)

            EMIT_MOV_REG_IMMED  (RAX, 1000)
            EMIT_CVTSI2SD_REG   (XMM2, RAX)

            EMIT_DIVPD          (XMM0, XMM1)
            EMIT_MULPD          (XMM0, XMM2)

            EMIT_CVTSD2SI_REG   (RAX, XMM0)
            EMIT_PUSH_REG       (RAX)
            break;
        }
            

        default:
            MY_ASSERT (1, "Error operation in sub_add case")
            break;
    }
}

static void dumpCmd (compilerInfo_t * compilerInfo, opcode_t cmdCode, const char * name, size_t lineSrcFile, const char * nameCallingFunc)
{
    FILE * logfile = openFile ("./logs/logTranslation.txt", "a");

    logDumpline ("------------------start(cmd)-------------------\n")

    logDumpline ("code line:                     %zu\n", lineSrcFile)
    logDumpline ("name of the calling function : %s\n\n", nameCallingFunc)

    logDumpline ("lengthx86Code:    %zu\n", compilerInfo->machineCode.len)
    if (cmdCode.code != BAD_CMD)
    {
        logDumpline ("cmd:              %lu (%lx)\n"
                     "name:             %s\n" 
                     "size_cmd:         %zu\n", cmdCode.code, cmdCode.code, name, cmdCode.size);
    }

    logDumpline ("-------------------end(cmd)--------------------\n\n")

    fclose (logfile);
}

static void dumpNum (compilerInfo_t * compilerInfo, int64_t number, size_t lineSrcFile, const char * nameCallingFunc)
{
    FILE * logfile = openFile ("./logs/logTranslation.txt", "a");

    logDumpline ("------------------start(num)-------------------\n")

    logDumpline ("code line:                     %zu\n", lineSrcFile)
    logDumpline ("name of the calling function : %s\n\n", nameCallingFunc)

    logDumpline ("lengthx86Code:    %zu\n", compilerInfo->machineCode.len)
    logDumpline ("number:           %ld\n", number)

    logDumpline ("-------------------end(num)--------------------\n\n")

    fclose (logfile);
}

static void dumpAbsPtr (compilerInfo_t * compilerInfo, u_int64_t ptr, size_t lineSrcFile, const char * nameCallingFunc)
{
    FILE * logfile = openFile ("./logs/logTranslation.txt", "a");

    logDumpline ("------------------start(ABSptr)-------------------\n")

    logDumpline ("code line:                     %zu\n", lineSrcFile)
    logDumpline ("name of the calling function : %s\n\n", nameCallingFunc)

    logDumpline ("lengthx86Code:    %zu\n", compilerInfo->machineCode.len)
    logDumpline ("ptr:              %zu (%lx)\n", ptr, ptr)

    logDumpline ("-------------------end(ABSptr)--------------------\n\n")

    fclose (logfile);
}

static void dumpRelPtr (compilerInfo_t * compilerInfo, u_int32_t ptr, size_t lineSrcFile, const char * nameCallingFunc)
{
    FILE * logfile = openFile ("./logs/logTranslation.txt", "a");

    logDumpline ("------------------start(RELptr)-------------------\n")

    logDumpline ("code line:                     %zu\n", lineSrcFile)
    logDumpline ("name of the calling function : %s\n\n", nameCallingFunc)

    logDumpline ("lengthx86Code:    %zu\n", compilerInfo->machineCode.len)
    logDumpline ("ptr:              %u (%x)\n", ptr, ptr)

    logDumpline ("-------------------end(RELptr)--------------------\n\n")

    fclose (logfile);
}

static void dumpFunc (compilerInfo_t * compilerInfo, const char * message, size_t lineSrcFile, const char * nameCallingFunc)
{
    FILE * logfile = openFile ("./logs/logTranslation.txt", "a");

    logDumpline ("------------------start(func)-------------------\n")

    logDumpline ("code line:                     %zu\n", lineSrcFile)
    logDumpline ("name of the calling function : %s\n\n", nameCallingFunc)

    logDumpline ("lengthx86Code:    %s\n", message)

    logDumpline ("-------------------end(func)--------------------\n\n")

    fclose (logfile);
}


static void x86TranslateHlt (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT   (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")
    EMIT_CMD    (RET)
}

static void x86TranslateRet (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")
    
    EMIT_SUB_RNUM_IMMED (R14, 8)
    EMIT_CMD            (PUSH_MEM_R14)
    EMIT_CMD            (RET)
}

static void insert8ByteSignNum (compilerInfo_t * compilerInfo, int64_t num)
{
    *((int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len)) = num;
    compilerInfo->machineCode.len += sizeof (int64_t);
}

static void insert8ByteUnsignNum (compilerInfo_t * compilerInfo, u_int64_t num)
{
    *(u_int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    num;
    compilerInfo->machineCode.len += sizeof (u_int64_t);
}

static void insert4ByteSignNum (compilerInfo_t * compilerInfo, int32_t num)
{
    *(int32_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    num;
    compilerInfo->machineCode.len += sizeof(int32_t);
}

static void insert4ByteUnsignNum (compilerInfo_t * compilerInfo, u_int32_t num)
{
    *(u_int32_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    num;
    compilerInfo->machineCode.len += sizeof(u_int32_t);
}

static void insertCmd (compilerInfo_t * compilerInfo, opcode_t cmd)
{
    *((u_int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len)) = cmd.code;
    compilerInfo->machineCode.len += cmd.size;
}

