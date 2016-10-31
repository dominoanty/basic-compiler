//
// Created by domino on 29/10/16.
//
#include <iostream>
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
    std::string getName(){return Name;}
};
class BinaryExprAST : public ExprAST {
    std::string Op;
    ExprAST *LHS, *RHS;

    public:
    BinaryExprAST(std::string Op, ExprAST* LHS,
                           ExprAST* RHS)
                 : Op(Op), LHS(LHS), RHS(RHS) {}
    void setOp(std::string Op)
    {
        if(this->Op == " ")
        {
            this->Op = Op;
        }
    }
    void setRHS(ExprAST* RHS)
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
class StatementAST{
    public:
    StatementAST(){}
};
class AssignmentStatementAST : public StatementAST{
    VariableExprAST* LValue;
    ExprAST* RValue;
    public:
    AssignmentStatementAST(VariableExprAST* LValue, ExprAST* RValue) :
            LValue(LValue), RValue(RValue) {};

};
class StatementBlockAST :  public StatementAST{
    std::vector<StatementAST*> Statements;

    public:
    StatementBlockAST(std::vector<StatementAST*> Statements) : Statements(Statements){}
};
class ConditionalStatementAST : public  StatementAST{
    BinaryExprAST* Condition;
    StatementAST* Then;
    StatementAST* Else;

    public:
    ConditionalStatementAST(BinaryExprAST* Condition,
                            StatementAST* Then,
                            StatementAST* Else) :
                            Condition(Condition), Then(Then), Else(Else) {};
};
class LoopStatementAST : public StatementAST{
    BinaryExprAST* Condition;
    StatementAST* LoopStatements;

    public:
    LoopStatementAST(BinaryExprAST* Condition, StatementAST* LoopStatements) :
            Condition(Condition), LoopStatements(LoopStatements){};

};
