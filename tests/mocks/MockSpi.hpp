#ifndef TESTS_MOCKS_MOCK_SPI_HPP
#define TESTS_MOCKS_MOCK_SPI_HPP

#include "ISpi.hpp"
#include <gmock/gmock.h>

class MockSpi : public cxx_hal::ISpi {
public:
    MOCK_METHOD(bool, transmit, (std::span<const uint8_t> tx_data), (noexcept, override));
    MOCK_METHOD(bool, receive, (std::span<uint8_t> rx_data), (noexcept, override));
    MOCK_METHOD(bool, transfer, ((std::span<const uint8_t> tx_data), (std::span<uint8_t> rx_data)), (noexcept, override));
};

#endif // TESTS_MOCKS_MOCK_SPI_HPP
