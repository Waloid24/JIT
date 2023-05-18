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
        .nativeIP       = i
    };
})
DEF_CMD (SUB,  4,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "sub",
        .cmd            = CMD_SUB,
        .nativeSize     = 1,
        .nativeIP       = i
    };
})
DEF_CMD (MUL,  5,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "mul",
        .cmd            = CMD_MUL,
        .nativeSize     = 1,
        .nativeIP       = i
    };
})
DEF_CMD (DIV,  6,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "div",
        .cmd            = CMD_DIV,
        .nativeSize     = 1,
        .nativeIP       = i
    };
})
DEF_CMD (OUT,  7,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "out",
        .cmd            = CMD_OUT,
        .nativeSize     = 1,
        .nativeIP       = i
    };
})
DEF_CMD (IN,   8,  NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "in",
        .cmd            = CMD_IN,
        .nativeSize     = 1,
        .nativeIP       = i
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
        .argument       = compilerInfo->byteCode.buf[i]
    };
})
DEF_CMD (CALL, 10, YES,
{
    i++;
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "call",
        .cmd            = CMD_CALL,
        .nativeSize     = 2,
        .nativeIP       = i-1,
        .argument_type  = LABEL,
        .argument       = compilerInfo->byteCode.buf[i]
    };
})
DEF_CMD (RET,  11, NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "ret",
        .cmd            = CMD_RET,
        .nativeSize     = 1,
        .nativeIP       = i
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
        .nativeIP       = i
    };
})
DEF_CMD (SQRT, 19, NO,
{
    compilerInfo->irInfo.irArray[numCmds] = {
        .name           = "sqrt",
        .cmd            = CMD_SQRT,
        .nativeSize     = 1,
        .nativeIP       = i
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
