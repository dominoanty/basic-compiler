//
// Created by domino on 29/10/16.
//
#include <iostream>
#include <vector>
#include <stack>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/GenericValue.h>

using namespace llvm;

static LLVMContext globalContext;
static IRBuilder<> Builder(globalContext);

class StatementBlockAST;

class CodeGenBlock{
public:
    BasicBlock *block;
    std::map<std::string, Value*> locals;

};

class CodeGenContext{
    std::stack<CodeGenBlock*> blocks;
    Function* mainFunction;

public:
    Module* module;
    CodeGenContext(){
        module = new Module("main", globalContext);
    }
    void generateCode(StatementBlockAST* root);
    GenericValue runCode();
    std::map<std::string, Value*> locals(){ return blocks.top()->locals;};
    BasicBlock *currentBlock() { return blocks.top()->block;}
    void pushBlock(BasicBlock *block){ blocks.push(new CodeGenBlock()); blocks.top()->block = block;}
    void popBlock() { CodeGenBlock* top = blocks.top(); blocks.pop(); delete top; }
    CodeGenBlock* topBlock() {return blocks.top();}

};

class Node{
    std::string node_type;
public:
    virtual ~Node(){}
    virtual void print(){
        std::cout<<"Blank node";
    }
    virtual void set_type(std::string Type) { this->node_type = Type;};
    virtual Value *codegen(CodeGenContext &context) = 0;
};

class ExprAST {
    public:
    virtual ~ExprAST(){}
    virtual void print(){
        std::cout<<"Blank expression";
    }
    virtual Value *codegen(CodeGenContext &context) = 0;
};

class NumberExprAST : public ExprAST{
    double Val;

    public:
    NumberExprAST(double val) : Val(val) {}
    void print(){
        std::cout<<Val;
    }
    Value* codegen(CodeGenContext &context);
};

class VariableExprAST : public ExprAST{
    std::string Name;

    public:
    VariableExprAST(std::string name) : Name(name) {}
    std::string getName(){return Name;}
    void print(){
        std::cout<<Name;
    }
    Value* codegen(CodeGenContext &context);
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

    Value* codegen(CodeGenContext &context);
};

class StatementAST : public Node{
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
    Value* codegen(CodeGenContext &context);
};

class VariableDeclarationAST : public StatementAST{
    VariableExprAST* id;
    AssignmentStatementAST *assignExpr;
public:
    VariableDeclarationAST(VariableExprAST* id, AssignmentStatementAST* assignExpr):
            id(id), assignExpr(assignExpr) {}
    VariableDeclarationAST(VariableExprAST* id) : id(id) {assignExpr = nullptr;};
    Value* codegen(CodeGenContext &context);
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

class CallExprAST : public ExprAST ,public StatementAST{
    std::string Callee;
    std::vector<ExprAST*> Args;
public:

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
    std::string getName(){
        return Callee;
    }
    Value* codegen(CodeGenContext &context);
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
    Value* codegen(CodeGenContext &context) ;
};
class FunctionAST : public Node{
    PrototypeAST* Proto;

public:
    StatementBlockAST* Body;
    FunctionAST(PrototypeAST* Proto,
                StatementBlockAST* Body) : Proto(Proto), Body(Body) {}
    void print(){
        Body->print();
    }
    Function* codegen(CodeGenContext &context);
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
    Value* codegen(CodeGenContext &context);
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
    Value* codegen(CodeGenContext &context) ;
};

Value* NumberExprAST::codegen(CodeGenContext &context){
    std::cout<<"Creating integer : "<< this->Val << std::endl;
    return ConstantFP::get(Type::getDoubleTy(globalContext), this->Val);
}
Value* VariableExprAST::codegen(CodeGenContext &context){
    std::cout<<" Creating identifier ref" << this->Name << std::endl;
    if (context.locals().find(this->Name) == context.locals().end()) {
        std::cerr << "undeclared variable " << this->Name << std::endl;
        return NULL;
    }
    return new LoadInst(context.locals()[this->Name], "", false, context.currentBlock());
}


Value* BinaryExprAST::codegen(CodeGenContext &context){
    Value *L = this->LHS->codegen(context);
    Value *R = this->RHS->codegen(context);

    if(!L || !R)
        return nullptr;

    std::cout << "Creating binary operation " << Op << std::endl;
    Instruction::BinaryOps instr;
     if(Op == "+") {
         instr = Instruction::FAdd;
    }
    else if(Op == "-") {
         instr = Instruction::FSub;
    }
    else if(Op == "/") {
         instr = Instruction::FDiv;
    }
    else if(Op == "*") {
         instr = Instruction::FMul;
    }
  /*  else if(Op == ">") {
         instr = Instruction::FCmp;
     } */
    else {
         Instruction::OtherOps ins;
         if(Op == ">"){
             ins = Instruction::FCmp;
             return FCmpInst::Create(ins, FCmpInst::FCMP_OGT, L, R, "", context.currentBlock());
         }
         else if(Op == "<"){
             ins = Instruction::FCmp;
             return FCmpInst::Create(ins, FCmpInst::FCMP_OLT,  L, R, "", context.currentBlock());
         }
         else if(Op == "<="){
             ins = Instruction::FCmp;
             return FCmpInst::Create(ins, FCmpInst::FCMP_OLE,  L, R, "", context.currentBlock());
         }
         else if(Op == ">="){
             ins = Instruction::FCmp;
             return FCmpInst::Create(ins, FCmpInst::FCMP_OGE,  L, R, "", context.currentBlock());
         }
     }
    return BinaryOperator::Create(instr, L, R, "", context.currentBlock());
}

/*

Value* ConditionalStatementAST::codegen(CodeGenContext &context) {
    Inst
    Function* parentFunction = Builder.GetInsertBlock()->getParent();

    BasicBlock *thenBB = BasicBlock::Create(globalContext, "then", parentFunction);
    BasicBlock *elseBB = BasicBlock::Create(globalContext, "else");
    BasicBlock *mergeBB = BasicBlock::Create(globalContext, "ifcont");

    Builder.CreateCondBr(this->Condition->codegen(context), thenBB, elseBB);


    Builder.SetInsertPoint(thenBB);

    Value* thenV = Then->codegen(context);
    if(thenV == 0) return 0;
    Builder.CreateBr(mergeBB);
    thenBB = Builder.GetInsertBlock();

    parentFunction->getBasicBlockList().push_back(elseBB);
    Builder.SetInsertPoint(elseBB);

    Value* elseV = Else->codegen(context);
    if(elseV == 0) return 0;
    Builder.CreateBr(mergeBB);
    elseBB = Builder.GetInsertBlock();

    parentFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
    PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(globalContext), 2, "iftmp");
    PN->addIncoming(thenV, thenBB);
    PN->addIncoming(elseV, elseBB);
    return PN;

}
 */
/*Value* ConditionalStatementAST::codegen(CodeGenContext &context) {

     std::cout << "Creating if statement " << std::endl;

     Value *condValue = this->Condition->codegen(context);
     if (condValue == nullptr) return nullptr;
     std::cout << condValue->getType()->getTypeID() << std::endl;

     Function *function = context.currentBlock()->getParent();

     BasicBlock *thenBlock = BasicBlock::Create(globalContext,"if.then", function);
     BasicBlock *elseBlock = BasicBlock::Create(globalContext,"if.else");
     BasicBlock *mergeBlock = BasicBlock::Create(globalContext,"if.cont");


     BranchInst::Create(thenBlock, elseBlock, condValue,context.currentBlock());

     // create then block
     Value *thenValue = this->Then->codegen(context);
     if (thenValue == nullptr) return nullptr;
     BranchInst::Create(mergeBlock, context.currentBlock());


     // create else block
    function->getBasicBlockList().push_back(elseBlock);
     Value *elseValue = this->Else->codegen(context);
    if (elseValue == nullptr) return nullptr;
    BranchInst::Create(mergeBlock, context.currentBlock());

     // create PHI node

    function->getBasicBlockList().push_back(mergeBlock);
    PHINode *PN = PHINode::Create(Type::getVoidTy(globalContext), 2, "", mergeBlock);

    PN->addIncoming(thenValue, thenBlock);
    if(Else)
        PN->addIncoming(elseValue, elseBlock);

    return PN;
}*/

Value* ConditionalStatementAST::codegen(CodeGenContext &context) {
    BasicBlock *previousBasicBlock = context.currentBlock();
    Value* condValue = this->Condition->codegen(context);

    BasicBlock* thenBasicBlock =
            BasicBlock::Create(globalContext, "if.then",
                               context.currentBlock()->getParent());
    context.pushBlock(thenBasicBlock);
    Then->codegen(context);

    BasicBlock *elseBasicBlock;
    if(Else){
        elseBasicBlock = BasicBlock::Create(globalContext, "if.else",
                                             context.currentBlock()->getParent());
        context.pushBlock(elseBasicBlock);
        Else->codegen(context);

    }
    BasicBlock *mergeBasicBlock = BasicBlock::Create(globalContext, "if.cont",
                                                     context.currentBlock()->getParent());
    context.pushBlock(mergeBasicBlock);

    if(thenBasicBlock->getTerminator() == NULL)
        BranchInst::Create(mergeBasicBlock, thenBasicBlock);
    if(Else){
        if(elseBasicBlock->getTerminator() == NULL)
            BranchInst::Create(mergeBasicBlock, elseBasicBlock);
        return BranchInst::Create(thenBasicBlock, elseBasicBlock, condValue, previousBasicBlock);
    } else
        return BranchInst::Create(thenBasicBlock,mergeBasicBlock, condValue, previousBasicBlock);

}

Value* LoopStatementAST::codegen(CodeGenContext &context) {

    BasicBlock *bodyBlock = BasicBlock::Create(globalContext, "while.body", context.currentBlock()->getParent());
    BasicBlock *mergeBlock = BasicBlock::Create(globalContext, "while.cont", context.currentBlock()->getParent());

    Value* condValue = this->Condition->codegen(context);
    if(condValue == NULL){
        fprintf(stderr, "Error generating condition value");
        return nullptr;
    }
    context.pushBlock(bodyBlock);
    BranchInst::Create(bodyBlock, mergeBlock, condValue, context.currentBlock());
    this->LoopStatements->codegen(context);

    condValue = Condition->codegen(context);
    BranchInst *lastBranch = BranchInst::Create(bodyBlock, mergeBlock, condValue, bodyBlock);
    return lastBranch;

}
Value* CallExprAST::codegen(CodeGenContext &context){
    Function *CalleeF = context.module -> getFunction(this->Callee);
    if(Callee.empty()){
        fprintf(stderr, "Unknown function referenced");
        return nullptr;
    }

    std::vector<Value*> ArgsV;
    std::vector<ExprAST*>::const_iterator it;
    for (it = Args.begin(); it != Args.end(); it++) {
        ArgsV.push_back((*it)->codegen(context));
    }
    CallInst *call = CallInst::Create(CalleeF, ArgsV, "", context.currentBlock());
    std::cout << "Creating method call: " << this->getName() << std::endl;
    return call;
}


Function* FunctionAST::codegen(CodeGenContext &context){
    std::vector< Type*> argTypes;
    auto it = Proto->Args.begin();
    for(it; it != Proto->Args.end(); it++){
        argTypes.push_back(Type::getDoubleTy(globalContext));
    }
    ArrayRef<Type*> newRef(argTypes);
    FunctionType *FT = FunctionType::get(Type::getVoidTy(globalContext), newRef, false);
    Function *TheFunction = Function::Create(FT, GlobalValue::InternalLinkage, this->Proto->getName(), context.module);
    BasicBlock *BB = BasicBlock::Create(globalContext, "entry", TheFunction);

    if (!TheFunction)
        return nullptr;

    context.pushBlock(BB);

    std::vector<VariableDeclarationAST*>::iterator it2;
    // Record the function arguments in the NamedValues map.
//    for (auto &Arg : TheFunction->args())
//        NamedValues[Arg.getName()] = &Arg;
    for(it2 = this->Proto->Args.begin(); it2!= Proto->Args.end(); it2++){
        (**it2).codegen(context);
    }

    Body->codegen(context);
    //Builder.CreateRet();
    ReturnInst::Create(globalContext,   BB);
    context.popBlock();
    std::cout<<"Creating function";
    return TheFunction;
}

Value* AssignmentStatementAST::codegen(CodeGenContext &context) {
    std::cout << "Creating assignment for " << this->LValue->getName() << std::endl;
    if (context.locals().find(LValue->getName()) == context.locals().end()) {
        std::cerr << "undeclared variable " << LValue->getName() << std::endl;
        return NULL;
    }
    return new StoreInst(RValue->codegen(context), context.locals()[LValue->getName()], false, context.currentBlock());
}
Value* VariableDeclarationAST::codegen(CodeGenContext &context){
    std::cout << "Creating variable declaration "  << this->id->getName() << std::endl;
    AllocaInst *alloc = new AllocaInst(Type::getDoubleTy(globalContext), id->getName().c_str(), context.currentBlock());
    context.topBlock()->locals[id->getName()] = alloc;
    if (this->assignExpr != nullptr) {
        assignExpr->codegen(context);
    }
    return alloc;
}

Value* StatementBlockAST::codegen(CodeGenContext &context) {
    std::vector<StatementAST*>::const_iterator it;
    Value* last = NULL;
    for(it = this->Statements.begin(); it != this->Statements.end(); it++)
    {
        std::cout<< "Generating code for"<< typeid(**it).name() << std::endl;
        last = (**it).codegen(context);
    }
    return last;
}void CodeGenContext::generateCode(StatementBlockAST *root) {
    std::cout<< " Generating code ";

    std::vector<llvm::Type*> argTypes;
    ArrayRef<Type*> newRef(argTypes);
    FunctionType *FT = FunctionType::get(Type::getVoidTy(globalContext), newRef, false);
    mainFunction = Function::Create(FT, GlobalValue::InternalLinkage, "main", module);
    BasicBlock *bblock = BasicBlock::Create(globalContext, "entry", mainFunction, 0);

    pushBlock(bblock);
    root->codegen(*this);
    ReturnInst::Create(globalContext, bblock);
    popBlock();

    std::cout<<"Code is generated";
    module->dump();
}

