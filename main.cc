#include <iostream>
#include <string>
#include "huffman"

int main(int argc, char const *argv[]) {
    std::string filename, arg;
    Huffman obj;
    if(argc > 1)
    {
        for(int i = 1; i < argc; i++)
        {
            arg = argv[i];
            if (arg.compare("-f") == 0)
            filename = argv[++i];
            else if (arg.compare("-c") == 0)
            {
            if(!obj.compress(filename))
                return 1;
            }
            else if (arg.compare("-d") == 0)
            {
            if(!obj.decompress(filename))
                return 1;
            }
            else
            std::cerr << "unknown option" << std::endl;
        }
    }
    else
        std::cerr << "Invalid arguments" << std::endl;
    return 0;
}
