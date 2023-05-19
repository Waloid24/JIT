#include "../include/translateCommand.hpp"

static void x86TranslatePushPop     (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateSimpleMath  (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateSqrt        (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateJmpCall     (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateIn          (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateRet         (compilerInfo_t * compilerInfo, ir_t irCommand);
static void x86TranslateHlt         (compilerInfo_t * compilerInfo, ir_t irCommand);
static void ptrTox86MemoryBuf       (compilerInfo_t * compilerInfo, u_int64_t ptr);
static void x86insertCmd            (compilerInfo_t * compilerInfo, opcode_t cmd);
static void x86insertPtr            (compilerInfo_t * compilerInfo, u_int32_t ptr);

static void dumpx86Represent  (compilerInfo_t * compilerInfo, opcode_t cmdCode, int64_t number, 
                        size_t lineSrcFile, char * nameCallingFunc);

const int64_t BAD_NUMBER = 123456;
const size_t SIZE_x86_BUF = 128;

#define dumpx86(compilerInfo, irCommand, number)                                        \
    dumpx86Represent (compilerInfo, irCommand, number, __LINE__, __PRETTY_FUNCTION__);  

#define logDumpline(message, ...)                       \
    fprintf (logfile, message, ##__VA_ARGS__);

#define EMIT(compilerInfo, name, cmd, reg, offset)      \
    opcode_t name = {                                   \
        .size = SIZE_##cmd,                             \
        .code = cmd | (((u_int64_t) reg) << offset)     \
    };                                                  \
    x86insertCmd (compilerInfo, name);

#define EMIT_MATH_OPERS(compilerInfo, name, cmd, reg1, offset1, reg2, offset2)              \
    opcode_t name = {                                                                       \
        .size = SIZE_##cmd,                                                                 \
        .code = cmd | (((u_int64_t) reg1) << offset1) | (((u_int64_t) reg2) << offset2)     \
    };                                                                                      \
    x86insertCmd (compilerInfo, name);

void JITCompile (compilerInfo_t * compilerInfo)
{
    MY_ASSERT (compilerInfo->irInfo.irArray == nullptr, "There is no access to commands array");
    MY_ASSERT (compilerInfo->irInfo.irArray[0].cmd == CMD_POP, "First command is pop, but stack is empty");

    size_t sizeArr = compilerInfo->irInfo.sizeArray;
    ir_t * irArr   = compilerInfo->irInfo.irArray;

    compilerInfo->machineCode.buf = (char *) calloc (sizeArr*4, sizeof(char));

    EMIT (compilerInfo, m_r_im, MOV_REG_IMMED, R15, 8)
    ptrTox86MemoryBuf (compilerInfo, (u_int64_t) compilerInfo->x86_memory_buf);

    for (size_t irIndex, machineIndex = 0; irIndex < sizeArr; irIndex++, machineIndex++)
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
            
            case CMD_SQRT:
                x86TranslateSqrt (compilerInfo, irArr[irIndex]);
                break;

            case CMD_HLT:
                x86TranslateHlt (compilerInfo, irArr[irIndex]);
                break;

            case CMD_IN:
                x86TranslateIn (compilerInfo, irArr[irIndex]);

            default:
                MY_ASSERT (1, "Incorrect command type");
                break;
        }

    }
}

static void x86insertCmd (compilerInfo_t * compilerInfo, opcode_t cmd)
{
    *((u_int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len)) = cmd.code;
    compilerInfo->machineCode.len += cmd.size;
}

static void x86insertNum (compilerInfo_t * compilerInfo, int64_t number)
{
    *((int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len)) = number;
    compilerInfo->machineCode.len += sizeof (int64_t);
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
                EMIT            (compilerInfo, m_r_im, MOV_REG_IMMED, RBX, 8)                            // mov rbx, ...
                x86insertNum    (compilerInfo, irCommand.argument);                                      // mov rbx, num
                EMIT_MATH_OPERS (compilerInfo, add_reg_reg, ADD_REG_REG, irCommand.reg_type, 19, RBX, 16)// add rax, rbx
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
                x86insertPtr   (compilerInfo, (u_int32_t) irCommand.argument);
            }
            else
            {
                EMIT        (compilerInfo, pop_r15_off, POP_R15_OFFSET, 0, 0)
                x86insertPtr   (compilerInfo, (u_int32_t) irCommand.argument);
            }
            break;

        case MEM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT            (compilerInfo, push_reg, PUSH_REG, R15, 0)
                EMIT_MATH_OPERS (compilerInfo, add_reg_reg, ADD_REG_REG, R15, 19, irCommand.reg_type, 16)
                EMIT            (compilerInfo, push_r15_offset, PUSH_R15_OFFSET, 0, 0)
                x86insertPtr       (compilerInfo, (u_int32_t) 0);
                EMIT            (compilerInfo, pop_reg, POP_REG, R15, 0)
            }
            else
            {
                EMIT            (compilerInfo, push_reg, PUSH_REG, R15, 0)
                EMIT_MATH_OPERS (compilerInfo, ad_r_r, ADD_REG_REG, R15, 19, irCommand.reg_type, 16)
                EMIT            (compilerInfo, pop_r15_off, POP_R15_OFFSET, 0, 0)
                x86insertPtr       (compilerInfo, (u_int32_t) 0);
                EMIT            (compilerInfo, pop_r, POP_REG, R15, 0)
            }
            break;

        case MEM_NUM_REG:
            if (irCommand.cmd == CMD_PUSH)
            {
                EMIT            (compilerInfo, push_r, PUSH_REG, R15, 0)
                EMIT_MATH_OPERS (compilerInfo, add_r_r, ADD_REG_REG, R15, 19, irCommand.reg_type, 16)
                EMIT            (compilerInfo, push_r15_off, PUSH_R15_OFFSET, 0, 0)
                x86insertPtr       (compilerInfo, (u_int32_t) irCommand.argument);
                EMIT            (compilerInfo, pop_r, POP_REG, R15, 0)
            }
            else
            {
                EMIT            (compilerInfo, push_r, PUSH_REG, R15, 0)
                EMIT_MATH_OPERS (compilerInfo, add_r_r, ADD_REG_REG, R15, 19, irCommand.reg_type, 16)
                EMIT            (compilerInfo, pop_r15_off, POP_R15_OFFSET, 0, 0)
                x86insertPtr       (compilerInfo, (u_int32_t) irCommand.argument);
                EMIT            (compilerInfo, pop_r, POP_REG, R15, 0)
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

    EMIT (compilerInfo, pop_rax, POP_REG, RAX, 0)
    EMIT (compilerInfo, pop_rbx, POP_REG, RBX, 0)

    EMIT_MATH_OPERS (compilerInfo, cmp_rax_rbx, CMP_REG_REG, RBX, 19, RAX, 16)

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
    x86insertPtr (compilerInfo, irCommand.argument);
}

static void x86TranslateSqrt (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT (compilerInfo, pop_rax, POP_REG, RAX, 0)
    EMIT (compilerInfo, cvtsi_xmm0_rax, CVTSI2SD_XMM0_RAX, 0, 0)

    EMIT (compilerInfo, mov_rax, MOV_REG_IMMED, RAX, 8)
    x86insertNum (compilerInfo, 1000);
    EMIT (compilerInfo, cvtsi_xmm1_rax, CVTSI2SD_XMM0_RAX, 0, 0)

    EMIT_MATH_OPERS (compilerInfo, divpd, DIVPD_XMM0_XMM0, XMM0, 24, XMM1, 27)
    EMIT (compilerInfo, sqrt, SQRTPD_XMM0_XMM0, 0, 0)

    EMIT (compilerInfo, cvtsd, CVTSD2SI_RAX_XMM0, 0, 0)
    EMIT (compilerInfo, push, PUSH_REG, RAX, 0)
}

static void x86TranslateJmpCall (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    switch (irCommand.cmd)
    {
        case CMD_JMP:
            EMIT (compilerInfo, jmp, x86_JMP, 0, 0)
            break;
        
        case CMD_CALL:
            EMIT (compilerInfo, call, x86_CALL, 0, 0)
            break;
    }

    // x86insertPtr (compilerInfo, );           дописать!

}

static void x86TranslateComp (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT (compilerInfo, pop_rax, POP_REG, RAX, 0)
    EMIT (compilerInfo, pop_rbx, POP_REG, RBX, 0)

    EMIT_MATH_OPERS (compilerInfo, cmp_rax_rbx, CMP_REG_REG, RBX, 19, RAX, 16)

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

    x86insertCmd (compilerInfo, bool_expr);                                 // 
                                                                            // -> j? .equal
    x86insertPtr (compilerInfo, SIZE_MOV_REG_IMMED+SIZE_NUM+SIZE_PUSH_REG+  //
                                SIZE_x86_JMP+SIZE_REL_PTR);                 //

    EMIT (compilerInfo, mov_rax_0, MOV_REG_IMMED, RAX, 8)   //
    x86insertNum (compilerInfo, 0);                         // -> push 0
    EMIT (compilerInfo, push_rax, PUSH_REG, RAX, 0)         //

    EMIT (compilerInfo, jmp_end, x86_JMP, 0, 0)             //jmp .after_equal
    x86insertPtr (compilerInfo, SIZE_MOV_REG_IMMED+SIZE_NUM+SIZE_PUSH_REG);

    EMIT (compilerInfo, mov_rax_0, MOV_REG_IMMED, RAX, 8)   //
    x86insertNum (compilerInfo, 1);                         // -> push 1
    EMIT (compilerInfo, push_rax, PUSH_REG, RAX, 0)         //

}

static void x86TranslateSimpleMath  (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT (compilerInfo, pop_reg1, POP_REG, RAX, 0)
    EMIT (compilerInfo, pop_reg2, POP_REG, RBX, 0)

    switch (irCommand.cmd)
    {
        case CMD_ADD:
        {
            EMIT_MATH_OPERS (compilerInfo, add_reg_reg, ADD_REG_REG, RAX, 19, RBX, 16)
            break;
        }
            
        
        case CMD_SUB:
        {
            EMIT_MATH_OPERS (compilerInfo, add_reg_reg, SUB_REG_REG, RBX, 19, RAX, 16)
            break;
        }
            
        case CMD_MUL:
        {
            EMIT (compilerInfo, pop1, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm0_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT (compilerInfo, pop2, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm1_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT (compilerInfo, mov_reg_num, MOV_REG_IMMED, RAX, 8)
            x86insertNum (compilerInfo, 1000);
            EMIT (compilerInfo, cvtsi_xmm2_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT_MATH_OPERS (compilerInfo, divpd1, DIVPD_XMM0_XMM0, XMM0, 24, XMM2, 27)
            EMIT_MATH_OPERS (compilerInfo, divpd2, DIVPD_XMM0_XMM0, XMM1, 24, XMM2, 27)

            EMIT_MATH_OPERS (compilerInfo, mulpd1, MULPD_XMM0_XMM0, XMM0, 24, XMM1, 27)
            EMIT_MATH_OPERS (compilerInfo, mulpd2, MULPD_XMM0_XMM0, XMM0, 24, XMM2, 27)

            EMIT (compilerInfo, cvtsd, CVTSD2SI_RAX_XMM0, 0, 0)
            EMIT (compilerInfo, push, PUSH_REG, RAX, 0)
            break;
        }
            

        case CMD_DIV:
        {
            EMIT (compilerInfo, pop1, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm1_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT (compilerInfo, pop2, POP_REG, RAX, 0)
            EMIT (compilerInfo, cvtsi_xmm0_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT (compilerInfo, mov_reg_num, MOV_REG_IMMED, RAX, 8)
            x86insertNum (compilerInfo, 1000);
            EMIT (compilerInfo, cvtsi_xmm2_rax, CVTSI2SD_XMM0_RAX, 0, 0)

            EMIT_MATH_OPERS (compilerInfo, divpd, DIVPD_XMM0_XMM0, XMM0, 24, XMM1, 27)
            EMIT_MATH_OPERS (compilerInfo, mulpd, MULPD_XMM0_XMM0, XMM0, 24, XMM2, 27)

            EMIT (compilerInfo, cvtsd_rax_xmm0, CVTSD2SI_RAX_XMM0, 0, 0)
            EMIT (compilerInfo, push, PUSH_REG, RAX, 0)
            break;
        }
            

        default:
            MY_ASSERT (1, "Error operation in sub_add case")
            break;
    }

    EMIT (compilerInfo, push_reg1, PUSH_REG, RAX, 0)
}

static void dumpx86Represent  (compilerInfo_t * compilerInfo, opcode_t cmdCode, int64_t number, 
                        size_t lineSrcFile, char * nameCallingFunc)
{
    FILE * logfile = openFile ("logTranslation.txt", "a");

    logDumpline ("------------------start-------------------\n")

    logDumpline ("code line:                     %zu\n", lineSrcFile);
    logDumpline ("name of the calling function : %s\n\n")

    logDumpline ("lengthx86Code:    %zu\n", compilerInfo->machineCode.len)
    if (number != BAD_NUMBER)
    {
        logDumpline ("newElem (number): %ld\n", number);
    }
    if (cmdCode.code != BAD_CMD)
    {
        logDumpline ("newElem (cmd):    %lu\n" 
                     "size_cmd:         %zu", cmdCode.code, cmdCode.size);
                      
    }

    logDumpline ("-------------------end--------------------\n\n")

    fclose (logfile);
}

static void x86TranslateHlt (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")

    EMIT (compilerInfo, mov_rax_0x3c, MOV_REG_IMMED, RAX, 8)
    x86insertNum (compilerInfo, 0x3c);
    EMIT (compilerInfo, xor_rdi_rdi, XOR_RDI_RDI, 0, 0)
    EMIT (compilerInfo, syscall, SYSCALL, 0, 0)
}

static void x86TranslateRet (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")
    
    EMIT (compilerInfo, ret, x86_RET, 0, 0)
}

static void x86TranslateIn (compilerInfo_t * compilerInfo, ir_t irCommand)
{
    MY_ASSERT (compilerInfo == nullptr, "There is no access to the main structure (compilerInfo)")


}

static void ptrTox86MemoryBuf (compilerInfo_t * compilerInfo, u_int64_t ptr)
{
    compilerInfo->x86_memory_buf = (char *) calloc (SIZE_x86_BUF, sizeof(char));
    MY_ASSERT (compilerInfo->x86_memory_buf == nullptr, "Unable to allocate new memory")

    *(u_int64_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    ptr;
    compilerInfo->machineCode.len += sizeof (u_int64_t);
}

static void x86insertPtr (compilerInfo_t * compilerInfo, u_int32_t ptr)
{
    * (u_int32_t *) (compilerInfo->machineCode.buf + compilerInfo->machineCode.len) = 
                    ptr;
    compilerInfo->machineCode.len += sizeof(u_int32_t);
}