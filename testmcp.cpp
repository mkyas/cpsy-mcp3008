
#include <iostream>

#include "MCP300x.hpp"

int main()
{
    MCP300x mcp300x;

    std::cout << mcp300x.read() << std::endl;
}
