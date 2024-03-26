#include "../../../include/midend/ValueTable/ValueTable.h"

int ScopeLevel = 0;
IdentTableNode* IdentTable = nullptr;
FuncTable funcTable;
IdentTableNode *t;

void IdentTableNode::addValue(const string &ident, int value)
{
    ConstTable.insert(pair<string, int>(ident, value));
}

void IdentTableNode::addVariable(const string &ident, int value){
    VarTable.insert(pair<string, int>(ident, value));
}

 bool IdentTableNode::findValue(const string &ident)
 {
     return ConstTable.find(ident) == ConstTable.end();
 }

 bool IdentTableNode::findVariable(const string &ident)
{
    return VarTable.find(ident) == VarTable.end();
}

void IdentTableNode::ValueMultDef(const string &ident)
{
    if (!findValue(ident))
    {
        cerr << '"' << ident << '"' << "has been defined" << endl;
        exit(-1);
    }
}

void IdentTableNode::VarContrdef(const string &ident)
{
    if (!findValue(ident))
    {
        cerr << '"' << ident << '"' << " has been defined as constant" << endl;
        exit(-1);
    }
}

void IdentTableNode::VarMultDef(const string &ident)
{
    if (!findVariable(ident))
    {
        cerr << '"' << ident << '"' << " redefined" << endl;
        exit(-1);
    }
}

void IdentTableNode::ValueAlter(const string &ident)
{
    if (!findValue(ident))
    {
        cerr << '"' << ident << " is a constant and can't be altered" << endl;
        exit(-1);
    }
}

void IdentTableNode::VarAlter(const string &ident, const string &sign2, int value)
{
    ValueAlter(ident);
    if (!findVariable(ident))
    {
        cout << "  "
             << "store " << sign2 << ","
             << "@" + ident + "_" + to_string(this->level) << endl;
        VarTable[ident] = value;
        return;
    }
    else if (this->father == nullptr)
    {
        cerr << '"' << ident << "is not defined" << endl;
        exit(-1);
    }
    else
    {
        this->father->VarAlter(ident, sign2, value);
    }
}

void IdentTableNode::IdentSearch(const string &ident, string &sign)
{
    if (!findValue(ident))
    {
        int CalValue = ConstTable.at(ident);
        sign = to_string(CalValue);
        return;
    }
    else if (!findVariable(ident))
    {
        alloc_now++;
        sign = "%" + to_string(alloc_now);
        cout << "  " << sign << " = "
             << "load "
             << "@" + ident + "_" + to_string(this->level) << endl;
        return;
    }
    else if (this->father == nullptr)
    {
        cerr << "Error: " << '"' << ident << '"' << " is not defined" << endl;
        exit(-1);
    }
    else
    {
        this->father->IdentSearch(ident, sign);
    }
}

int IdentTableNode::IdentCalc(const string &ident)
{
    if (!findValue(ident))
    {
        int CalValue = ConstTable.at(ident);
        return CalValue;
    }
    else if (!findVariable(ident))
    {
        return VarTable.at(ident);
    }
    else if (!this->father)
    {
        cerr << "Error: " << '"' << ident << '"' << " is not defined" << endl;
        exit(-1);
    }
    else
    {
       return this->father->IdentCalc(ident);
    }
}

void IdentTableNode::IdentSearch(const string &ident, string &sign,int &type)
{
    if (!findValue(ident))
    {
        int CalValue = ConstTable.at(ident);
        sign = to_string(CalValue);
        type = FIND_CONST;
        return;
    }
    else if (!findVariable(ident))
    {
        type = FIND_VAR;
        sign = "@" + ident + "_" + to_string(this->level);
        return;
    }
    else if (this->father == nullptr)
    {
        cerr << "Error: " << '"' << ident << '"' << " is not defined" << endl;
        exit(-1);
    }
    else
    {
        //cout << "father's level " << this->father->level << endl;
        this->father->IdentSearch(ident,sign,type);
    }
}
