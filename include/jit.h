#ifndef JIT_H_INCLUDED
#define JIT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

enum CMD_TYPES {

    IMMED = ARG_IMMED,

    REG = ARG_REG,
    REG_IMMED = ARG_REG | ARG_IMMED,

    RAM_IMMED = ARG_RAM | ARG_IMMED,
    RAM_REG = ARG_RAM | ARG_REG,
    RAM_REG_IMMED = RAM_REG | ARG_IMMED,

} ;

enum X86_CMD : u_int64_t {    

    //mov r_x, num
    MOV_REG_IMMED = 0xb848,           // need to "|" with shifted by 8 reg mask
                                      // must be followed with 64 bit integer 
    //mov r_x, r_x
    MOV_REG_REG = 0xc08948,           // first: need to "|" with shited left by 16 reg mask, second - by 19

    //mov r_x, [r15 + offset]
    MOV_REG_R15_OFFSET = 0x498b87,    // need to "|" with shited left by 19 reg mask
                                      // must be followed with 32 bit offset of r15
    //mov [r15 + offset], r_x
    MOV_R15_OFFSET_REG = 0x878949,    // the same as above

    PUSH_REG = 0x50,                  // "|" regMask

    //push [r15 + offset]
    PUSH_R15_OFFSET = 0xb7ff41,       // must be followed by 32bit offset

    POP_REG = 0x58,                    // "|" regMask

    //pop [r15 + offset]
    POP_R15_OFFSET = 0x878f41,         // must be followed by 32bit offset

    ADD_REG_REG = 0xc00148,            // first reg - "|" with shifted by 19 REG_MASK, second reg - "|" with shifted by 16 REG_MASK

    SUB_REG_REG = 0xc02948,            // same

    IMUL_REG_REG = 0xc0af0f48,         // first reg - "|" with shifted by 27 REG_MASK, second reg - "|" with shifted by 24 REG_MASK

    CQO = 0x9948,                      // signed rax -> (rdx, rax)

    IDIV_REG = 0xf8f748,               //  "|" with shifted by 16 REG_MASK


    /* sqrt: 
            mov rax, double (*1000) // 1 + 8
            cvtsi2sd xmm0, rax      // 5 
            mov rax, 1000           // 1 + 8
            cvtsi2sd xmm1, rax      // 5
            div xmm0, xmm1          // 4        amount = 45
            sqrtpd(xmm0)            // 4
            mul xmm0, xmm1          // 4
            cvtsi2sd rax, xmm0      // 5
    */

    CVTSI2SD_XMM0_RAX = 0xc02a0f48f2,   // mov xmm0, rax
    CVTSD2SI_RAX_XMM0 = 0xc02d0f48f2,   // mov rax, xmm0
    CVTSI2SD_XMM1_RAX = 0xc82a0f48f2,   // mov xmm1, rax

    DIVPD_XMM0_XMM1 = 0xc15e0f66,       // xmm0/xmm1 -> xmm0
    MULPD_XMM0_XMM1 = 0xc1590f66,       // xmm0*xmm1 -> xmm0

    SQRTPD_XMM0_XMM0 = 0xc0510f66,      // sqrt(xmm0) -> xmm0 

    x86_RET = 0xC3,                     // ret

    x86_JMP = 0xe9,                     // followed by 32-bit difference (dst - src)

    x86_COND_JMP = 0x000f,              // apply cond_jmp mask shifted by 8

    x86_CALL = 0xe8,

    CMP_REG_REG =  0xc03948,            // shifted by 19 and by 16 REG_MASK                     

};


enum x86_Commands_Size {
    SIZE_MOV_REG_IMMED = 2,
    SIZE_MOV_REG_REG = 3,
    SIZE_MOV_REG_R15_OFFSET = 3,
    SIZE_MOV_R15_OFFSET_REG = 3,
    SIZE_PUSH_REG = 1,
    SIZE_PUSH_R15_OFFSET = 3,
    SIZE_POP_REG = 1,
    SIZE_POP_R15_OFFSET = 3,
    SIZE_ADD_REG_REG = 3,
    SIZE_SUB_REG_REG = 3,
    SIZE_IMUL_REG_REG = 4,
    SIZE_CQO = 2,
    SIZE_IDIV_REG = 3,
    SIZE_CVTSI2SD_XMM0_RAX = 5,
    SIZE_CVTSD2SI_RAX_XMM0 = 5,
    SIZE_SQRTPD_XMM0_XMM0 = 4,
    SIZE_DIVPD_XMM0_XMM1 = 4,
    SIZE_MULPD_XMM0_XMM1 = 4,
    SIZE_CVTSI2SD_XMM1_RAX = 5,
    SIZE_x86_RET = 1,
    SIZE_x86_JMP = 1,
    SIZE_x86_COND_JMP = 2,
    SIZE_x86_CALL = 1,
    SIZE_CMP_REG_REG = 3,
};

enum REG_MASK {
    RAX,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
};

enum COND_JMPS {
    JE_MASK = 0x84,
    JNE_MASK = 0x85,
    JG_MASK = 0x8f,
    JGE_MASK = 0x8d,
    JL_MASK = 0x8c,
    JLE_MASK = 0x8e,
};

void fillIRArray(IR_HEAD_T* IR_HEAD);
void IR_CTOR(IR_HEAD_T* IR_HEAD, size_t cmdCt);
void IR_DTOR(IR_HEAD_T*);
void readByteCode(FILE* byteCode, IR_HEAD_T* IR_HEAD);
size_t getCodeSize(FILE* byteCode);
void fillIRArrayCommand(IR_HEAD_T* IR_HEAD, const char* name, int arg);
void HandleArgsCmd(IR_HEAD_T* IR_HEAD);
void IR_Dump(IR_HEAD_T* IR_HEAD);
int isJump(int num);
int isPushPop(int num);
char* getArgType(int argType);


void SetX86CMD(IR_HEAD_T* IR_HEAD, x86CMDarr_T* x86CMDarr);

void x86CMD_CTOR(x86CMDarr_T* x86CMDarr, size_t size);

#endif