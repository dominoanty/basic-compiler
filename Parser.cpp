#include <iostream>
#include <string>
#include <fstream>
#include "Lexer.cpp"
#include "AST.cpp"
#include <cstdlib>

class Parser{
    Lexer *L;
    Token *curr_Token;
    std::vector<Node*> TopLevelNodes;
    CodeGenContext context;
    std::ofstream lex_out;
    std::ofstream parse_out;
    FILE* parse_o;

public:

    Parser(std::string input_string){
        L = new Lexer(input_string);
        curr_Token = L->get_token();
        lex_out.open("lex.out", std::ios::out);
        lex_out<<"\nReceived String : ";
        lex_out<<input_string;
        parse_o = fopen("parse.out", "w");

        lex_out<<"\nSomething atleast";

    }
    ~Parser(){
        parse_out.close();
        lex_out.close();
    }
    void print_curr_token(){
        lex_out<<std::endl;
        lex_out<<curr_Token->get_token_string();
        switch(curr_Token->get_token_type())
        {
            case  EOF_TOKEN : lex_out<<"  EOF_TOKEN"; break;
            case  IDENTIFIER : lex_out<<"  IDENTIFIER"; break;
            case  KEYWORD : lex_out<<"  KEYWORD"; break;
            case  NUMBER : lex_out<<"  NUMBER"; break;
            case  SYMBOL : lex_out<<"  SYMBOL"; break;
            case  CONDITIONAL : lex_out<<"  CONDITIONAL"; break;

            default:
                lex_out<<"  UNRECOGNIZED";
                break;
        }
    }
    void get_next_token()
    {
        if(curr_Token->get_token_type() != EOF_TOKEN)
            curr_Token = L->get_token();
        print_curr_token();

    }

    //Parse Numbers and eat the corresponding token
    ExprAST* ParseNumberExpr(){
        auto Result = new NumberExprAST(curr_Token->get_token_val());
        get_next_token(); //Consume the number
        return Result;
    }

    //Parse identifiers and function calls
    ExprAST* ParseIdentifierExpr()
    {
        std::string IdName = curr_Token->get_token_string();

        get_next_token();  //Consume identifier

        if(curr_Token->get_token_string() != "("){
            return new VariableExprAST(IdName);
        }

        get_next_token(); //Consume (

        std::vector<ExprAST*> Args;
        if(curr_Token ->get_token_string() != ")"){
            while(1){
                if(auto Arg = ParseExpression())
                    Args.push_back(Arg);
                else
                    return nullptr;

                if(curr_Token->get_token_string() == ")")
                    break;

                if(curr_Token->get_token_string() != ","){
                    fprintf(stderr, "Expected  ) or  , in argument list");
                }
                get_next_token();
            }
        }
        get_next_token(); //Consume )
        return new CallExprAST(IdName, Args);
    }

    // Parse identifers or numbers based on token type
    ExprAST* ParseFactor() {
        switch (curr_Token->get_token_type()) {
            case IDENTIFIER:
                return ParseIdentifierExpr();
            case NUMBER:
                return ParseNumberExpr();
            case SYMBOL:
                if(curr_Token -> get_token_string() == "(") {
                    get_next_token(); // Consume (
                    auto result = ParseExpression(); // Generate expression tree
                    if(curr_Token -> get_token_string() != ")")
                    {
                        fprintf(stderr,"Error parsing between paranthes");
                        return nullptr;
                    }
                    else
                        get_next_token(); // Consume )
                    return result;
                }
            default:
                fprintf(stderr, "Expected number identifier or brackets but got ");
                std::cout<<curr_Token->get_token_string() << "::" << curr_Token -> get_token_type();
                return nullptr;
        }
    }
    // PARSE TERMS
    // T  -> FT'
    // T' -> *FT' | /FT' | epsilon
    ExprAST* ParseTermDash(BinaryExprAST* curr_expr_tree)
    {
        if(curr_Token -> get_token_string() == "*" ||
                curr_Token -> get_token_string() == "/")
        {
            auto Op = curr_Token -> get_token_string();
            curr_expr_tree->setOp(Op);
            get_next_token(); //Consume operator

            auto oldRHS = ParseFactor();
            curr_expr_tree->setRHS(oldRHS);

            auto new_expr_tree =
                    ParseTermDash(new BinaryExprAST
                                          (" ", curr_expr_tree, nullptr));
            return new_expr_tree;

        }
        else
        {
            return curr_expr_tree->getLHS();
        }
    }

   ExprAST* ParseTerm(){
        auto LHS = ParseFactor();
        if(!LHS)
        {
            fprintf(stderr, "Error parsing");
            return nullptr;

        }
        auto curr_expr_tree = new BinaryExprAST(" ", LHS, nullptr);
        auto Result = ParseTermDash(curr_expr_tree);
       return Result;

   }
    //PARSE EXPRESSIONS
    //E  -> TE'
    //E' -> +TE' | -TE' | epsilon
    ExprAST* ParseExprDash(BinaryExprAST* curr_expr_tree)
    {
        if(curr_Token -> get_token_string() == "+" ||
                curr_Token -> get_token_string() == "-")
        {
            auto Op = curr_Token -> get_token_string();
            curr_expr_tree->setOp(Op);
            get_next_token(); //Consume the operator

            auto oldRHS = ParseTerm();
            curr_expr_tree->setRHS(oldRHS);

            auto new_expr_tree =
                    ParseExprDash(new BinaryExprAST
                                          (" ", curr_expr_tree, nullptr));
            return new_expr_tree;

        }
        else
        {
            return curr_expr_tree->getLHS();
        }
    }
    ExprAST* ParseExpression(){
        auto LHS = ParseTerm();
        if(!LHS)
        {
            fprintf(stderr, "\nError parsing expression");
            return nullptr;
        }
        auto curr_expr_tree = new BinaryExprAST(" ", LHS, nullptr);
        return ParseExprDash(curr_expr_tree);

    }
    // C -> E > E
    BinaryExprAST* ParseCondition() {
        auto LHS = ParseExpression();

        if(!LHS) {
            fprintf(stderr, "\nError parsing first expression in conditional");
            return nullptr;
        }

        if(curr_Token ->get_token_type() != CONDITIONAL){
            fprintf(stderr, "\nError parsing conditional operator");
            return nullptr;
        }
        std::string Op = curr_Token -> get_token_string();

        get_next_token(); // Consume operator
        auto RHS = ParseExpression();
        return new BinaryExprAST(Op, LHS, RHS);
    }

    PrototypeAST* ParsePrototype() {
        if(curr_Token -> get_token_type() != IDENTIFIER) {
            fprintf(stderr, "\nExpected function name");
            return nullptr;
        }

        std::string fnName = curr_Token ->get_token_string();
        get_next_token(); //consume identifier

        if(curr_Token -> get_token_string() != "(") {
            fprintf(stderr, "\nExpected arguments list");
            return nullptr;
        }
        get_next_token(); //consume (

        std::vector<VariableDeclarationAST*> arguments;

        while(curr_Token -> get_token_type() == IDENTIFIER){
            arguments.push_back( new VariableDeclarationAST( new VariableExprAST(curr_Token->get_token_string())));
            get_next_token(); //consume identfier

            if(curr_Token -> get_token_string() != "," && curr_Token -> get_token_string() != ")") {
                fprintf(stderr, "\nExpected multiple arguments or end of arguments list");
                return nullptr;
            }
            if(curr_Token ->get_token_string() == ",")
                get_next_token();

        }

         if(curr_Token -> get_token_string() != ")") {
             fprintf(stderr, "\nExpected )");
             return nullptr;
         }

        get_next_token(); // consume )
         if(curr_Token -> get_token_string() != ";") {
             fprintf(stderr, "\nExpected ;");
         }
        else
        get_next_token(); // consume ;
        return new PrototypeAST(fnName, arguments);
    }

    FunctionAST* ParseDefinition() {
        get_next_token(); //Consume def
        PrototypeAST* Proto = (PrototypeAST*) ParsePrototype();
        if (!Proto)
            return nullptr;

        StatementBlockAST* E = (StatementBlockAST*) ParseStatement();

        if(E)
            return new FunctionAST(Proto, E);
        return nullptr;
    }

    FunctionAST* ParseTopLevelStatement() {

        if (StatementBlockAST* E = (StatementBlockAST*) ParseBlockStatement()) {
            // Make an anonymous proto.
            auto Proto = new PrototypeAST("__main_st", std::vector<VariableDeclarationAST*>());
            return new FunctionAST(Proto, E);
        }
        return nullptr;
    }

    StatementAST* ParseIfStatement() {
        get_next_token(); //Consume 'if'


        if(curr_Token -> get_token_string() != "(")
        {
            fprintf(stderr, "Error : Expected ( after if ");
            return nullptr;
        }
        get_next_token(); // Consume (

        auto Condition = ParseCondition();

        if(curr_Token -> get_token_string() != ")")
        {
            fprintf(stderr, "Error : Expected ) after condition ");
            return nullptr;
        }
        get_next_token(); //consume )

        if(curr_Token -> get_token_string() != "then")
        {
            fprintf(stderr, "\nError : Couldn't find then after if ");
            return nullptr;
        }

        get_next_token(); // Consume 'then'

        StatementAST* ThenStatement;
        ThenStatement = ParseStatement();

        if(curr_Token -> get_token_string() != "else")
            return new ConditionalStatementAST(Condition, ThenStatement, nullptr);


        get_next_token(); // Consume else

        StatementAST* ElseStatement;
        ElseStatement = ParseStatement();

        return new ConditionalStatementAST(Condition, ThenStatement, ElseStatement);
    }

    StatementAST* ParseLoopStatement() {
        get_next_token(); //Consume 'while'

        if(curr_Token -> get_token_string() != "(")
        {
            fprintf(stderr, "Error : Eopected ( after condition ");

            return nullptr;
        }
        get_next_token(); //consume (

        auto Condition = ParseCondition();


        if(curr_Token -> get_token_string() != ")")
        {
            fprintf(stderr, "Error : Expected ) after condition ");
            return nullptr;
        }
        get_next_token(); //consume )


        if(curr_Token -> get_token_string() != "do")
        {
            fprintf(stderr, "\nError : Couldn't find do after while ");
            return nullptr;
        }

        get_next_token(); // Consume 'do'

        StatementAST* ThenStatement;
        ThenStatement = ParseStatement();
        fprintf(parse_o, "\nParsed do-while loop statement");

        return new LoopStatementAST(Condition, ThenStatement);

    }

    StatementAST* ParseAssignmentStatement() {
        if (curr_Token->get_token_type() != IDENTIFIER) {
            fprintf(stderr, "\nExpected identifier on LHS ");
            return nullptr;
        }
        VariableExprAST* LHS = (VariableExprAST*) ParseIdentifierExpr();
        if (curr_Token->get_token_string() != "=") {
            fprintf(stderr, "\nExpected  = ");
            return nullptr;
        }
        get_next_token(); // consume =

        auto RHS = ParseExpression();
        if (!RHS) {
            fprintf(stderr, "\ncould not read rhs ");
            return nullptr;
        }

        return new AssignmentStatementAST(LHS, RHS);
    }

    StatementAST* ParseBlockStatement()
    {
        if(curr_Token -> get_token_string() != "begin")
        {
            fprintf(stderr, "\nExpected begin ");
            return nullptr;
        }

        get_next_token(); // Consume 'begin'

        std::vector<StatementAST*> StatementList;
        while(curr_Token -> get_token_string() != "end" && curr_Token -> get_token_type() != EOF)
        {
            StatementList.push_back(ParseStatement());
        }

        if(curr_Token -> get_token_string()!= "end"){
            fprintf(stderr, "\nExpected end");
        }

        get_next_token(); // cosume end

        if(curr_Token->get_token_string() != ";"){

        }
        return new StatementBlockAST(StatementList);
    }

    CallExprAST* ParseCallStatement(){

        if(curr_Token -> get_token_string() != "call") {
            fprintf(stderr, "\nExpected call");
            return nullptr;
        }

        get_next_token(); //consume call

        CallExprAST* Result = (CallExprAST*) ParseIdentifierExpr();

        return Result;
    }
    StatementAST* ParseVarDecStatement(){
        get_next_token(); //  Consume var

        if(curr_Token -> get_token_type() != IDENTIFIER){
            fprintf(stderr, "\nExpected variable identifier after var");
            return nullptr;
        }

        VariableExprAST* newVar = (VariableExprAST*) ParseIdentifierExpr(); //Consume identifier

        if(curr_Token -> get_token_string() != "="){
            fprintf(stderr, "\nExpected = ");
            return nullptr;
        }
        get_next_token(); //Parse  =

        auto RHS = ParseExpression();
        auto AssignStatement = new AssignmentStatementAST(newVar, RHS);

        return new VariableDeclarationAST(newVar, AssignStatement );
    }
    StatementAST* ParsePrintStatement(){
        get_next_token(); //consume print
        if( auto E = ParseExpression()){
            return new PrintStatementAST(E);
        };
        return nullptr;
    }
    StatementAST* ParseStatement() {
        StatementAST* retStatement;
       switch(curr_Token -> get_token_type())
       {
           case IDENTIFIER: retStatement =  ParseAssignmentStatement();
           case KEYWORD:
               if(curr_Token -> get_token_string() == "begin") {
                   fprintf(parse_o, "\nParsing block statement");
                   retStatement =  ParseBlockStatement();
               }

               else if(curr_Token -> get_token_string() == "if") {

                   fprintf(parse_o, "\nParsing if statement");
                   retStatement =  ParseIfStatement();
               }

               else if(curr_Token -> get_token_string() == "while"){
                   fprintf(parse_o, "\nParsing while statement");
                   retStatement =  ParseLoopStatement();
               }

               else if( curr_Token -> get_token_string() == "call") {
                   fprintf(parse_o, "\nParsing call statement");
                   retStatement =  ParseCallStatement();
               }
               else if(curr_Token -> get_token_string() == "var"){
                   fprintf(parse_o, "\nParsing variable declaration");
                   retStatement =  ParseVarDecStatement();
               }
               else if(curr_Token -> get_token_string() == "out"){
                   fprintf(parse_o, "\nPrinting statement");
                   retStatement =  ParsePrintStatement();
               }
               else {
                   fprintf(parse_o, "\nCould not recognize statement");
               }
               break;
           case EOF:
               fprintf(parse_o, "Finished parsing");
               retStatement =  nullptr;
               break;
           default:
               fprintf(stderr, "Unexpected input" );
               exit(0);
       }
        if(curr_Token->get_token_string() != ";"){
            fprintf(stderr, "\nExpected ;");
        }
        else{

            get_next_token(); //consume ;
        }
        return retStatement;
    }


    void HandleDefinition() {
       FunctionAST* Result = (FunctionAST*) ParseDefinition();
       if(Result){
           Result->set_type("func_def");
           Result->addToContext(context);
           TopLevelNodes.push_back(Result);
           fprintf(parse_o, "\nParsed a function definiton\n");
       }
       else
       {
           get_next_token();
       }
    }

    void HandleMain(){
        FunctionAST* Result = (FunctionAST*) ParseTopLevelStatement();
        if(Result){
            Result->set_type("main_func");
            context.generateCode(Result->Body);

            TopLevelNodes.push_back(Result);
            TopLevelNodes.push_back(Result);
            fprintf(parse_o, "\nParsed main \n");
        }
        else
        {
            get_next_token();
        }
    }
    void MainLoop()
    {
        while(true)
        {
            fprintf(stderr, "ready> ");
            switch(curr_Token->get_token_type())
            {
                case EOF_TOKEN:
                    return;
                case KEYWORD:
                    if(curr_Token ->get_token_string() == "procedure")
                        HandleDefinition();
                    else if(curr_Token -> get_token_string() == "begin")
                        HandleMain();
                    break;

                default:
                    if(curr_Token -> get_token_string() == ";")
                        get_next_token();
                    else
                    {
                        fprintf(stderr,"Unexpected input");
                        exit(0);
                    }
                    break;
            }
        }
    }

};


