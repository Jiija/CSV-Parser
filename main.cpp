#include <iostream>
#include <fstream>
#include "CSVParser/CSVParser.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Filename not passed to the program, finishing execution" << std::endl;
        return 0;
    }

    std::fstream file;
    file.open(argv[1]);
    if (!file.good())
    {
        std::cout << "Error: could not open the file" << std::endl;
        return 1;
    }

    try
    {
        CSVParser parser;
        parser.parse(file);
        file.close();
        parser.print(std::cout);
    }
    catch (const std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
