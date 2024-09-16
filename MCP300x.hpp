


#ifndef MCP300X_HPP_INCLUDED
#define MCP300X_HPP_INCLUDED

#include <cstdint>
#include <array>

#include <linux/spi/spidev.h>

class MCP300x
{
private:
    static constexpr std::uint16_t tenbit_mask = 0b0000001111111111;

    static constexpr std::uint8_t TX_WORD1     = 0b00000001;
    // These configurations are for single-ended conversions.
    static constexpr std::uint8_t TX_WORD2_CH0 = 0b10000000;
    static constexpr std::uint8_t TX_WORD2_CH1 = 0b10010000;
    static constexpr std::uint8_t TX_WORD2_CH2 = 0b10100000;
    static constexpr std::uint8_t TX_WORD2_CH3 = 0b10110000;
    static constexpr std::uint8_t TX_WORD2_CH4 = 0b11000000;
    static constexpr std::uint8_t TX_WORD2_CH5 = 0b11010000;
    static constexpr std::uint8_t TX_WORD2_CH6 = 0b11100000;
    static constexpr std::uint8_t TX_WORD2_CH7 = 0b11110000;
    static constexpr std::uint8_t TX_WORD3     = 0b00000000;

    static constexpr const std::array<std::uint8_t, 8> TX_WORD2 = {
        MCP300x::TX_WORD2_CH0,
        MCP300x::TX_WORD2_CH1,
        MCP300x::TX_WORD2_CH2,
        MCP300x::TX_WORD2_CH3,
        MCP300x::TX_WORD2_CH4,
        MCP300x::TX_WORD2_CH5,
        MCP300x::TX_WORD2_CH6,
        MCP300x::TX_WORD2_CH7,
    };

    static constexpr std::uint8_t RX_WORD1_MASK          = 0b00000000;
    static constexpr std::uint8_t RX_WORD2_NULL_BIT_MASK = 0b00000100;
    static constexpr std::uint8_t RX_WORD2_MASK          = 0b00000011;
    static constexpr std::uint8_t RX_WORD3_MASK          = 0b11111111;

    const char *spi_path;
    const int spi_cs;
    const float reference_voltage;
    // This is the SPI clock speed. The sample rate is 1/18 of this value.
    std::uint32_t spi_speed;
    int spibus;

    spi_ioc_transfer spi_ch_transfer;
public:
    static constexpr std::uint8_t CH_AMOUNT = 8;
    static constexpr std::uint8_t BPW = 8;
    static constexpr std::uint8_t MODE = 0;
    static constexpr std::uint8_t LEN = 3;
    static constexpr std::uint8_t DELAY = 0;
    static constexpr std::uint16_t RESOLUTION = 1023;
    static constexpr std::uint32_t SPEED_5V_MAX_HZ  = 3600000; // 200 ksps
    static constexpr std::uint32_t SPEED_3V3_MAX_HZ = 2340000; // 130 ksps
    static constexpr std::uint32_t SPEED_MIN_HZ = 10000;	/* 10kHz at 85 degree c. */

    explicit MCP300x(const int cs = 5, const char *path = "/dev/spidev0.1");
    MCP300x(const MCP300x&) = delete;
    ~MCP300x();

    void set_speed(std::uint32_t);
    std::uint16_t read_raw(std::uint_fast8_t channel = 0);
    float read_v(std::uint_fast8_t channel = 0);
    void read_raw(std::array<std::uint16_t, 8>&, const std::array<bool, 8>&);
    void read_v(std::array<float, 8>&, const std::array<bool, 8>&);
};

#endif
