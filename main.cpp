#include <iostream>
#include <string>
#include "Parser.cpp"

int main()
{
    std::string input_string;

    Parser *P;

    std::cout<<"Enter required string : ";
    std::getline(std::cin, input_string);

    P = new Parser(input_string);
    P->MainLoop();


    return 0;
}
