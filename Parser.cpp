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

                fprintf(stderr, "Expected number identifier or brackets but got %s :: %s",
                        curr_Token->get_token_string(), curr_Token -> get_token_type());
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
                                          (' ', curr_expr_tree, nullptr));

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
        auto curr_expr_tree = new BinaryExprAST(' ', LHS, nullptr);
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
                                          (' ', curr_expr_tree, nullptr));

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
        auto curr_expr_tree = new BinaryExprAST(' ', LHS, nullptr);
        return ParseExprDash(curr_expr_tree);

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


