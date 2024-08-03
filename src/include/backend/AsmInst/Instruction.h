#ifndef STORMY_ASM
#define STORMY_ASM
#define SP 2
#define T0 5
#include <iostream>
#include <vector>
using namespace std;
enum Type{
            RISCV_LA, //la指令
            RISCV_MV, //lw指令
            RISCV_LW, //lw指令
            RISCV_SW, //sw指令
            RISCV_LI, //li指令
            RISCV_ADDI, //addi指令
            RISCV_CALL, //call指令
            RISCV_SLLI, //slli指令
            RISCV_MUL, //mul指令
            RISCV_ADD, //add指令
            RISCV_SUB, //sub指令
            RISCV_RET, //ret指令
            RISCV_XOR, //xor指令
            RISCV_SEQZ, //seqz指令
            RISCV_SNEZ, //snez指令
            RISCV_DIV, //div指令
            RISCV_REM, //rem指令
            RISCV_SLT, //slt指令
            RISCV_OR,//or指令
            RISCV_FADDS, //fadd.s指令
            RISCV_FSUBS, //fsub.s指令
            RISCV_FMULS, //fmul.s指令
            RISCV_FDIVS, //fdiv.s指令
            RISCV_FGES, //fge.s指令
            RISCV_FGTS, //fgt.s指令
            RISCV_FLTS, //flt.s指令
            RISCV_FEQS, //feq.s指令
            RISCV_BNEZ, //bneq指令
            RISCV_J,     //j指令
            RISCV_SD,     //sd指令
            RISCV_LD,     //ld指令
        };
//个人感觉还是应该单独来看方便一点
class AsmInst {//首先我要确定一下inst里面有什么，首先是指令的种类
    public:
        Type Op;
        uint32_t rs1;//第一个操作数(整型)
        uint32_t rs2;//第二个操作数（整型）
        uint32_t rd;//第三个操作数（整型）
        int imm;//整型立即数（可能会使用到其他指令，再添加也不迟）
        float fimm;//单精度浮点点立即数
        uint32_t frs1;//浮点指令第一个操作数
        uint32_t frs2;//浮点指令第二个操作数
        uint32_t frd;//浮点指令第三个操作数
        string label;//跳转指令使用

    AsmInst() {} 
    AsmInst(Type op, uint32_t rs1, uint32_t rs2, uint32_t rd, int imm,float fimm,uint32_t frs1,uint32_t frs2,uint32_t frd,string label){
        this->Op = op;
        this->rs1 = rs1;
        this->rs2 = rs2;
        this->rd = rd;
        this->imm = imm;
        this->fimm = fimm;
        this->frs1 = frs1;
        this->frs2 = frs2;
        this->frd = frd;
        this->label = label;
    }
     //这里可能和标准指令的格式不太一样
    static AsmInst * CreateLa(uint32_t rd,string label);//创建La指令
    static AsmInst * CreateLw(uint32_t rd,uint32_t rs1,int imm);//创建lw指令
    static AsmInst * CreateLw(uint32_t rd,string label);
    static AsmInst * CreateLd(uint32_t rd, uint32_t rs1, int imm); //创建ld指令
    static AsmInst * CreateSw(uint32_t rd,uint32_t rs1,int imm);
    static AsmInst *CreateSd(uint32_t rd, uint32_t rs1, int imm);
    // 创建sw指令
    static AsmInst * CreateLi(uint32_t rd,int imm);//创建li指令
    static AsmInst * CreateMv(uint32_t rd,int rs1);//创建mv指令
    static AsmInst * CreateAddi(uint32_t rd, uint32_t rs1, int imm);//创建addi指令
    static AsmInst * CreateCall(string label);//创建call指令(这里确实只需要一个label)
    static AsmInst * CreateSlli(uint32_t rd, uint32_t rs1, uint32_t shamt);//创建slli指令
    static AsmInst * CreateMul(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建mul指令
    static AsmInst * CreateAdd(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建add指令
    static AsmInst * CreateSub(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建sub指令
    static AsmInst * CreateRet();//创建ret指令
    static AsmInst * CreateXor(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建xor指令
    static AsmInst * CreateSeqz(uint32_t rd, uint32_t rs1);//创建seqz指令
    static AsmInst * CreateSnez(uint32_t rd, uint32_t rs1);//创建snez指令
    static AsmInst * CreateDiv(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建div指令
    static AsmInst * CreateRem(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建rem指令
    static AsmInst * CreateSlt(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建slt指令
    static AsmInst * CreateOr(uint32_t rd, uint32_t rs1, uint32_t rs2);//创建or指令
    static AsmInst * CreateFadds(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建fadd.s指令
    static AsmInst * CreateFsubs(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建fsub.s指令
    static AsmInst * CreateFmuls(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建fmul.s指令
    static AsmInst * CreateFdivs(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建fdiv.s指令
    static AsmInst * CreateFges(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建fge.s指令
    static AsmInst * CreateFgts(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建fgt.s指令
    static AsmInst * CreateFles(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建flt.s指令
    static AsmInst * CreateFeqs(uint32_t frd, uint32_t frs1, uint32_t frs2);//创建feq.s指令
    static AsmInst * CreateBnez(uint32_t rs1,string label);//创建bneq指令
    static AsmInst * CreateJ(string label);//创建j指令

    //需要重载<<操作符来让cout可以直接输出AsmInst
    friend ostream & operator<<(ostream &os, const AsmInst &inst);
};

#endif