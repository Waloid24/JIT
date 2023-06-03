DEF_CMD (PUSH, 1, YES, 
{
    MY_ASSERT (1, "the command was not detected earlier");
})
DEF_CMD (POP,  2, YES,
{
    MY_ASSERT (1, "the command was not detected earlier");
})
DEF_CMD (ADD,  3,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "add",
        .cmd            = CMD_ADD,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_POP_REG + SIZE_POP_REG + SIZE_ADD_REG_REG + SIZE_PUSH_REG
    };
})
DEF_CMD (SUB,  4,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "sub",
        .cmd            = CMD_SUB,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_POP_REG + SIZE_POP_REG + SIZE_SUB_REG_REG + SIZE_PUSH_REG
    };
})
DEF_CMD (MUL,  5,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "mul",
        .cmd            = CMD_MUL,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_POP_REG + SIZE_CVTSI2SD + SIZE_POP_REG +
                            SIZE_CVTSI2SD + SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM + 
                            SIZE_CVTSI2SD + SIZE_DIVPD + SIZE_DIVPD +
                            SIZE_MULPD + SIZE_MULPD + SIZE_CVTSD2SI +
                            SIZE_PUSH_REG
    };
})
DEF_CMD (DIV,  6,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "div",
        .cmd            = CMD_DIV,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_POP_REG + SIZE_CVTSI2SD + SIZE_POP_REG +
                            SIZE_CVTSI2SD + SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM +
                            SIZE_CVTSI2SD + SIZE_DIVPD + 
                            SIZE_MULPD + SIZE_CVTSD2SI + 
                            SIZE_PUSH_REG
    };
})
DEF_CMD (OUT,  7,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "out",
        .cmd            = CMD_OUT,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_CVTSI2SD + SIZE_PUSH_REG + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_MOV_REG_REG + SIZE_ALIGN_STACK + SIZE_CALL +
                            SIZE_REL_PTR + SIZE_MOV_REG_REG + SIZE_POP_REG + SIZE_POP_RNUM + SIZE_POP_RNUM + SIZE_POP_RNUM + SIZE_ADD_REG_IMMED + SIZE_4BYTE_NUM
    };
})
DEF_CMD (IN,   8,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "in",
        .cmd            = CMD_IN,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_SUB_REG_IMMED + SIZE_4BYTE_NUM + SIZE_MOV_REG_REG + SIZE_PUSH_REG + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_PUSH_RNUM + SIZE_MOV_RBP_RSP + SIZE_ALIGN_STACK + 
                            SIZE_CALL + SIZE_REL_PTR + SIZE_MOV_RSP_RBP + SIZE_POP_REG + SIZE_POP_RNUM + SIZE_POP_RNUM + SIZE_POP_RNUM
    };
})
DEF_CMD (JMP,  9,  YES,
{
    i++;
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "JMP",
        .cmd            = CMD_JMP,
        .nativeSize     = 2,
        .nativeIP       = i-1,
        .argument_type  = LABEL,
        .argument       = compilerInfo->byteCode.buf[i],
        .x86size        = SIZE_JMP + SIZE_REL_PTR
    };
})
DEF_CMD (CALL, 10, YES,
{
    i++;
    // x86ip += SIZE_MOV_RNUM_REG + SIZE_MOV_REG_RNUM;
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "call",
        .cmd            = CMD_CALL,
        .nativeSize     = 2,
        .nativeIP       = i-1,
        .argument_type  = LABEL,
        .argument       = compilerInfo->byteCode.buf[i],
        .x86size        = SIZE_MOV_REG_IMMED + SIZE_8BYTE_NUM + SIZE_MOV_MEM_R14_RAX + 
                            SIZE_ADD_RNUM_IMMED + SIZE_4BYTE_NUM + SIZE_JMP + SIZE_REL_PTR
    };
})
DEF_CMD (RET,  11, NO,
{
    // x86ip += SIZE_MOV_RNUM_REG + SIZE_MOV_REG_RNUM;
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "ret",
        .cmd            = CMD_RET,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_SUB_RNUM_IMMED + SIZE_4BYTE_NUM + SIZE_PUSH_MEM_R14 + SIZE_RET
    };
})
DEF_CMD (JA,   12, YES,
{
    i++;
    JUMP_FORM (JA)
})
DEF_CMD (JB,   13, YES,
{
    i++;
    JUMP_FORM (JB)
})
DEF_CMD (JBE,  14, YES,
{
    i++;
    JUMP_FORM (JBE)
})
DEF_CMD (JGE,  15, YES,
{
    i++;
    JUMP_FORM (JGE)
})
DEF_CMD (JE,   16, YES,
{
    i++;
    JUMP_FORM (JE)
})
DEF_CMD (JNE,  17, YES,
{
    i++;
    JUMP_FORM (JNE)
})
DEF_CMD (HLT,  18, NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "hlt",
        .cmd            = CMD_HLT,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_RET
    };
})
DEF_CMD (SQRT, 19, NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "sqrt",
        .cmd            = CMD_SQRT,
        .nativeSize     = 1,
        .nativeIP       = i,
        .x86size        = SIZE_POP_REG + SIZE_CVTSI2SD + SIZE_MOV_REG_IMMED +
                            SIZE_8BYTE_NUM + SIZE_CVTSI2SD + SIZE_DIVPD + 
                            SIZE_SQRTPD_XMM0_XMM0 + SIZE_MULPD + SIZE_CVTSD2SI + SIZE_PUSH_REG
    };
})
DEF_CMD (MEOW, 20, NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "meow",
        .cmd            = CMD_MEOW,
        .nativeSize     = 1,
        .nativeIP       = i
    };
    MY_ASSERT (1, "This version doesn't support this command (meow), sorry :(")
})
DEF_CMD (BA,   21, NO,
{
    BOOL_EXPR (BA)
})
DEF_CMD (BB,   22, NO,
{
    BOOL_EXPR (BB)
})
DEF_CMD (BBE,  23, NO,
{
    BOOL_EXPR (BBE)
})
DEF_CMD (BGE,  24, NO,
{
    BOOL_EXPR (BGE)
})
DEF_CMD (BE,   25, NO,
{
    BOOL_EXPR (BE)
})
DEF_CMD (BNE,  26, NO,
{
    BOOL_EXPR (BNE)
})
