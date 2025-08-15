#pragma once

#include <ucanutils/can/byte_order.hpp>

#include <cstdint>

namespace ucanutils {
namespace can {

struct signal_desc {
  uint8_t start_bit;
  uint8_t bit_length;
  byte_order order;
  bool is_signed;
  double factor;
  double offset;
};

namespace detail {

template<uint8_t BitLength>
consteval auto deduce_unsigned_raw_type() {
  static_assert(BitLength >= 1 && BitLength <= 64,
                "bit_length must be in [1, 64]");
  if constexpr (BitLength <= 8) {
    return uint8_t{};
  } else if constexpr (BitLength <= 16) {
    return uint16_t{};
  } else if constexpr (BitLength <= 32) {
    return uint32_t{};
  } else {
    return uint64_t{};
  }
}

template<uint8_t BitLength>
consteval auto deduce_signed_raw_type() {
  static_assert(BitLength >= 1 && BitLength <= 64,
                "bit_length must be in [1, 64]");
  if constexpr (BitLength <= 8) {
    return int8_t{};
  } else if constexpr (BitLength <= 16) {
    return int16_t{};
  } else if constexpr (BitLength <= 32) {
    return int32_t{};
  } else {
    return int64_t{};
  }
}

} // namespace detail

template<signal_desc Desc>
using raw_type_t = decltype([] {
  if constexpr (Desc.is_signed) {
    return detail::deduce_signed_raw_type<Desc.bit_length>();
  } else {
    return detail::deduce_unsigned_raw_type<Desc.bit_length>();
  }
}());

} // namespace can
} // namespace ucanutils
