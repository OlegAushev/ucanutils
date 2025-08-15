#pragma once

#include <ucanutils/can/byte_order.hpp>
#include <ucanutils/can/types.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>

namespace ucanutils {
namespace can {
namespace detail {

constexpr uint64_t make_mask(uint8_t bit_length) {
  return (bit_length >= 64) ? ~uint64_t{0}
                            : ((uint64_t{1} << bit_length) - 1);
}

// Interpret payload as a little-endian uint64_t.
// Byte 0 is the least significant byte.
template<size_t Size>
constexpr uint64_t payload_to_le_u64(payload<Size> const& p) {
  if constexpr (Size == 8) {
    if constexpr (std::endian::native == std::endian::little) {
      return std::bit_cast<uint64_t>(p);
    } else {
      return std::byteswap(std::bit_cast<uint64_t>(p));
    }
  } else {
    uint64_t val = 0;
    for (size_t i = 0; i < Size; ++i) {
      val |= static_cast<uint64_t>(p[i]) << (i * 8u);
    }
    return val;
  }
}

template<size_t Size>
constexpr void le_u64_to_payload(uint64_t val, payload<Size>& p) {
  if constexpr (Size == 8) {
    if constexpr (std::endian::native == std::endian::little) {
      p = std::bit_cast<payload<8>>(val);
    } else {
      p = std::bit_cast<payload<8>>(std::byteswap(val));
    }
  } else {
    for (size_t i = 0; i < Size; ++i) {
      p[i] = static_cast<uint8_t>((val >> (i * 8u)) & 0xFFu);
    }
  }
}

// Interpret payload as a big-endian uint64_t.
// Byte 0 is the most significant byte.
template<size_t Size>
constexpr uint64_t payload_to_be_u64(payload<Size> const& p) {
  if constexpr (Size == 8) {
    if constexpr (std::endian::native == std::endian::little) {
      return std::byteswap(std::bit_cast<uint64_t>(p));
    } else {
      return std::bit_cast<uint64_t>(p);
    }
  } else {
    uint64_t val = 0;
    for (size_t i = 0; i < Size; ++i) {
      val |= static_cast<uint64_t>(p[i]) << ((Size - 1u - i) * 8u);
    }
    return val;
  }
}

template<size_t Size>
constexpr void be_u64_to_payload(uint64_t val, payload<Size>& p) {
  if constexpr (Size == 8) {
    if constexpr (std::endian::native == std::endian::little) {
      p = std::bit_cast<payload<8>>(std::byteswap(val));
    } else {
      p = std::bit_cast<payload<8>>(val);
    }
  } else {
    for (size_t i = 0; i < Size; ++i) {
      p[i] = static_cast<uint8_t>(
          (val >> ((Size - 1u - i) * 8u)) & 0xFFu);
    }
  }
}

// Convert DBC Motorola start_bit to MSB position in be-u64.
constexpr unsigned
motorola_msb_in_be_u64(uint8_t start_bit, size_t payload_size) {
  return static_cast<unsigned>(
      (payload_size - 1u - start_bit / 8u) * 8u
      + (start_bit % 8u));
}

// Extract raw bits from payload (unsigned result).
template<byte_order Order, size_t Size>
constexpr uint64_t extract_raw(payload<Size> const& data,
                               uint8_t start_bit,
                               uint8_t bit_length) {
  uint64_t mask = make_mask(bit_length);
  if constexpr (Order == byte_order::little_endian) {
    uint64_t val = payload_to_le_u64(data);
    return (val >> start_bit) & mask;
  } else {
    uint64_t val = payload_to_be_u64(data);
    unsigned msb_pos = motorola_msb_in_be_u64(start_bit, Size);
    unsigned lsb_pos = msb_pos - (bit_length - 1u);
    return (val >> lsb_pos) & mask;
  }
}

// Insert raw bits into payload.
template<byte_order Order, size_t Size>
constexpr void insert_raw(payload<Size>& data,
                          uint8_t start_bit,
                          uint8_t bit_length,
                          uint64_t raw) {
  uint64_t mask = make_mask(bit_length);
  raw &= mask;

  if constexpr (Order == byte_order::little_endian) {
    uint64_t val = payload_to_le_u64(data);
    val &= ~(mask << start_bit);
    val |= (raw << start_bit);
    le_u64_to_payload(val, data);
  } else {
    uint64_t val = payload_to_be_u64(data);
    unsigned msb_pos = motorola_msb_in_be_u64(start_bit, Size);
    unsigned lsb_pos = msb_pos - (bit_length - 1u);
    val &= ~(mask << lsb_pos);
    val |= (raw << lsb_pos);
    be_u64_to_payload(val, data);
  }
}

// Sign-extend a value from bit_length to the full width of Raw.
template<uint8_t BitLength, typename Raw>
constexpr Raw sign_extend(uint64_t val) {
  constexpr uint64_t sign_bit = uint64_t{1} << (BitLength - 1u);
  constexpr uint64_t mask = make_mask(BitLength);
  uint64_t masked = val & mask;
  if (masked & sign_bit) {
    masked |= ~mask;
  }
  return static_cast<Raw>(masked);
}

} // namespace detail
} // namespace can
} // namespace ucanutils
