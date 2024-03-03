#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include <cassert>
#include <iostream>

//defination
void Visit(const RawValue* &value,string &sign) {       
    const auto& kind = value->value;
    switch(kind->tag) {
    case RVT_RETURN: {
        //printf("parse return\n");
        Visit(kind->data.ret,sign);
        cout << "  addi sp, sp, 256" <<  endl;
        cout << "  ret" << endl;
        break;
    }
    case RVT_INTEGER: {
        //printf("parse integer\n");
        Visit(kind->data.integer,sign);
        //reg_alloc.insert(pair<koopa_raw_value_t,string>(value,s
        break;
    }
    case RVT_BINARY: {
        //printf("parse binary\n");
        Visit(kind->data.binary,sign);
        //reg_alloc.insert(pair<koopa_raw_value_t,string>(value,s
        break;
    }
    case RVT_ALLOC: {
        //printf("parse alloc\n");
        //int offset = alloc_stack();
        //stack_alloc.insert(pair<koopa_raw_value_t,int>(value,of
        break;
    }
    case RVT_LOAD: {
        //printf("parse load\n");
        Visit(kind->data.load,sign);
        //reg_alloc.insert(pair<koopa_raw_value_t,string>(value,s
        break;
    }
    case RVT_STORE: {
        //printf("parse store\n");
        Visit(kind->data.store,sign);
        break;
    }
    default:
        assert(false);
}
}
}

void Visit(const RawBasicBlock* &bb,string &sign){
     Visit(bb->insts,sign);
} 

void Visit(const RawFunction* &func,string &sign)
{
         printf("  .globl %s\n",func->name+1);
         printf("%s:\n",func->name+1);
         cout << "  addi sp, sp, -256" <<  endl;
         Visit(func->bbs,sign);
       
}
//Visit RawSlice
void Visit(const  RawSlice &slice,string &sign){
    for(size_t i = 0; i < slice.len; i++) {
        //cout << endl;
        //printf("slice.len == %d, i == %d\n",slice.len,i);
        //auto ptr = slice.buffer[slice.len-1];
        auto ptr = slice.buffer[i];
        switch(slice.kind) {
            case RSK_FUNCTION:
                //printf("begin parse function\n");
                Visit(reinterpret_cast<const RawFunction *&>(ptr),sign);
                break;
            case RSK_BASICBLOCK:
                //printf("begin parse block\n");
                Visit(reinterpret_cast<const RawBasicBlock *&>(ptr),sign);break;
            case RSK_BASICVALUE:
                //printf("begin parse value\n");
                Visit(reinterpret_cast<const RawValue * &>(ptr),sign);break;
            default:
                assert(false);
        }
    }
}









