
#include <iostream>

#include <pigpio.h>

#include "MCP300x.hpp"

int main()
{
    if (0 > gpioInitialise()) {
        std::cerr << "gpioInitialize failed" << std::endl;
	return 1;
    }

    MCP300x mcp300x;
    std::array<float, 8> result;
    std::array<bool, 8> channel = {true, false, false, false, false, false, false, true};
    constexpr float scaling = 29.8 / 9.8;
    for (auto i = 0; i < 100; i++) {
	mcp300x.read_v(result, channel);
        std::cout << result[0] << " " << scaling * result[7] << std::endl;
    }
    gpioTerminate();
}
