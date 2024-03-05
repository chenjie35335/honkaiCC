#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/hardwareManager.h"
#include <cassert>
#include <iostream>

//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
void Visit(const RawValueP &value) {    
    if(IsRegister(value)) {
        return;
    }  
    else if(IsMemory(value)) {
        LoadFromMemory(value);
        return;
    }
    else {
    const auto& kind = value->value;
    switch(kind->tag) {
    case RVT_RETURN: {
        const auto& ret = kind->data.ret.value; 
        Visit(ret);
        const char *RetRegister = GetRegister(ret);
        cout << "  mv   a0, "<< RetRegister << endl;
        cout << "  addi sp, sp, 256" <<  endl;
        cout << "  ret" << endl;
        break;
    }
    case RVT_INTEGER: {
        const auto& integer = kind->data.integer.value;
        if(value == 0) {
            AllocX0(value);
        } else {
            AllocRegister(value);
            const char *reg = GetRegister(value);
            cout << "  li"  <<  reg  << ","  << integer << endl;
        }
        break;
    }
    case RVT_BINARY: {
        break;
    }
    case RVT_ALLOC: {
        break;
    }
    case RVT_LOAD: {
        break;
    }
    case RVT_STORE: {
        break;
    }
    default:
        assert(false);
    }
}
}

// Visit RawBlock
void Visit(const RawBasicBlockP &bb){
    //const &RawSlice *value = bb->insts;
     Visit(bb->insts);
} 
// Visit RawFunction
void Visit(const RawFunctionP &func)
{
         printf("  .globl %s\n",func->name+1);
         printf("%s:\n",func->name+1);
         cout << "  addi sp, sp, -256" <<  endl;
         Visit(func->bbs);
       
}
//Visit RawSlice
void Visit(const RawSlice &slice){
    for(size_t i = 0; i < slice.len; i++) {
        //cout << endl;
        //printf("slice.len == %d, i == %d\n",slice.len,i);
        //auto ptr = slice.buffer[slice.len-1];
        auto ptr = slice.buffer[i];
        switch(slice.kind) {
            case RSK_FUNCTION:
                //printf("begin parse function\n");
                Visit(reinterpret_cast<RawFunctionP>(ptr));
                break;
            case RSK_BASICBLOCK:
                //printf("begin parse block\n");
                Visit(reinterpret_cast<RawBasicBlockP>(ptr));break;
            case RSK_BASICVALUE:
                //printf("begin parse value\n");
                Visit(reinterpret_cast<RawValueP>(ptr));break;
            default:
                assert(false);
        }
    }
}

void generateASM(const RawProgramme& value) {
    cout << "\ttext" << endl;
    Visit(value.Funcs);
}









