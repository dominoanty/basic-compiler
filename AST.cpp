//
// Created by domino on 29/10/16.
//
#include <iostream>
#include <bits/unique_ptr.h>
#include <vector>


class ExprAST{
    public:
    virtual ~ExprAST(){}
};

class NumberExprAST : public ExprAST{
    double Val;

    public:
    NumberExprAST(double val) : Val(val) {}
};

class VariableExprAST : public ExprAST{
    std::string Name;

    public:
    VariableExprAST(std::string name) : Name(name) {}
};
class BinaryExprAST : public ExprAST {
    char Op;
    ExprAST *LHS, *RHS;

    public:
    BinaryExprAST(char Op, ExprAST* LHS,
                           ExprAST* RHS)
                 : Op(Op), LHS(LHS), RHS(RHS) {}
    void setOp(char Op)
    {
        if(this->Op == '')
        {
            this->Op = Op;
        }
    }
    void setRHS(ExprAST* Op)
    {
        if(this->RHS == nullptr)
        {
            this->RHS = RHS;
        }
    }
    ExprAST* getLHS()
    {
        return this->LHS;
    }

};

class CallExprAST : public ExprAST{
    std::string Callee;
    std::vector<ExprAST*> Args;

    public:
    CallExprAST(const std::string &Callee,
                std::vector<ExprAST*> Args)
            : Callee(Callee), Args(Args) {}

};
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name,
                 std::vector<std::string> Args)
            : Name(Name), Args(std::move(Args)) {}
};
class FunctionAST {
    PrototypeAST* Proto;
    ExprAST* Body;

public:
    FunctionAST(PrototypeAST* Proto,
                ExprAST* Body) : Proto(Proto), Body(Body) {}
};
