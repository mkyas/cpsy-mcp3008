
#include <iostream>

#include <pigpio.h>

#include "MCP300x.hpp"

int main()
{
    MCP300x mcp300x;

    if (0 > gpioInitialise()) {
	    std::cerr << "Failed to initialise GPIO" << std::endl;
	    return -1;
    }
    std::cout << mcp300x.read() << std::endl;
    gpioTerminate();
}
