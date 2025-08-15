#pragma once

#include <ucanutils/can/signal.hpp>
#include <ucanutils/can/types.hpp>

#include <cstddef>

namespace ucanutils {
namespace can {

namespace detail {

// Compute the highest byte index touched by a signal.
consteval size_t signal_max_byte(signal_desc desc) {
  if (desc.order == byte_order::little_endian) {
    unsigned highest_bit = desc.start_bit + desc.bit_length - 1u;
    return highest_bit / 8u;
  } else {
    unsigned bits_in_first_byte = (desc.start_bit % 8u) + 1u;
    if (desc.bit_length <= bits_in_first_byte) {
      return desc.start_bit / 8u;
    }
    unsigned remaining = desc.bit_length - bits_in_first_byte;
    unsigned extra_bytes = (remaining + 7u) / 8u;
    return desc.start_bit / 8u + extra_bytes;
  }
}

consteval bool signal_fits_in_payload(signal_desc desc,
                                      size_t payload_size) {
  return signal_max_byte(desc) < payload_size;
}

// Compute the 64-bit bitmask of DBC bit positions occupied by a signal.
consteval uint64_t signal_bitmask(signal_desc desc) {
  uint64_t mask = 0;
  if (desc.order == byte_order::little_endian) {
    for (unsigned i = 0; i < desc.bit_length; ++i) {
      mask |= uint64_t{1} << (desc.start_bit + i);
    }
  } else {
    unsigned bit = desc.start_bit;
    for (unsigned i = 0; i < desc.bit_length; ++i) {
      mask |= uint64_t{1} << bit;
      if (bit % 8u == 0u) {
        bit = bit + 15u;
      } else {
        bit = bit - 1u;
      }
    }
  }
  return mask;
}

consteval bool signals_overlap(signal_desc a, signal_desc b) {
  return (signal_bitmask(a) & signal_bitmask(b)) != 0u;
}

template<signal_desc... Descs>
consteval bool any_signals_overlap() {
  if constexpr (sizeof...(Descs) < 2) {
    return false;
  } else {
    constexpr signal_desc descs[] = {Descs...};
    constexpr auto n = sizeof...(Descs);
    for (size_t i = 0; i < n; ++i) {
      for (size_t j = i + 1; j < n; ++j) {
        if (signals_overlap(descs[i], descs[j])) {
          return true;
        }
      }
    }
    return false;
  }
}

} // namespace detail

template<frame_format Format, typename Format::id_type Id,
         size_t PayloadSize, signal_desc... Descs>
struct message {
  static_assert(Id <= Format::id_max,
                "ID exceeds frame format range");
  static_assert(PayloadSize >= 1 && PayloadSize <= 64,
                "PayloadSize must be in [1, 64]");
  static_assert(
      (detail::signal_fits_in_payload(Descs, PayloadSize) && ...),
      "one or more signals exceed PayloadSize");
  static_assert(!detail::any_signals_overlap<Descs...>(),
                "signals in this message have overlapping bits");

  using format = Format;
  using frame_type = frame<Format, PayloadSize>;

  static constexpr typename Format::id_type id = Id;
  static constexpr size_t payload_size = PayloadSize;

  static constexpr frame_type make_frame() {
    return frame_type{.id = Id, .data = {}};
  }

  template<signal_desc Desc>
  static constexpr auto decode_raw(frame_type const& f) {
    return signal<Desc>::decode_raw(f.data);
  }

  template<signal_desc Desc>
  static constexpr double decode(frame_type const& f) {
    return signal<Desc>::decode(f.data);
  }

  template<signal_desc Desc>
  static constexpr void
  encode_raw(frame_type& f, typename signal<Desc>::raw_type raw) {
    signal<Desc>::encode_raw(f.data, raw);
  }

  template<signal_desc Desc>
  static constexpr void encode(frame_type& f, double physical) {
    signal<Desc>::encode(f.data, physical);
  }
};

} // namespace can
} // namespace ucanutils
