#ifndef TRANSLATE_COMMAND_HPP
#define TRANSLATE_COMMAND_HPP

#include "lexer.hpp"
#include <string.h>
#include <stdio.h>

typedef struct opcode {
    size_t size;
    u_int64_t code;
} opcode_t;

void JITCompile (compilerInfo_t * compilerInfo);

enum X86_CMD : u_int64_t {    

    //mov r_x, num
    MOV_REG_IMMED = 0xb848,           // need to "|" with shifted by 8 reg mask
                                      // must be followed with 64 bit integer 
    MOV_RNUM_IMMED = 0xb849,
    //mov r_x, r_x
    MOV_REG_REG = 0xc08948,           // first: need to "|" with shited left by 16 reg mask, second - by 19

    MOV_RNUM_REG = 0xc08949,
    MOV_RNUM_RNUM = 0xc0894d,
    MOV_RBP_RSP = 0xe48949,
    MOV_RSP_RBP = 0xe4894c,
    MOV_REG_RNUM = 0xc0894c,          // need to "|" with shifted left by 16 reg mask, second - by 19
    
    MOV_MEM_R14_RAX = 0x068949,       // mov qword [r14], rax

    //mov [r15 + offset], r_x
    MOV_R15_OFFSET_REG = 0x878949,    // the same as above

    PUSH_REG = 0x50,                  // "|" regMask
    PUSH_RNUM = 0x5041,               // need to "|" with shifted left by 8

    //push [r15 + offset]
    PUSH_R15_OFFSET = 0xb7ff41,       // must be followed by 32bit offset

    PUSH_MEM_R14 = 0x36ff41,            //push [r14]
    
    POP_REG = 0x58,                    // "|" regMask
    POP_RNUM = 0x5841,               // need to "|" with shifted left by 8

    //pop [r15 + offset]
    POP_R15_OFFSET = 0x878f41,         // must be followed by 32bit offset

    ADD_REG_REG = 0xc00148,            // first reg - "|" with shifted by 16 REG_MASK, second reg - "|" with shifted by 19 REG_MASK
    ADD_RNUM_REG = 0xc00149,
    ADD_RNUM_RNUM = 0xc0014d,

    SUB_REG_REG = 0xc02948,            // same
    SUB_RNUM_REG = 0xc02949,
    SUB_REG_RNUM = 0xc0294c,
    SUB_RNUM_RNUM = 0xc0294d,

    CVTSI2SD_XMM0_RSP = 0x24042a0ff2,   // mov xmm0, [rsp]

    MULPD = 0xc0590f66,
    DIVPD = 0xc05e0f66,

    SQRTPD_XMM0_XMM0 = 0xc0510f66,      // sqrt(xmm0) -> xmm0 

    RET = 0xC3,                     // ret

    JMP = 0xe9,                     // followed by 32-bit difference (dst - src)

    COND_JMP = 0x000f,              // apply cond_jmp mask shifted by 8

    CALL = 0xe8,

    CMP_REG_REG     = 0xc03948,            // shifted by 16 and by 19 REG_MASK  
    CMP_RNUM_REG    = 0xc03949,
    CMP_REG_RNUM    = 0xc0394c,
    CMP_RNUM_RNUM   = 0xc0394d,

    XOR_RDI_RDI = 0xff3148,

    CALL_PRINTF = 0xfffffd71e8,

    CALL_SCANF = 0xfffffdb6e8,

    ALIGN_STACK = 0xf0e48348,

    SUB_REG_IMMED  = 0xe88148,
    SUB_RNUM_IMMED = 0xe88149,

    ADD_REG_IMMED  = 0xc08148,
    ADD_RNUM_IMMED = 0xc08149,

    ADD_RSP = 0xc48148,
    SUB_RSP = 0xec8148,

    CVTSI2SD_REG  = 0xc02a0f48f2,
    CVTSI2SD_RNUM = 0xc02a0f49f2,

    CVTSD2SI_REG  = 0xc02d0f48f2,
    CVTSD2SI_RNUM = 0xc02d0f49f2
};


enum x86_Commands_Size {
    SIZE_MOV_REG_IMMED = 2,     //ok
    SIZE_MOV_RNUM_IMMED = 2,    //ok
    SIZE_MOV_REG_REG = 3,       //ok
    SIZE_MOV_RNUM_REG = 3,      //ok
    SIZE_MOV_RNUM_RNUM = 3,     //ok
    SIZE_MOV_RBP_RSP = 3,       //ok   
    SIZE_MOV_MEM_R14_RAX = 3,   //ok
    SIZE_MOV_RSP_RBP = 3,       //ok
    SIZE_MOV_REG_RNUM = 3,      //ok
    SIZE_MOV_R15_OFFSET_REG = 3,//ok
    SIZE_PUSH_REG = 1,          //ok
    SIZE_PUSH_MEM_R14 = 3,
    SIZE_PUSH_RNUM = 2,         //ok
    SIZE_PUSH_R15_OFFSET = 3,   //ok
    SIZE_POP_REG = 1,           //ok
    SIZE_POP_RNUM = 2,          //okFAN
    SIZE_POP_R15_OFFSET = 3,    //ok
    SIZE_ADD_REG_REG = 3,       //ok
    SIZE_ADD_RNUM_REG = 3,      //ok
    SIZE_ADD_RNUM_RNUM = 3,     //ok
    SIZE_SUB_REG_REG = 3,       //ok
    SIZE_SUB_REG_RNUM = 3,
    SIZE_SUB_RNUM_REG = 3,
    SIZE_SUB_RNUM_RNUM = 3,
    SIZE_SUB_R14_8 = 4,
    SIZE_SQRTPD_XMM0_XMM0 = 4,
    SIZE_MULPD = 4,
    SIZE_DIVPD = 4,
    SIZE_RET = 1,               //ok
    SIZE_JMP = 1,
    SIZE_COND_JMP = 2,
    SIZE_CALL = 1,

    SIZE_CMP_REG_REG = 3,
    SIZE_CMP_RNUM_REG = 3,
    SIZE_CMP_REG_RNUM = 3,
    SIZE_CMP_RNUM_RNUM = 3, 

    SIZE_4BYTE_NUM = 4,
    SIZE_8BYTE_NUM = 8,
    SIZE_REL_PTR = 4,
    SIZE_ABS_PTR = 8,
    SIZE_ALIGN_STACK = 4,

    SIZE_SUB_REG_IMMED = 3,
    SIZE_SUB_RNUM_IMMED = 3,
    SIZE_ADD_REG_IMMED = 3,
    SIZE_ADD_RNUM_IMMED = 3,

    SIZE_CVTSI2SD_XMM0_RSP = 5,
    SIZE_CVTSI2SD = 5,
    SIZE_CVTSD2SI = 5
};

enum REG_MASK {
    RAX = 0,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
    R8 = 0,
    R9, 
    R10,
    R11,
    R12, 
    R13,
    R14,
    R15,
    XMM0 = 0,
    XMM1,
    XMM2
};


enum COND_JMPS {
    JE_MASK = 0x84,
    JNE_MASK = 0x85,
    JG_MASK = 0x8f,
    JGE_MASK = 0x8d,
    JL_MASK = 0x8c,
    JLE_MASK = 0x8e,
};


#endif