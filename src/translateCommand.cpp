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
static void insertAbsPtr            (compilerInfo_t * compilerInfo, u_int64_t ptr);
static void insertNum               (compilerInfo_t * compilerInfo, int64_t number);
static void insertCmd               (compilerInfo_t * compilerInfo, opcode_t cmd);
static void insertRelPtr            (compilerInfo_t * compilerInfo, int32_t ptr);
static void myPrintf                (double num);
static void myScanf                 (int * num);

static void dumpCmd     (compilerInfo_t * compilerInfo, opcode_t cmdCode, const char * name, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpNum     (compilerInfo_t * compilerInfo, int64_t number, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpAbsPtr  (compilerInfo_t * compilerInfo, u_int64_t number, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpRelPtr  (compilerInfo_t * compilerInfo, u_int32_t number, size_t lineSrcFile, const char * nameCallingFunc);
static void dumpFunc    (compilerInfo_t * compilerInfo, const char * message, size_t lineSrcFile, const char * nameCallingFunc);


const int64_t BAD_NUMBER = 123456;
const u_int32_t NUMBER_OF_FUNCTION = 100;

#define dumpx86Func(compilerInfo, message)                             \
    dumpFunc (compilerInfo, message, __LINE__, __PRETTY_FUNCTION__);

#define dumpx86Cmd(compilerInfo, cmdOpcode, nameCmd)                                     \
    dumpCmd (compilerInfo, cmdOpcode, #nameCmd, __LINE__, __PRETTY_FUNCTION__);  

#define dumpx86Num(compilerInfo, number)                                        \
    dumpNum (compilerInfo, number, __LINE__, __PRETTY_FUNCTION__);  

#define dumpx86AbsPtr(compilerInfo, pointer)                                    \
    dumpAbsPtr (compilerInfo, pointer, __LINE__, __PRETTY_FUNCTION__);  

#define dumpx86RelPtr(compilerInfo, pointer)                                        \
    dumpRelPtr (compilerInfo, pointer, __LINE__, __PRETTY_FUNCTION__);  

#define x86insertCmd(compilerInfo, cmd)         \
    insertCmd (compilerInfo, cmd);              \
    dumpx86Cmd (compilerInfo, cmd, #cmd)

#define x86insertNum(compilerInfo, num)         \
    insertNum (compilerInfo, num);              \
    dumpx86Num (compilerInfo, num)

#define x86insertRelPtr(compilerInfo, ptr)      \
    insertRelPtr (compilerInfo, ptr);           \
    dumpx86RelPtr (compilerInfo, ptr)

#define x86insertAbsPtr(compilerInfo, ptr)      \
    insertAbsPtr (compilerInfo, ptr);           \
    dumpx86AbsPtr (compilerInfo, ptr)

#define logDumpline(message, ...)                       \
    fprintf (logfile, message, ##__VA_ARGS__);

#define EMIT(compilerInfo, name, cmd, reg, offset)      \
    opcode_t name = {                                   \
        .size = SIZE_##cmd,                             \
        .code = cmd | (((u_int64_t) reg) << offset)     \
    };                                                  \
    x86insertCmd (compilerInfo, name)

#define EMIT_MATH_OPERS(compilerInfo, name, cmd, reg1, offset1, reg2, offset2)              \
    opcode_t name = {                                                                       \
        .size = SIZE_##cmd,                                                                 \
        .code = cmd | (((u_int64_t) reg1) << offset1) | (((u_int64_t) reg2) << offset2)     \
    };                                                                                      \
    x86insertCmd (compilerInfo, name)


void JITCompile (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "There is no access to commands array");
    MY_ASSERT (compilerInfo->irInfo.irArray[0].cmd == CMD_POP, "First command is pop, but stack is empty");

    size_t sizeArr = compilerInfo->irInfo.sizeArray;
    ir_t * irArr   = compilerInfo->irInfo.irArray;

    FILE * logfile = openFile ("./logs/logTranslation.txt", "w");           // clear old information
    fclose (logfile);

    EMIT (compilerInfo, mov_r15_mem_buf, MOV_RNUM_IMMED, R15, 8)
    x86insertAbsPtr (compilerInfo, (u_int64_t) compilerInfo->x86_memory_buf);

    EMIT (compilerInfo, mov_rsp_stackBuf, MOV_RNUM_IMMED, R14, 8)
    x86insertAbsPtr (compilerInfo, (u_int64_t) compilerInfo->x86StackBuf);

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
    EMIT (compilerInfo, mov_reg_num, MOV_REG_IMMED, irCommand.reg_type, 8)
    x86insertNum (compilerInfo, irCommand.argument)
}

static void x86TranslateOut (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    dumpx86Func (compilerInfo, "Translate [OUT]")

    size_t addrToContinue = irCommand.x86ip + SIZE_CVTSI2SD_XMM0_RAX + SIZE_PUSHA2 + SIZE_PUSHA1 + 
             SIZE_MOV_RBP_RSP + SIZE_ALIGN_STACK + SIZE_x86_CALL + SIZE_REL_PTR;

    int32_t relAddrPrintf = (u_int64_t)myPrintf - (u_int64_t)(compilerInfo->machineCode.buf + addrToContinue);

    EMIT (compilerInfo, cvtsi_xmm0_rsp, CVTSI2SD_XMM0_RSP, 0, 0)
    EMIT (compilerInfo, pusha2, PUSHA2, 0, 0)
    EMIT (compilerInfo, pusha1, PUSHA1, 0, 0)
    EMIT (compilerInfo, mov_rbp_rsp, MOV_RBP_RSP, 0, 0)
    EMIT (compilerInfo, align_stack, ALIGN_STACK, 0, 0)

    EMIT (compilerInfo, call, x86_CALL, 0, 0)
    x86insertRelPtr (compilerInfo, relAddrPrintf)

    EMIT (compilerInfo, mov_rsp_rbp, MOV_RSP_RBP, 0, 0)
    EMIT (compilerInfo, popa1, POPA1, 0, 0)
    EMIT (compilerInfo, popa2, POPA2, 0, 0)
    EMIT (compilerInfo, add_rsp_8, ADD_RSP_8, 0, 0)

}

static void x86TranslateIn (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    dumpx86Func (compilerInfo, "Translate [IN]")

    size_t addrToContinue = irCommand.x86ip + SIZE_SUB_RSP_8 + SIZE_MOV_REG_REG + SIZE_PUSHA1 + SIZE_PUSHA2 + SIZE_MOV_RBP_RSP +
                            SIZE_ALIGN_STACK + SIZE_x86_CALL + SIZE_REL_PTR;

    int32_t relAddr = (u_int64_t)myScanf - (u_int64_t)(compilerInfo->machineCode.buf + addrToContinue);

    EMIT (compilerInfo, sub_rsp_8, SUB_RSP_8, 0, 0)
    EMIT_MATH_OPERS (compilerInfo, mov_rdi_rsp, MOV_REG_REG, RDI, 16, RSP, 19)
    EMIT (compilerInfo, pusha2, PUSHA2, 0, 0)
    EMIT (compilerInfo, pusha1, PUSHA1, 0, 0)
    EMIT (compilerInfo, mov_rbp_rsp, MOV_RBP_RSP, 0, 0)
    EMIT (compilerInfo, align_stack, ALIGN_STACK, 0, 0)

    EMIT (compilerInfo, call, x86_CALL, 0, 0)
    x86insertRelPtr (compilerInfo, relAddr);

    EMIT (compilerInfo, mov_rsp_rpb, MOV_RSP_RBP, 0, 0)
    EMIT (compilerInfo, popa1, POPA1, 0, 0)
    EMIT (compilerInfo, popa2, POPA2, 0, 0)
}

static void x86TranslatePushPop (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.argument_type)
    {
        case NUMBER:
        {
            EMIT         (compilerInfo, m_r_im, MOV_REG_IMMED, irCommand.reg_type, 8)   // mov rax, ...
            x86insertNum (compilerInfo, irCommand.argument);                            // mov rax, num
            EMIT         (compilerInfo, push_reg, PUSH_REG, irCommand.reg_type, 0)      // push rax
            break;
        }

        case REGISTER:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT (compilerInfo, push_reg, PUSH_REG, irCommand.reg_type, 0)            // push r?x
            }
            else
            {
                EMIT (compilerInfo, pop_reg, POP_REG, irCommand.reg_type, 0)             // pop r?x
            }
            break;
        
        case NUM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT            (compilerInfo, m_r_im, MOV_REG_IMMED, RDX, 8)                            // mov rbx, ...
                x86insertNum    (compilerInfo, irCommand.argument);                                      // mov rbx, num
                EMIT_MATH_OPERS (compilerInfo, add_reg_reg, ADD_REG_REG, irCommand.reg_type, 19, RDX, 16)// add rax, rbx
                EMIT            (compilerInfo, push_reg, PUSH_REG, irCommand.reg_type, 0)
            }
            else
            {
                MY_ASSERT (1, "Only push can have num_reg argument");
            }
            break;
        
        case MEM_NUM:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT        (compilerInfo, push_r15_offset, PUSH_R15_OFFSET, 0, 0)
                x86insertRelPtr   (compilerInfo, (u_int32_t) irCommand.argument);
            }
            else
            {
                EMIT        (compilerInfo, pop_r15_off, POP_R15_OFFSET, 0, 0)
                x86insertRelPtr   (compilerInfo, (u_int32_t) irCommand.argument);
            }
            break;

        case MEM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_MATH_OPERS (compilerInfo, mov_r12_r15, MOV_RNUM_RNUM, R12, 16, R15, 19)
                EMIT_MATH_OPERS (compilerInfo, add_reg_reg, ADD_RNUM_REG, R15, 16, irCommand.reg_type, 19)
                EMIT            (compilerInfo, push_r15_offset, PUSH_R15_OFFSET, 0, 0)
                x86insertRelPtr (compilerInfo, (int32_t) 0);
                EMIT_MATH_OPERS (compilerInfo, mov_r15_r12, MOV_RNUM_RNUM, R15, 16, R12, 19)
            }
            else
            {
                EMIT_MATH_OPERS (compilerInfo, mov_r12_r15, MOV_RNUM_RNUM, R12, 16, R15, 19)
                EMIT_MATH_OPERS (compilerInfo, ad_r_r, ADD_RNUM_REG, R15, 16, irCommand.reg_type, 19)
                EMIT            (compilerInfo, pop_r15_off, POP_R15_OFFSET, 0, 0)
                x86insertRelPtr (compilerInfo, (int32_t) 0);
                EMIT_MATH_OPERS (compilerInfo, mov_r15_r12, MOV_RNUM_RNUM, R15, 16, R12, 19)
            }
            break;

        case MEM_NUM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT_MATH_OPERS (compilerInfo, mov_r12_r15, MOV_RNUM_RNUM, R12, 16, R15, 19)
                EMIT_MATH_OPERS (compilerInfo, add_r_r, ADD_RNUM_REG, R15, 16, irCommand.reg_type, 19)
                EMIT            (compilerInfo, push_r15_off, PUSH_R15_OFFSET, 0, 0)
                x86insertRelPtr (compilerInfo, (u_int32_t) irCommand.argument);
                EMIT_MATH_OPERS (compilerInfo, mov_r15_r12, MOV_RNUM_RNUM, R15, 16, R12, 19)
            }
            else
            {
                EMIT_MATH_OPERS (compilerInfo, mov_r12_r15, MOV_RNUM_RNUM, R12, 16, R15, 19)
                EMIT_MATH_OPERS (compilerInfo, add_r_r, ADD_RNUM_REG, R15, 16, irCommand.reg_type, 19)
                EMIT            (compilerInfo, pop_r15_off, POP_R15_OFFSET, 0, 0)
                x86insertRelPtr (compilerInfo, (u_int32_t) irCommand.argument);
                EMIT_MATH_OPERS (compilerInfo, mov_r15_r12, MOV_RNUM_RNUM, R15, 16, R12, 19)
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

    size_t addrToContinue = irCommand.x86ip + SIZE_x86_COND_JMP + SIZE_REL_PTR + 
                            SIZE_POP_REG + SIZE_POP_REG + SIZE_CMP_REG_REG;

    EMIT (compilerInfo, pop_rax, POP_REG, RAX, 0)
    EMIT (compilerInfo, pop_rbx, POP_REG, RDX, 0)

    EMIT_MATH_OPERS (compilerInfo, cmp_rax_rbx, CMP_REG_REG, RDX, 16, RAX, 19)

    opcode_t typeJmp = {
        .size = SIZE_x86_COND_JMP,
        .code = x86_COND_JMP
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

    x86insertRelPtr (compilerInfo, relPtr);
}

static void x86TranslateJmpCall (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.cmd)
    {
        case CMD_JMP:
        {
            EMIT (compilerInfo, cmd_jmp, x86_JMP, 0, 0)
            int32_t relPtr = irCommand.argument - (irCommand.x86ip + SIZE_x86_JMP + SIZE_REL_PTR);
            x86insertRelPtr (compilerInfo, relPtr);
            break;
        }

        case CMD_CALL:
        {
            size_t addrToContinue = irCommand.x86ip + SIZE_MOV_REG_IMMED + SIZE_ABS_PTR + 
                    SIZE_MOV_MEM_R14_RAX + SIZE_ADD_R14_8 + SIZE_x86_JMP + SIZE_REL_PTR;

            int32_t relPtr = irCommand.argument - addrToContinue;
            u_int64_t absAddr = (u_int64_t)(compilerInfo->machineCode.buf + addrToContinue);

            EMIT (compilerInfo, mov_rax_absAddr, MOV_REG_IMMED, RAX, 8)
            x86insertAbsPtr (compilerInfo, absAddr);
            EMIT (compilerInfo, mov_mem_r14_rax, MOV_MEM_R14_RAX, 0, 0)
            EMIT (compilerInfo, add_r14_8, ADD_R14_8, 0, 0)
            EMIT (compilerInfo, jump, x86_JMP, 0, 0)
            x86insertRelPtr(compilerInfo, relPtr);

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

    EMIT            (compilerInfo, pop_rax, POP_REG, RAX, 0)
    EMIT            (compilerInfo, cvtsi_xmm0_rax, CVTSI2SD_XMM0_RAX, 0, 0)

    EMIT            (compilerInfo, mov_rax, MOV_REG_IMMED, RAX, 8)
    x86insertNum    (compilerInfo, 1000);
    EMIT            (compilerInfo, cvtsi_xmm1_rax, CVTSI2SD_XMM1_RAX, 0, 0)

    EMIT_MATH_OPERS (compilerInfo, divpd_xmm0_xmm1, DIVPD_XMM0_XMM0, XMM0, 27, XMM1, 24)
    EMIT            (compilerInfo, sqrt, SQRTPD_XMM0_XMM0, 0, 0)
    EMIT_MATH_OPERS (compilerInfo, mulpd_xmm0_xmm1, MULPD_XMM0_XMM0, XMM0, 27, XMM1, 24)

    EMIT            (compilerInfo, cvtsd, CVTSD2SI_RAX_XMM0, 0, 0)
    EMIT            (compilerInfo, push, PUSH_REG, RAX, 0)
}

static void x86TranslateComp (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT (compilerInfo, pop_rax, POP_REG, RAX, 0)
    EMIT (compilerInfo, pop_rbx, POP_REG, RDX, 0)

    EMIT_MATH_OPERS (compilerInfo, cmp_rax_rbx, CMP_REG_REG, RDX, 16, RAX, 19)

    opcode_t bool_expr = {
        .size = SIZE_x86_COND_JMP,
        .code = x86_COND_JMP
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

    x86insertCmd (compilerInfo, bool_expr);                                             // 
    int32_t relAddr = SIZE_MOV_REG_IMMED + SIZE_NUM + SIZE_PUSH_REG + SIZE_x86_JMP +    // -> j? .equal
                    SIZE_REL_PTR;                                                       //
    x86insertRelPtr (compilerInfo, relAddr);                                            //

    EMIT            (compilerInfo, mov_rax_0, MOV_REG_IMMED, RAX, 8)    //
    x86insertNum    (compilerInfo, 0);                                  // -> push 0
    EMIT            (compilerInfo, push_rax, PUSH_REG, RAX, 0)          //

    EMIT (compilerInfo, jmp_end, x86_JMP, 0, 0)                         //jmp .after_equal
    int32_t relAddrEnd = SIZE_MOV_REG_IMMED+SIZE_NUM+SIZE_PUSH_REG;
    x86insertRelPtr (compilerInfo, relAddrEnd);

    //.equal
    EMIT (compilerInfo, mov_rax_1, MOV_REG_IMMED, RAX, 8)   //
    x86insertNum (compilerInfo, 1);                         // -> push 1
    EMIT (compilerInfo, push_rax1, PUSH_REG, RAX, 0)        //
    //.after_equal

}

static void x86TranslateSimpleMath  (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.cmd)
    {
        case CMD_ADD:
        {
            EMIT (compilerInfo, pop_reg1, POP_REG, RAX, 0)
            EMIT (compilerInfo, pop_reg2, POP_REG, RDX, 0)
            EMIT_MATH_OPERS (compilerInfo, add_reg_reg, ADD_REG_REG, RDX, 16, RAX, 19)
            EMIT (compilerInfo, push_reg1, PUSH_REG, RDX, 0)
            break;
        }
            
        
        case CMD_SUB:
        {
            EMIT (compilerInfo, pop_reg1, POP_REG, RAX, 0)
            EMIT (compilerInfo, pop_reg2, POP_REG, RDX, 0)
            EMIT_MATH_OPERS (compilerInfo, add_reg_reg, SUB_REG_REG, RDX, 16, RAX, 19)
            EMIT (compilerInfo, push_reg1, PUSH_REG, RDX, 0)
            break;
        }
            
        case CMD_MUL:
        {
            EMIT (compilerInfo, pop1, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm1_rax, CVTSI2SD_XMM1_RAX, 0, 0)

            EMIT (compilerInfo, pop2, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm0_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT (compilerInfo, mov_reg_num, MOV_REG_IMMED, RAX, 8)
            x86insertNum (compilerInfo, 1000);
            EMIT (compilerInfo, cvtsi_xmm2_rax, CVTSI2SD_XMM2_RAX, 0, 0)

            EMIT_MATH_OPERS (compilerInfo, divpd1, DIVPD_XMM0_XMM0, XMM0, 27, XMM2, 24)
            EMIT_MATH_OPERS (compilerInfo, divpd2, DIVPD_XMM0_XMM0, XMM1, 27, XMM2, 24)

            EMIT_MATH_OPERS (compilerInfo, mulpd1, MULPD_XMM0_XMM0, XMM0, 27, XMM1, 24)
            EMIT_MATH_OPERS (compilerInfo, mulpd2, MULPD_XMM0_XMM0, XMM0, 27, XMM2, 24)

            EMIT (compilerInfo, cvtsd, CVTSD2SI_RAX_XMM0, 0, 0)
            EMIT (compilerInfo, push, PUSH_REG, RAX, 0)
            break;
        }
            

        case CMD_DIV:
        {
            EMIT (compilerInfo, pop1, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm1_rax, CVTSI2SD_XMM1_RAX, 0, 0)

            EMIT (compilerInfo, pop2, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm0_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT (compilerInfo, mov_reg_num, MOV_REG_IMMED, RAX, 8)
            x86insertNum (compilerInfo, 1000);
            EMIT (compilerInfo, cvtsi_xmm2_rax, CVTSI2SD_XMM2_RAX, 0, 0)

            EMIT_MATH_OPERS (compilerInfo, divpd, DIVPD_XMM0_XMM0, XMM0, 27, XMM1, 24)
            EMIT_MATH_OPERS (compilerInfo, mulpd, MULPD_XMM0_XMM0, XMM0, 27, XMM2, 24)

            EMIT (compilerInfo, cvtsd_rax_xmm0, CVTSD2SI_RAX_XMM0, 0, 0)
            EMIT (compilerInfo, push, PUSH_REG, RAX, 0)
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
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT (compilerInfo, ret_main, x86_RET, 0, 0)
}

static void x86TranslateRet (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")
    
    EMIT (compilerInfo, sub_r14_8, SUB_R14_8, 0, 0)
    EMIT (compilerInfo, push_mem_r14, PUSH_MEM_R14, 0, 0)
    EMIT (compilerInfo, ret, x86_RET, 0, 0)
}

static void insertAbsPtr (compilerInfo_t * compilerInfo, u_int64_t ptr)
{
    *(u_int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    ptr;
    compilerInfo->machineCode.len += sizeof (u_int64_t);
}

static void insertRelPtr (compilerInfo_t * compilerInfo, int32_t ptr)
{
    * (int32_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    ptr;
    compilerInfo->machineCode.len += sizeof(int32_t);
}

static void insertCmd (compilerInfo_t * compilerInfo, opcode_t cmd)
{
    *((u_int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len)) = cmd.code;
    compilerInfo->machineCode.len += cmd.size;
}

static void insertNum (compilerInfo_t * compilerInfo, int64_t number)
{
    *((int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len)) = number;
    compilerInfo->machineCode.len += sizeof (int64_t);
}