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

    //mov r_x, [r15 + offset]
    MOV_REG_R15_OFFSET = 0x498b87,    // need to "|" with shited left by 19 reg mask
                                      // must be followed with 32 bit offset of r15
    //mov [r15 + offset], r_x
    MOV_R15_OFFSET_REG = 0x878949,    // the same as above

    PUSH_REG = 0x50,                  // "|" regMask
    PUSH_RNUM = 0x5041,               // need to "|" with shifted left by 8

    //push [r15 + offset]
    PUSH_R15_OFFSET = 0xb7ff41,       // must be followed by 32bit offset

    PUSHA1 = 0x544155415641,
    PUSH_MEM_R14 = 0x36ff41,            //push [r14]
    PUSHA2 = 0x5741534153,
    POPA1  = 0x5f415e415d415c41,
    POPA2  = 0x5b5b41,
    
    POP_REG = 0x58,                    // "|" regMask
    POP_RNUM = 0x5841,               // need to "|" with shifted left by 8

    //pop [r15 + offset]
    POP_R15_OFFSET = 0x878f41,         // must be followed by 32bit offset

    ADD_REG_REG = 0xc00148,            // first reg - "|" with shifted by 19 REG_MASK, second reg - "|" with shifted by 16 REG_MASK
    ADD_RNUM_REG = 0xc00149,
    ADD_RNUM_RNUM = 0xc0014d,
    ADD_R14_8 = 0x08c68349,             // for callStack
    SUB_R14_8 = 0x08ee8349,

    SUB_REG_REG = 0xc02948,            // same

    IMUL_REG_REG = 0xc0af0f48,         // first reg - "|" with shifted by 27 REG_MASK, second reg - "|" with shifted by 24 REG_MASK

    CQO = 0x9948,                      // signed rax -> (rdx, rax)

    IDIV_REG = 0xf8f748,               //  "|" with shifted by 16 REG_MASK


    /* sqrt: 
            mov rax, double (*1000)
            cvt xmm0, rax
            mov rax, 1000
            cvt xmm1, rax
            div xmm0, xmm1
            sqrtpd(xmm0)
            mul xmm0, xmm1
            cvt rax, xmm0
        */

    CVTSI2SD_XMM0_RAX = 0xc02a0f48f2,   // mov xmm0, rax
    CVTSD2SI_RAX_XMM0 = 0xc02d0f48f2,   // mov rax, xmm0
    CVTSI2SD_XMM1_RAX = 0xc82a0f48f2,   // mov xmm1, rax
    CVTSI2SD_XMM2_RAX = 0xd02a0f48f2,
    CVTSI2SD_XMM0_RSP = 0x24042a0ff2,   // mov xmm0, [rsp]

    DIVPD_XMM0_XMM0 = 0xc05e0f66,       // xmm0/xmm0 -> xmm0
                                        // first reg - "|" with shifted by 27 REG_MASK, second reg - "|" with shifted by 24 REG_MASK

    MULPD_XMM0_XMM0 = 0xc0590f66,       // xmm0*xmm0 -> xmm0
                                        // first reg - "|" with shifted by 27 REG_MASK, second reg - "|" with shifted by 24 REG_MASK

    SQRTPD_XMM0_XMM0 = 0xc0510f66,      // sqrt(xmm0) -> xmm0 

    x86_RET = 0xC3,                     // ret

    x86_JMP = 0xe9,                     // followed by 32-bit difference (dst - src)

    x86_COND_JMP = 0x000f,              // apply cond_jmp mask shifted by 8

    x86_CALL = 0xe8,

    CMP_REG_REG =  0xc03948,            // shifted by 19 and by 16 REG_MASK   

    SYSCALL = 0x050f, 

    XOR_RDI_RDI = 0xff3148,

    CALL_PRINTF = 0xfffffd71e8,

    CALL_SCANF = 0xfffffdb6e8,

    AND_RSP_10 = 0xffffff10e48148,
    ALIGN_STACK = 0xf0e48348,

    SUB_RSP_8  = 0x08ec8348,
    ADD_RSP_8  = 0x08c48348,  
    LEA_RDI_RSP = 0x00247c8d48,

    ADD_RSP = 0xc48148,
    SUB_RSP = 0xec8148
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
    SIZE_MOV_REG_R15_OFFSET = 3,//ok
    SIZE_MOV_R15_OFFSET_REG = 3,//ok
    SIZE_PUSH_REG = 1,          //ok
    SIZE_PUSH_MEM_R14 = 3,
    SIZE_PUSHA1 = 6,
    SIZE_PUSHA2 = 5,
    SIZE_POPA1 = 8,
    SIZE_POPA2 = 3,
    SIZE_PUSH_RNUM = 2,         //ok
    SIZE_PUSH_R15_OFFSET = 3,   //ok
    SIZE_POP_REG = 1,           //ok
    SIZE_POP_RNUM = 2,          //okFAN
    SIZE_POP_R15_OFFSET = 3,    //ok
    SIZE_ADD_R14_8 = 4,         //ok
    SIZE_ADD_REG_REG = 3,       //ok
    SIZE_ADD_RNUM_REG = 3,      //ok
    SIZE_ADD_RNUM_RNUM = 3,     //ok
    SIZE_SUB_REG_REG = 3,       //ok
    SIZE_SUB_R14_8 = 4,
    SIZE_IMUL_REG_REG = 4,      
    SIZE_CQO = 2,
    SIZE_IDIV_REG = 3,
    SIZE_CVTSI2SD_XMM0_RAX = 5,
    SIZE_CVTSI2SD_XMM1_RAX = 5,
    SIZE_CVTSI2SD_XMM2_RAX = 5,
    SIZE_CVTSD2SI_RAX_XMM0 = 5,
    SIZE_CVTSI2SD_XMM0_RSP = 5,
    SIZE_SQRTPD_XMM0_XMM0 = 4,
    SIZE_DIVPD_XMM0_XMM0 = 4,
    SIZE_MULPD_XMM0_XMM0 = 4,
    SIZE_x86_RET = 1,               //ok
    SIZE_x86_JMP = 1,
    SIZE_x86_COND_JMP = 2,
    SIZE_x86_CALL = 1,
    SIZE_CMP_REG_REG = 3,
    SIZE_NUM = 8,
    SIZE_REL_PTR = 4,
    SIZE_ABS_PTR = 8,
    SIZE_SYSCALL = 2,
    SIZE_XOR_RDI_RDI = 3,
    SIZE_ALIGN_STACK = 4,
    SIZE_SUB_RSP_8  = 4,
    SIZE_ADD_RSP_8  = 4,
    SIZE_LEA_RDI_RSP = 5,
    SIZE_ADD_RSP = 3,
    SIZE_SUB_RSP = 3, 
    SIZE_AND_RSP_10 = 7
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