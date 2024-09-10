
#include <cstdint>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


#include "MCP300x.hpp"


MCP300x::MCP300x(void) : spi_mode(MCP300x::MODE), spi_bpw(MCP300x::BPW), spi_delay(MCP300x::DELAY), spi_speed(MCP300x::SPEED_3V3_MAX_HZ)
{
    this->spibus = open(MCP300x::spidev_path, O_RDWR);
    if (this->spibus) {
        throw std::runtime_error("error opening spi bus.");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_WR_MODE &this->spi_mode)) {
        throw std::runtime_error("MCP300x read ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_MODE, &this->spi_mode)) {
        throw std::runtime_error("MCP300x read ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_WR_BITS_PER_WORD, &this->spi_bpw)) {
        throw std::runtime_error("MCP300x read ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_BITS_PER_WORD, &this->spi_bpw)) {
        throw std::runtime_error("MCP300x read ioctl failed");
    }
    this->set_speed(this->spi_speed);
}


MCP300x::~MCP300x()
{
    close(this->spibus);
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
}


std::uint16_t MCP300x::read(std::uint_fast8_t channel)
{
    std::uint8_t buffer[3];

    buffer[0] = MCP300x::TX_WORD1;
    buffer[1] = MCP300x::MCP300x::TX_WORD2.at(channel);
    buffer[2] = MCP300x::TX_WORD3;
    this->spi_ch_transfer.tx_buf = reinterpret_cast<std::uintptr_t>(buffer);
    this->spi_ch_transfer.rx_buf = reinterpret_cast<std::uintptr_t>(buffer);
    if (0 >ioctl(this->spibus, SPI_IOC_MESSAGE(1), &this->spi_ch_transfer)) {
        throw std::runtime_error("MCP300x::read() ioctl() failed");
    }
    // Validating
    if (0 != (buffer[0] & MCP300x::RX_WORD1_MASK)) {
        throw std::runtime_error("MCP300x::read() RX error");
    }
    // Null bit check
    if (0 != (buffer[1] & MCP300x::RX_WORD2_NULL_BIT_MASK)) {
        throw std::runtime_error("MCP300x::read() RX no null bit");
    }
    return static_cast<std::uint16_t>((buffer[1] * 256 +  buffer[2]) & MCP300x::tenbit_mask);
}
