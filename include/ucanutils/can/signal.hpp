#pragma once

#include <ucanutils/can/bit_ops.hpp>
#include <ucanutils/can/signal_desc.hpp>
#include <ucanutils/can/types.hpp>

#include <cstddef>

namespace ucanutils {
namespace can {

template<signal_desc Desc>
struct signal {
  static_assert(Desc.bit_length >= 1 && Desc.bit_length <= 64,
                "bit_length must be in [1, 64]");
  static_assert(Desc.start_bit < 64,
                "start_bit must be in [0, 63]");
  static_assert(Desc.factor != 0.0,
                "factor must be non-zero");

  using raw_type = raw_type_t<Desc>;

  static constexpr signal_desc desc = Desc;

  template<size_t Size>
  static constexpr raw_type
  decode_raw(payload<Size> const& data) {
    uint64_t bits =
        detail::extract_raw<Desc.order>(data,
                                        Desc.start_bit,
                                        Desc.bit_length);
    if constexpr (Desc.is_signed) {
      return detail::sign_extend<Desc.bit_length, raw_type>(bits);
    } else {
      return static_cast<raw_type>(bits);
    }
  }

  template<size_t Size>
  static constexpr void
  encode_raw(payload<Size>& data, raw_type raw) {
    uint64_t bits;
    if constexpr (Desc.is_signed) {
      bits = static_cast<uint64_t>(raw)
             & detail::make_mask(Desc.bit_length);
    } else {
      bits = static_cast<uint64_t>(raw);
    }
    detail::insert_raw<Desc.order>(data,
                                   Desc.start_bit,
                                   Desc.bit_length,
                                   bits);
  }

  template<size_t Size>
  static constexpr double decode(payload<Size> const& data) {
    return static_cast<double>(decode_raw(data)) * Desc.factor
           + Desc.offset;
  }

  template<size_t Size>
  static constexpr void encode(payload<Size>& data, double physical) {
    auto raw = static_cast<raw_type>(
        (physical - Desc.offset) / Desc.factor);
    encode_raw(data, raw);
  }
};

} // namespace can
} // namespace ucanutils
