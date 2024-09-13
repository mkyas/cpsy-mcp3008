
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

extern "C" {
#include <pigpio.h>
}

#include "MCP300x.hpp"


MCP300x::MCP300x(const int cs, const char* path)
	: spi_path(path), spi_cs(cs),
	  reference_voltage(3.3),
	  spi_speed(MCP300x::SPEED_3V3_MAX_HZ),
	  spi_ch_transfer()
{
    this->spibus = open(this->spi_path, O_RDWR);
    if (this->spibus < 0) {
        throw std::runtime_error("error opening spi bus.");
    }

    this->spi_ch_transfer.len = MCP300x::LEN;
    this->spi_ch_transfer.delay_usecs= MCP300x::DELAY;
    this->spi_ch_transfer.bits_per_word = MCP300x::BPW;

    gpioInitialise();
    gpioSetMode(this->spi_cs, PI_OUTPUT);
    gpioWrite(this->spi_cs, 1);

    std::uint8_t spi_mode{MCP300x::MODE};
    if (0 > ioctl(this->spibus, SPI_IOC_WR_MODE, &spi_mode)) {
        throw std::runtime_error("MCP300x() write mode ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_MODE, &spi_mode)) {
        throw std::runtime_error("MCP300x() read mode ioctl failed");
    }

    std::uint8_t spi_bpw{MCP300x::BPW};
    if (0 > ioctl(this->spibus, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw)) {
        throw std::runtime_error("MCP300x() write BPW ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_BITS_PER_WORD, &spi_bpw)) {
        throw std::runtime_error("MCP300x() read BPW ioctl failed");
    }

    this->set_speed(this->spi_speed);
}


MCP300x::~MCP300x()
{
    close(this->spibus);
    gpioTerminate();
}


void MCP300x::set_speed(std::uint32_t speed)
{
    this->spi_speed = speed;
    if (0 > ioctl(this->spibus, SPI_IOC_WR_MAX_SPEED_HZ, &this->spi_speed)) {
        throw std::runtime_error("MCP300x read ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_MAX_SPEED_HZ, &this->spi_speed)) {
        throw std::runtime_error("MCP300x read ioctl failed");
    }
    this->spi_ch_transfer.speed_hz = this->spi_speed;
}


std::uint16_t MCP300x::read_internal(std::uint_fast8_t channel)
{
    std::uint8_t buffer[3];

    buffer[0] = MCP300x::TX_WORD1;
    buffer[1] = MCP300x::MCP300x::TX_WORD2.at(channel);
    buffer[2] = MCP300x::TX_WORD3;
    this->spi_ch_transfer.tx_buf = reinterpret_cast<std::uintptr_t>(buffer);
    this->spi_ch_transfer.rx_buf = reinterpret_cast<std::uintptr_t>(buffer);
    if (0 > ioctl(this->spibus, SPI_IOC_MESSAGE(1), &this->spi_ch_transfer)) {
        throw std::runtime_error("MCP300x::read_internal() ioctl() failed");
    }
    // Validating
    if (0 != (buffer[0] & MCP300x::RX_WORD1_MASK)) {
        throw std::runtime_error("MCP300x::read_internal() RX error");
    }
    // Null bit check
    if (0 != (buffer[1] & MCP300x::RX_WORD2_NULL_BIT_MASK)) {
        throw std::runtime_error("MCP300x::read_internal() RX no null bit");
    }
    return static_cast<std::uint16_t>((buffer[1] * 256 +  buffer[2]) & MCP300x::tenbit_mask);
}


std::uint16_t MCP300x::read_raw(std::uint_fast8_t channel)
{
    gpioWrite(this->spi_cs, 0);
    std::uint16_t result = this->read_internal(channel);
    gpioWrite(this->spi_cs, 1);
    return result;
}

float MCP300x::read_v(std::uint_fast8_t channel)
{
    gpioWrite(this->spi_cs, 0);
    std::uint16_t result = this->read_internal(channel);
    gpioWrite(this->spi_cs, 1);
    return this->reference_voltage * static_cast<float>(result) / 1023.0;
}
