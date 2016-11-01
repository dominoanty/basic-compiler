//
// Created by domino on 29/10/16.
//
#include <iostream>
#include <vector>


class ExprAST{
    public:
    virtual ~ExprAST(){}
    virtual void print(){
        std::cout<<"Blank";
    }
};

class NumberExprAST : public ExprAST{
    double Val;

    public:
    NumberExprAST(double val) : Val(val) {}
    void print(){
        std::cout<<Val;
    }
};

class VariableExprAST : public ExprAST{
    std::string Name;

    public:
    VariableExprAST(std::string name) : Name(name) {}
    std::string getName(){return Name;}
    void print(){
        std::cout<<Name;
    }
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
    void print(){
        std::cout<<"\nLHS is "; 
        if(!LHS)
          std::cout<<"null";
        else
          LHS->print();

        std::cout<<", operator is "<<Op;

        std::cout<<"\nRHS is "; 
        if(!RHS)
          std::cout<<"null";
        else
          RHS->print();
    }

};
class CallExprAST : public ExprAST{
    std::string Callee;
    std::vector<ExprAST*> Args;

    public:
    CallExprAST(const std::string &Callee,
                std::vector<ExprAST*> Args)
            : Callee(Callee), Args(Args) {}
    void print(){
      std::cout<<"\nCallee is "<<Callee;
      for(std::vector<ExprAST*>::iterator it= Args.begin();
          it!=Args.end(); ++it){
          if((*it) != nullptr)
              (*it)->print();
      }
    }
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
    virtual void print(){std::cout<<"Blank statement";}
};

class AssignmentStatementAST : public StatementAST{
    VariableExprAST* LValue;
    ExprAST* RValue;
    public:
    AssignmentStatementAST(VariableExprAST* LValue, ExprAST* RValue) :
            LValue(LValue), RValue(RValue) {};
    void print(){
      std::cout<<"\nLvalue is";
      if(!LValue)
        std::cout<<"null";
      else
        LValue->print();

      std::cout<<"Rvalue is";
      if(!RValue)
        std::cout<<"null";
      else
        RValue->print();
    }
};

class StatementBlockAST :  public StatementAST{
    std::vector<StatementAST*> Statements;

    public:
    StatementBlockAST(std::vector<StatementAST*> Statements) : Statements(Statements){}
    void print(){
       for(std::vector<StatementAST*>::iterator it= Statements.begin();
          it!=Statements.end(); ++it){
          if((*it) != nullptr)
              (*it)->print();
      }
    }
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
    void print(){
      if(!Condition)
        std::cout<<"\nNo condition";
      else
        Condition->print();

      if(!Then)
        std::cout<<"\nNo then statement";
      else
        Then->print();

      if(!Else)
        std::cout<<"\nNo else statement";
      else
        Else->print();
    }
};

class LoopStatementAST : public StatementAST{
    BinaryExprAST* Condition;
    StatementAST* LoopStatements;

    public:
    LoopStatementAST(BinaryExprAST* Condition, StatementAST* LoopStatements) :
            Condition(Condition), LoopStatements(LoopStatements){};

    void print(){
        if(!Condition)
          std::cout<<"\nNo condition statement";
        else
          Condition->print();

        if(!LoopStatements)
          std::cout<<"\nNo loop statements";
        else
          LoopStatements->print();

    }
};
