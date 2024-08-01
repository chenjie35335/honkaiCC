#include "../../../include/backend/AsmInst/Instruction.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include <cassert>


AsmInst *AsmInst::CreateAdd(uint32_t rd, uint32_t rs1, uint32_t rs2){
    string label = "";
    AsmInst *inst = new AsmInst(RISCV_ADD,rs1,rs2,rd,0,0.0,0,0,0,label);
    return inst;
}

AsmInst *AsmInst::CreateSub(uint32_t rd, uint32_t rs1, uint32_t rs2){
    string label = "";
    AsmInst *inst = new AsmInst(RISCV_SUB,rs1,rs2,rd,0,0.0,0,0,0,label);
    return inst;
}

AsmInst *AsmInst::CreateMul(uint32_t rd, uint32_t rs1, uint32_t rs2){
    string label = "";
    AsmInst *inst = new AsmInst(RISCV_MUL,rs1,rs2,rd,0,0.0,0,0,0,label);
    return inst;
}

AsmInst *AsmInst::CreateDiv(uint32_t rd, uint32_t rs1, uint32_t rs2){
    string label = "";
    AsmInst *inst = new AsmInst(RISCV_DIV,rs1,rs2,rd,0,0.0,0,0,0,label);
    return inst;
}

AsmInst *AsmInst::CreateRem(uint32_t rd, uint32_t rs1, uint32_t rs2){
    string label = "";
    AsmInst *inst = new AsmInst(RISCV_REM,rs1,rs2,rd,0,0.0,0,0,0,label);
    return inst;
}

AsmInst * AsmInst::CreateLa(uint32_t rd,string label){
    AsmInst *inst = new AsmInst(RISCV_LA,0,0,rd,0,0.0,0,0,0,label);
    return inst;
}

AsmInst * AsmInst::CreateLw(uint32_t rd,uint32_t rs1,int imm){
    AsmInst *inst = new AsmInst(RISCV_LW,rs1,0,rd,imm,0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateSw(uint32_t rd,uint32_t rs1,int imm){
    AsmInst *inst = new AsmInst(RISCV_LW,rs1,0,rd,imm,0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateLi(uint32_t rd,int imm){
    AsmInst *inst = new AsmInst(RISCV_LI,0,0,rd,imm,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateAddi(uint32_t rd, uint32_t rs1, int imm){
    AsmInst *inst = new AsmInst(RISCV_ADDI,rs1,0,rd,imm,0.0,0,0,0,"");
    return inst;
}

AsmInst *AsmInst::CreateCall(string label){
    AsmInst *inst = new AsmInst(RISCV_CALL,0,0,0,0,0.0,0,0,0,label);
    return inst;
}

AsmInst * AsmInst::CreateRet(){
    AsmInst *inst = new AsmInst(RISCV_RET,0,0,0,0,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateXor(uint32_t rd, uint32_t rs1, uint32_t rs2){
    AsmInst *inst = new AsmInst(RISCV_XOR,rs1,rs2,rd,0,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateSeqz(uint32_t rd, uint32_t rs1){
    AsmInst *inst = new AsmInst(RISCV_SEQZ,rs1,0,rd,0,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateSlli(uint32_t rd, uint32_t rs1, uint32_t shamt){
    AsmInst *inst = new AsmInst(RISCV_SLLI,rs1,0,rd,shamt,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateSnez(uint32_t rd, uint32_t rs1){
    AsmInst *inst = new AsmInst(RISCV_SNEZ,rs1,0,rd,0,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateSlt(uint32_t rd, uint32_t rs1, uint32_t rs2){
    AsmInst *inst = new AsmInst(RISCV_SLT,rs1,rs2,rd,0,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateOr(uint32_t rd, uint32_t rs1, uint32_t rs2){
    AsmInst *inst = new AsmInst(RISCV_OR,rs1,rs2,rd,0,0.0,0,0,0,"");
    return inst;
}

AsmInst * AsmInst::CreateFadds(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FADDS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFsubs(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FSUBS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFmuls(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FMULS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFdivs(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FDIVS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFges(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FGES,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFgts(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FGTS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFles(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FLTS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateFeqs(uint32_t frd, uint32_t frs1, uint32_t frs2){
    AsmInst *inst = new AsmInst(RISCV_FEQS,0,0,0,0,0.0,frs1,frs2,frd,"");
    return inst;
}

AsmInst * AsmInst::CreateBnez(uint32_t rs1,string label){
    AsmInst *inst = new AsmInst(RISCV_BNEZ,rs1,0,0,0,0.0,0,0,0,label);
    return inst;
}

AsmInst * AsmInst::CreateJ(string label){
    AsmInst *inst = new AsmInst(RISCV_J,0,0,0,0,0.0,0,0,0,label);
    return inst;
}

AsmInst * AsmInst::CreateMv(uint32_t rd,int rs1){
    AsmInst *inst = new AsmInst(RISCV_MV,rs1,0,rd,0,0.0,0,0,0,"");
    return inst;
}

ostream & operator << (ostream &os,const AsmInst &inst){
    auto type = inst.Op;
    string rd = RegisterManager::regs[inst.rd],rs1 = RegisterManager::regs[inst.rs1],rs2 = RegisterManager::regs[inst.rs2];
    string frd = RegisterManager::fregs[inst.frd],frs1 = RegisterManager::fregs[inst.frs1],frs2 = RegisterManager::fregs[inst.frs2];
    switch(type){
        case RISCV_ADD:
            os << "\t" << "add " << rd << ", " << rs1 << ", " << rs2;
            break;
        case RISCV_SUB:
            os << "\t" << "sub " << rd << ", " << rs1 << ", " << rs2;
            break;
        case RISCV_ADDI:
            os << "\t" << "addi " << rd << ", " << rs1 << ", " << inst.imm;
            break;
        case RISCV_SLT:
            os << "\t" << "slt " << rd << ", " << rs1 << ", " << rs2;
            break;
        case RISCV_LA:
            os << "\t" << "li " << rd << ", " << inst.imm << endl;
            break;
        case RISCV_XOR:
            os << "\t" << "xor " << rd << ", " << rs1 << ", " << rs2 << endl;
            break;
        case RISCV_MV:
            os << "\t" << "mv " << rd << ", " << rs1 << endl;
            break;
        case RISCV_LW:
            os << "\t" << "lw " << rd << ", " << inst.imm << "(" << rs1 << ")" << endl;
            break;
        case RISCV_SW:
            os << "\t" << "sw " << rd << ", " << inst.imm << "(" << rs1 << ")" << endl;
            break;
        case RISCV_LI:
            os << "\t" << "li " << rd << ", " << inst.imm << endl;
            break;
        case RISCV_CALL:
            os << "\t" << "call " << inst.label << endl;
            break;
        case RISCV_SLLI:
            os << "\t" << "slli " << rd << ", " << rs1 << ", " << inst.imm<<endl;
            break;
        case RISCV_MUL:
            os << "\t" << "mul " << rd << ", " << rs1 << ", " << rs2 << endl;
            break;
        case RISCV_RET:
            os << "\t" << "ret " << endl;
            break;
        case RISCV_SEQZ:   
            os << "\t" << "seqz " << rd << ", " << rs1 << endl;
            break;
        case RISCV_SNEZ:
            os << "\t" << "snez " << rd << "," << rs1 << endl;
            break;
        case RISCV_DIV:
            os << "\t" << "div " << rd << ", " << rs1 << ", " << rs2 << endl;
            break;
        case RISCV_REM:   
            os << "\t" << "rem " << rd << ", " << rs1 << ", " << rs2 << endl;
            break;
        case RISCV_OR:
            os << "\t" << "or " << rd << ", " << rs1 << ", " << rs2 << endl;
            break;
        case RISCV_FADDS:
            os << "\t" << "fadd.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_FSUBS:   
            os << "\t" << "fsub.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_FMULS:
            os << "\t" << "fmul.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_FDIVS:
            os << "\t" << "fdiv.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_FGES:   
            os << "\t" << "fge.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_FLTS:   
            os << "\t" << "flt.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_FEQS:
            os << "\t" << "feq.s " << frd << ", " << frs1 << ", " << frs2 << endl;
            break;
        case RISCV_BNEZ:   
            os << "\t" << "bnez " << frs1 << ", " << inst.label << endl;
            break;
        case RISCV_J:   
            os << "\t" << "j " << inst.label << endl;
            break;
        default: {
            cerr << "Unsupported instruction" << endl;
            assert(0);
        }
    }
    return os;
}