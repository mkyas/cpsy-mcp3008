
#include <cstdint>
#include <ctime>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

extern "C" {
#include <pigpio.h>
}

#include "MCP300x.hpp"

static const struct timespec fiftyns = { 0, 100 };
static const struct timespec hundredns = { 0, 100 };

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

    // Set mode
    std::uint8_t spi_mode{MCP300x::MODE};
    if (0 > ioctl(this->spibus, SPI_IOC_WR_MODE, &spi_mode)) {
        throw std::runtime_error("MCP300x() write mode ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_MODE, &spi_mode)) {
        throw std::runtime_error("MCP300x() read mode ioctl failed");
    }

    // Set bits per word
    std::uint8_t spi_bpw{MCP300x::BPW};
    if (0 > ioctl(this->spibus, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw)) {
        throw std::runtime_error("MCP300x() write BPW ioctl failed");
    }
    if (0 > ioctl(this->spibus, SPI_IOC_RD_BITS_PER_WORD, &spi_bpw)) {
        throw std::runtime_error("MCP300x() read BPW ioctl failed");
    }

    // Set speed (we may want to change it at runtime
    this->set_speed(this->spi_speed);

    // Set up the CS line. Set high to put the ADC into sleep mode.
    gpioSetMode(this->spi_cs, PI_OUTPUT);
    gpioWrite(this->spi_cs, 1);
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
    this->spi_ch_transfer.speed_hz = this->spi_speed;
}


std::uint16_t MCP300x::read_raw(std::uint_fast8_t channel)
{
    std::uint8_t buffer[3];

    // Read a sample
    gpioWrite(this->spi_cs, 0);
    nanosleep(&hundredns, nullptr);
    buffer[0] = MCP300x::TX_WORD1;
    buffer[1] = MCP300x::MCP300x::TX_WORD2.at(channel);
    buffer[2] = MCP300x::TX_WORD3;
    this->spi_ch_transfer.tx_buf = reinterpret_cast<std::uintptr_t>(buffer);
    this->spi_ch_transfer.rx_buf = reinterpret_cast<std::uintptr_t>(buffer);
    if (0 > ioctl(this->spibus, SPI_IOC_MESSAGE(1), &this->spi_ch_transfer)) {
        throw std::runtime_error("MCP300x::read_internal() ioctl() failed");
    }
    gpioWrite(this->spi_cs, 1);

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


float MCP300x::read_v(std::uint_fast8_t channel)
{
    std::uint16_t result = this->read_raw(channel);
    return this->reference_voltage * static_cast<float>(result) / 1023.0;
}

void MCP300x::read_raw(std::array<std::uint16_t, 8>& result, const std::array<bool, 8>& channel)
{
    for (auto i = 0; i < 8; i++) {
        if (channel[i]) {
            result[i] = this->read_raw(i);
        }
    }
}

void MCP300x::read_v(std::array<float, 8>& result, const std::array<bool, 8>& channel)
{
    for (auto i = 0; i < 8; i++) {
        if (channel[i]) {
            std::uint16_t r = this->read_raw(i);
            result[i] = this->reference_voltage * static_cast<float>(r) / 1023.0;
        }
    }
}
