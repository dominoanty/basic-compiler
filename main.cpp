#include <iostream>
#include <string>
#include <fstream>
#include "Parser.cpp"


int main()
{
    std::string input_string;
    std::string input;
    std::fstream input_file;

    Parser *P;


    input_file.open("input.txt");

    std::getline(input_file, input);

    while(input != ""){
        input_string += input;
	input_string += " ";
	std::getline(input_file, input);
    }

    std::cout<<input_string;

    P = new Parser(input_string);
    P->MainLoop();


    return 0;
}
