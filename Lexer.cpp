#include <string>
#include <iostream>
#define EOF_TOKEN -1
#define IDENTIFIER -2
#define KEYWORD -3
#define NUMBER -4
#define SYMBOL -5


class Token
{
    std::string token_string;
    double token_val;
    int token_type;

public:

    Token(std::string token_string, int token_type)
            : token_string(token_string), token_type(token_type) {
        if(token_type == NUMBER)
        {
            token_val = std::stod(token_string);
        }
    }
    int equals(Token T)
    {

        if((this->token_string == T.token_string || this->token_val == T.token_val)
           && this->token_type == T.token_type)
            return 1;
        return 0;
    }
    std::string get_token_string()
    {
        return token_string;
    }
    int get_token_type()
    {
        return token_type;
    }
    double get_token_val()
    {
        return token_val;
    }


}Token_EOF("EOF", EOF_TOKEN);


class Scanner
{
    std::string input_string;
    int index;

public:
    Scanner(std::string new_string) : input_string(new_string) {index=0;}

    char get_char()
    {
        if(input_string[index] == '\0')
        {
            return EOF_TOKEN;
        }
        else
        {
            return input_string[index++];

        }
    }
    char look_ahead()
    {
        if(input_string[index+1] == '\0')
        {
            return EOF_TOKEN;
        }
        else
        {
            return input_string[index+1];
        }
    }
    void rewind()
    {
        if(input_string[index] == '\0') return;
        if(index > 0)
            index--;
    }
};
class Lexer
{
    Scanner *S;

public:

    Lexer(std::string new_string)
    {
        S = new Scanner(new_string);

    }

    void print_string()
    {
        char curr_char;

        while((curr_char = S->get_char()) != EOF_TOKEN)
        {
            std::cout<<curr_char;
        }
    }

    int check_keyword(std::string test_string)
    {
        if(test_string == "if"        ||
           test_string == "then"      ||
           test_string == "while"     ||
           test_string == "do"        ||
           test_string == "const"     ||
           test_string == "var"       ||
           test_string == "procedure" ||
           test_string == "begin"     ||
           test_string == "end"       ||
           test_string == "call"      ||
           test_string == "def" )
           return KEYWORD;
        return IDENTIFIER;
    }
    Token* get_token()
    {
        std::string collected_string = "";

        char curr_char = S->get_char();

        //Clear Whitespaces
        while(isspace(curr_char))
            curr_char = S->get_char();

        //Collect keywords and identifiers
        if(isalpha(curr_char))
        {
            collected_string+=curr_char;
            curr_char = S->get_char();
            while(isalnum(curr_char) || curr_char == '_')
            {
                collected_string += curr_char;
                curr_char = S->get_char();
            }
            
            S->rewind();
            return new Token(collected_string, check_keyword(collected_string));
        }

        //Collect numbers
        if(isdigit(curr_char))
        {
            collected_string+=curr_char;
            curr_char = S->get_char();
            while(isdigit(curr_char) || curr_char == '.')
            {
                collected_string += curr_char;
                curr_char = S->get_char();
            }
            S->rewind();

            return new Token(collected_string, NUMBER);

        }

        //If EOF, return an EOF token
        if(curr_char == EOF_TOKEN)
            return new Token("EOF", EOF_TOKEN);

        //Collect symbols
        collected_string += curr_char;
        return new Token(collected_string, SYMBOL);


    }
};
