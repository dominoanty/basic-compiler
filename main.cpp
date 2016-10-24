#include <iostream>
#include <string>
#include "Lexer.cpp"

int main()
{
    std::string input_string;

    Lexer *L;

    std::cout<<"Enter required string : ";
    std::getline(std::cin, input_string);

    L = new Lexer(input_string);
    Token* T;

    while(!(T = L->get_token())->equals(Token_EOF))
    {
       std::cout<<"\nCollected Token : "<< T->get_token_string();
       std::cout<<"\nToken Type : "<< T->get_token_type();
    }

    return 0;
}
