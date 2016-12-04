//
// Created by domino on 29/10/16.
//
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <fstream>

union Value{
    double Number;
};

class FunctionAST;
class StatementBlockAST;
class CodeGenBlock{
public:
    std::map<std::string, double> locals;
};

class CodeGenContext{
    std::stack<CodeGenBlock*> blocks;

public:
    std::fstream gen_out;
    std::map<std::string, FunctionAST*> functionList;
    CodeGenContext(){
        gen_out.open("gen.out", std::ios::out);
    }
    ~CodeGenContext(){
        gen_out.close();
    }
    std::map<std::string, double> locals(){ return blocks.top()->locals;};
    void pushBlock(){ blocks.push(new CodeGenBlock()); }
    void popBlock() { CodeGenBlock* top = blocks.top(); blocks.pop(); delete top; }
    CodeGenBlock* topBlock() {return blocks.top();}
    void generateCode(StatementBlockAST* root);

};

class Node{
    std::string node_type;
public:
    virtual ~Node(){}
    virtual void set_type(std::string Type) { this->node_type = Type;};
    virtual double eval(CodeGenContext &context)=0;
};

class ExprAST {
    public:
    virtual ~ExprAST(){}
    virtual double eval(CodeGenContext &context) = 0;
};

class NumberExprAST : public ExprAST{
    double Val;

    public:
    NumberExprAST(double val) : Val(val) {}

    double eval(CodeGenContext &context){
        context.gen_out<<"\nthe number"<<Val;
        return Val;
    }
};

class VariableExprAST : public ExprAST{
    std::string Name;

    public:
    VariableExprAST(std::string name) : Name(name) {}
    std::string getName(){return Name;}

    double eval(CodeGenContext &context){
        if(context.locals().find(Name) == context.locals().end())
        {
            context.gen_out<<"\nUndeclared variable "<<Name;
            fprintf(stderr, "Undeclared variable ");
            return 0;
        }
        context.gen_out<<"\nthe variable "<<Name;
        return context.locals()[Name];
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

    double eval(CodeGenContext &context){
        double L = LHS->eval(context);
        double R = RHS->eval(context);
        context.gen_out<<"\n the binary expression "<<L<<" "<<Op<<" "<<R;
        if(Op == "+"){
            return L+R;
        }
        else if(Op == "+"){
            return L+R;
        }
        else if(Op == "-"){
            return L-R;
        }
        else if(Op == "*"){
            return L*R;
        }
        else if(Op == "/"){
            if(R == 0)
            {
                context.gen_out<<"\nError : Zero Division Error.";
                fprintf(stderr, "Zero division error");
                return 0;
            }
            return L/R;
        }
        else if(Op == ">="){
            return L>=R;
        }
        else if(Op == "<="){
            return L<=R;
        }
        else if(Op == ">"){
            return L>R;
        }
        else if(Op == "<"){
            return L<R;
        }
        return 0;
    }

};

class StatementAST : public Node{
    public:
    StatementAST(){}
};

class AssignmentStatementAST : public StatementAST{
    VariableExprAST* LValue;
    ExprAST* RValue;
    public:
    AssignmentStatementAST(VariableExprAST* LValue, ExprAST* RValue) :
            LValue(LValue), RValue(RValue) {};
    double eval(CodeGenContext &context){
        double R = RValue->eval(context);
        std::string LName = LValue->getName();
        if(context.topBlock()->locals.find(LName) == context.topBlock()->locals.end())
        {
            context.gen_out<<"\nError : Variable "<<LName<<" has not been declared in the scope.";
            fprintf(stderr, "Error variable not declared");
            return 0;
        }
        context.gen_out<<"\n the assignment "<<LName<<" = "<<R;
        context.topBlock()->locals[LName] = R;
        return R;
    }
};

class VariableDeclarationAST : public StatementAST{
    VariableExprAST* id;
    AssignmentStatementAST *assignExpr;
public:
    VariableDeclarationAST(VariableExprAST* id, AssignmentStatementAST* assignExpr):
            id(id), assignExpr(assignExpr) {}
    VariableDeclarationAST(VariableExprAST* id) : id(id) {assignExpr= nullptr; };
    double eval(CodeGenContext &context){
        std::string LName = id->getName();
        context.topBlock()->locals[LName] = 0;
        if(assignExpr != nullptr){
            double R = assignExpr->eval(context);
            context.topBlock()->locals[LName] = R;
            return R;
        }
        return 0;
    }
    double eval(CodeGenContext &context, double initVal){
        context.gen_out<<"\n Created variable" << id->getName();
        std::string LName = id->getName();
        context.topBlock()->locals[LName] = initVal;
        return initVal;
    }
};
class PrototypeAST {
    std::string Name;

public:
    std::vector<VariableDeclarationAST*> Args;
    PrototypeAST(const std::string &Name,
                 std::vector<VariableDeclarationAST*> Args)
            : Name(Name), Args(Args) {}
    std::string getName(){
        return Name;
    }
};

class PrintStatementAST : public StatementAST{
    ExprAST* Var;

public:
    PrintStatementAST(ExprAST* Var) : Var(Var) {};

    double eval(CodeGenContext &context){
        context.gen_out<<"\n print statement";
        double R = Var->eval(context);
        fprintf(stderr, "\n%f\n", R);
        return R;
    }
};

class CallExprAST : public ExprAST ,public StatementAST{
    std::string Callee;
    std::vector<ExprAST*> Args;
public:

    public:
    CallExprAST(const std::string &Callee,
                std::vector<ExprAST*> Args)
            : Callee(Callee), Args(Args) {}
    std::string getName(){
        return Callee;
    }
    double eval(CodeGenContext &context);
};


class StatementBlockAST :  public StatementAST{
    std::vector<StatementAST*> Statements;

    public:
    StatementBlockAST(std::vector<StatementAST*> Statements) : Statements(Statements){}
    double eval(CodeGenContext &context) ;
};
class FunctionAST : public Node{
    PrototypeAST* Proto;

public:
    StatementBlockAST* Body;
    FunctionAST(PrototypeAST* Proto,
                StatementBlockAST* Body) : Proto(Proto), Body(Body) {}
    double eval(CodeGenContext &context){return 0;}
    double eval(CodeGenContext &context, std::vector<ExprAST*>);
    void addToContext(CodeGenContext &context){
        context.functionList[this->Proto->getName()] = this;
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

    double eval(CodeGenContext &context);
};

class LoopStatementAST : public StatementAST{
    BinaryExprAST* Condition;
    StatementAST* LoopStatements;

    public:
    LoopStatementAST(BinaryExprAST* Condition, StatementAST* LoopStatements) :
            Condition(Condition), LoopStatements(LoopStatements){};


    double eval(CodeGenContext &context) ;
};

double ConditionalStatementAST::eval(CodeGenContext &context) {
    context.gen_out<<"\n for conditional statement\n";
    double R;
    if(this->Condition->eval(context)){
        R = Then->eval(context);
    }
    else{
        if(Else != nullptr)
         R = Else->eval(context);
    }
    return R;
}

double LoopStatementAST::eval(CodeGenContext &context) {
    context.gen_out<<"\n for loop statement \n";
    double retVal;
    while(this->Condition->eval(context)){
       retVal =  this->LoopStatements->eval(context);
    }
    return retVal;
}

double CallExprAST::eval(CodeGenContext &context){
    context.gen_out<<"\nfor function call statement \n";
    if(context.functionList.find(this->Callee) == context.functionList.end()){
        fprintf(stderr, "Function not defined ");
        return 0;
    }
    return context.functionList[Callee]->eval(context, this->Args);
}


double FunctionAST::eval(CodeGenContext &context, std::vector<ExprAST*> Args){
   if(this->Proto->Args.size() != Args.size()){
       fprintf(stderr, "Wrong number of arguments");
       return 0;
   }
   double *evalArgs = new double[Args.size()];
   std::vector<ExprAST*>::const_iterator it;
    int i=0;
   for( it = Args.begin() ; it != Args.end();i++, it++){
       evalArgs[i] = (*it)->eval(context);
   }
    context.pushBlock();

   std::vector<VariableDeclarationAST*>::const_iterator it2;
   for(i=0, it2 = Proto->Args.begin(); it2 != Proto->Args.end(); it2++, i++){
       (*it2)->eval(context, evalArgs[i]);
   }
   double retVal = this->Body->eval(context);
   context.popBlock();
   return retVal;
}



double StatementBlockAST::eval(CodeGenContext &context) {
    std::vector<StatementAST*>::const_iterator it;
    double last = 0;
    for(it = this->Statements.begin(); it != this->Statements.end(); it++)
    {
        context.gen_out<< "\nGenerating code for"<<  std::endl;
        last = (**it).eval(context);
    }
    return last;
}

void CodeGenContext::generateCode(StatementBlockAST *root) {
    gen_out<< "\n\n Generating code \n ";

    this->pushBlock();
    root->eval(*this);
    popBlock();

    gen_out<<"\n Finished generating code";
}

