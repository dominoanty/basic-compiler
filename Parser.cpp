#include <iostream>
#include <string>
#include "Lexer.cpp"
#include "AST.cpp"

class Parser{
    Lexer *L;
    Token *curr_Token;

public:

    Parser(std::string input_string){
        L = new Lexer(input_string);
        curr_Token = L->get_token();
    }
    void get_next_token()
    {
        if(curr_Token->get_token_type() != EOF_TOKEN)
            curr_Token = L->get_token();
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
            fprintf(stderr, "Error parsing expression");
            return nullptr;
        }
        auto curr_expr_tree = new BinaryExprAST(" ", LHS, nullptr);
        return ParseExprDash(curr_expr_tree);

    }
    // C -> E > E
    BinaryExprAST* ParseCondition() {
        auto LHS = ParseExpression();

        if(!LHS) {
            fprintf(stderr, "Error parsing first expression in conditional");
            return nullptr;
        }

        if(curr_Token ->get_token_type() != CONDITIONAL){
            fprintf(stderr, "Error parsing conditional operator");
            return nullptr;
        }
        std::string Op = curr_Token -> get_token_string();

        get_next_token(); // Consume operator
        auto RHS = ParseExpression();
        return new BinaryExprAST(Op, LHS, RHS);
    }

    PrototypeAST* ParsePrototype() {
        if(curr_Token -> get_token_type() != IDENTIFIER) {
            fprintf(stderr, "Expected function name");
            return nullptr;
        }

        std::string fnName = curr_Token ->get_token_string();
        get_next_token(); //consume identifier

        if(curr_Token -> get_token_string() != "(") {
            fprintf(stderr, "Expected arguments list");
            return nullptr;
        }
        get_next_token(); //consume (

        std::vector<std::string> argument_names;
        do{
            argument_names.push_back(curr_Token->get_token_string());
            get_next_token(); //consume identfier

            if(curr_Token -> get_token_string() != "," && curr_Token -> get_token_string() != ")") {
                fprintf(stderr, "Expected multiple arguments or end of arguments list");
                return nullptr;
            }
            if(curr_Token ->get_token_string() == ",")
                get_next_token();

        }while(curr_Token -> get_token_type() == IDENTIFIER);

         if(curr_Token -> get_token_string() != ")") {
             fprintf(stderr, "Expected )");
             return nullptr;
         }

        get_next_token(); // consume )
        return new PrototypeAST(fnName, argument_names);
    }

    FunctionAST* ParseDefinition() {
        get_next_token(); //Consume def
        auto Proto = ParsePrototype();
        if (!Proto)
            return nullptr;

        if (auto E = ParseExpression())
            return new FunctionAST(Proto, E);
        return nullptr;
    }

    FunctionAST* ParseTopLevelExpr() {
        if (auto E = ParseExpression()) {
            // Make an anonymous proto.
            auto Proto = new PrototypeAST("__anon_expr",
                                                         std::vector<std::string>());
            return new FunctionAST(Proto, E);
        }
        return nullptr;
    }

    StatementAST* ParseIfStatement() {
        get_next_token(); //Consume 'if'

        auto Condition = ParseCondition();

        if(curr_Token -> get_token_string() != "then")
        {
            fprintf(stderr, "Error : Couldn't find then after if ");
        }

        get_next_token(); // Consume 'then'

        StatementAST* ThenStatement;
        ThenStatement = ParseStatement();
        fprintf(stderr, "Parsed if statement");

        if(curr_Token -> get_token_string() != "else")
            if(curr_Token -> get_token_string() == "end")
            {
                get_next_token(); // Consume end
                return new ConditionalStatementAST(Condition, ThenStatement, nullptr);
            }
            else
            {
                fprintf(stderr, "Expected else or end");
                return nullptr;
            }

        get_next_token(); // Consume else

        StatementAST* ElseStatement;
        ElseStatement = ParseStatement();

        return new ConditionalStatementAST(Condition, ThenStatement, ElseStatement);
    }

    StatementAST* ParseLoopStatement() {
        get_next_token(); //Consume 'while'

        auto Condition = ParseCondition();

        if(curr_Token -> get_token_string() != "do")
        {
            fprintf(stderr, "Error : Couldn't find do after while ");
        }

        get_next_token(); // Consume 'do'

        StatementAST* ThenStatement;
        ThenStatement = ParseStatement();
        fprintf(stderr, "Parsed do statement");

        return new LoopStatementAST(Condition, ThenStatement);

    }

    StatementAST* ParseAssignmentStatement() {
        if (curr_Token->get_token_type() != IDENTIFIER) {
            fprintf(stderr, "Expected identifier on LHS ");
            return nullptr;
        }
        auto LHS = curr_Token->get_token_string();
        get_next_token(); //consume Identifier
        if (curr_Token->get_token_string() != "=") {
            fprintf(stderr, "Expected  = ");
            return nullptr;
        }
        get_next_token(); // consume =

        auto RHS = ParseExpression();
        if (!RHS) {
            fprintf(stderr, "could not read rhs ");
        }

        return new AssignmentStatementAST(new VariableExprAST(LHS), RHS);
    }

    StatementAST* ParseBlockStatement()
    {
        if(curr_Token -> get_token_string() != "begin")
        {
            fprintf(stderr, "Expected begin ");
            return nullptr;
        }

        get_next_token(); // Consume 'begin'

        std::vector<StatementAST*> StatementList;
        while(curr_Token -> get_token_string() != "end")
        {
            StatementList.push_back(ParseStatement());
        }

        if(curr_Token -> get_token_string()!= "end"){
            fprintf(stderr, "Expected end");
        }

        get_next_token(); // cosume end
        return new StatementBlockAST(StatementList);
    }

    StatementAST* ParseCallStatement(){

    }

    StatementAST* ParseStatement() {
       switch(curr_Token -> get_token_type())
       {
           case IDENTIFIER: return ParseAssignmentStatement();
           case KEYWORD:
               if(curr_Token -> get_token_string() == "begin")
                   return ParseBlockStatement();
               else if(curr_Token -> get_token_string() == "if")
                   return ParseIfStatement();
               else if(curr_Token -> get_token_string() == "while")
                   return ParseLoopStatement();
               else if( curr_Token -> get_token_string() == "call")
                    return ParseCallStatement();
               else
               {
                   fprintf(stderr, "Could not recognize statement");
               }
       }
    }
    void HandleIfStatement()
    {
        if(ParseIfStatement())
        {
            fprintf(stderr, "Parsed an if statement");
        }
        else
        {
            get_next_token();
        }
    }
    void HandleLoopStatement()
    {
        if(ParseLoopStatement())
        {
            fprintf(stderr, "Parsed an if statement");
        }
        else
        {
            get_next_token();
        }
    }

    void HandleDefinition()
    {
       if(ParseDefinition()){
           fprintf(stderr, "Parsed a function definiton\n");
       }
       else
       {
           get_next_token();
       }
    }

    void HandleTopLevelExpression(){
        if(ParseTopLevelExpr()){
            fprintf(stderr, "Parsed a top level expr\n");
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
                    if(curr_Token ->get_token_string() == "def")
                        HandleDefinition();
                    else if(curr_Token -> get_token_string() == "if")
                        HandleIfStatement();
                    else if(curr_Token -> get_token_string() == "while")
                        HandleLoopStatement();
                    break;

                default:
                    if(curr_Token -> get_token_string() == ";")
                        get_next_token();
                    else
                        HandleTopLevelExpression();
                    break;
            }
        }
    }

};


