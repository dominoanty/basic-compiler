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
    ExprAST* ParseExpression();

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
                if(curr_Token -> get_token_string() == '(') {
                    get_next_token(); // Consume (
                    auto result = ParseExpression(); // Generate expression tree
                    if(curr_Token -> get_token_string() != '(')
                        fprintf(stderr,"Error detected");
                    else
                        get_next_token(); // Consume )
                    return result;
                }
            default:
                fprintf(stderr, "Unkown token");
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
            auto Op = get_next_token(); //Consume the operator
            curr_expr_tree->setOp(Op);

            auto oldRHS = ParseFactor();
            curr_expr_tree->setRHS(oldRHS);

            auto new_expr_tree =
                    ParseTermDash(new BinaryExprAST
                                          ('', curr_expr_tree, nullptr));

        }
        else
        {
            return curr_expr_tree->getLHS();
        }
    }

   ExprAST* ParseTerm(){
        auto LHS = ParseFactor();
        if(!LHS)
            fprintf(stderr, "Error parsing");
        auto curr_expr_tree = new BinaryExprAST('', LHS, nullptr);
        auto Result = ParseExprDash(ExprAST* curr_expr_tree);

    }
    //PARSE EXPRESSIONS
    //E  -> TE'
    //E' -> +TE' | -TE' | epsilon
    ExprAST* ParseExprDash(BinaryExprAST* curr_expr_tree)
    {
        if(curr_Token -> get_token_string() == "+" ||
                curr_Token -> get_token_string() == "-")
        {
            auto Op = get_next_token(); //Consume the operator
            curr_expr_tree->setOp(Op);

            auto oldRHS = ParseTerm();
            curr_expr_tree->setRHS(oldRHS);

            auto new_expr_tree =
                    ParseExprDash(new BinaryExprAST
                                          ('', curr_expr_tree, nullptr));

        }
        else
        {
            return curr_expr_tree->getLHS();
        }
    }
    ExprAST* ParseExpression(){
        auto LHS = ParseTerm();
        if(!LHS)
            fprintf(stderr, "Error parsing");
        auto curr_expr_tree = new BinaryExprAST('', LHS, nullptr);
        return ParseExprDash(ExprAST* curr_expr_tree);

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
                    if(curr_Token -> get_token_string() == ';')
                        get_next_token();
                    else
                        HandleTopLevelExpression();
                    break;
            }
        }
    }

};


