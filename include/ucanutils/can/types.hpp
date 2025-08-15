#pragma once

#include <cstddef>
#include <emb/meta.hpp>

#include <array>
#include <cstdint>

namespace ucanutils {
namespace can {

struct base_frame_format {
  using id_type = uint32_t;
  static constexpr uint8_t id_bits = 11;
  static constexpr id_type id_max = 0x7FF;
};

struct extended_frame_format {
  using id_type = uint32_t;
  static constexpr uint8_t id_bits = 29;
  static constexpr id_type id_max = 0x1FFFFFFF;
};

template<typename T>
concept frame_format =
    emb::same_as_any<T, base_frame_format, extended_frame_format>;

template<size_t Size>
using payload = std::array<uint8_t, Size>;

template<frame_format FrameFormat, size_t PayloadSize>
struct frame {
  using format = FrameFormat;
  static constexpr size_t payload_size = PayloadSize;

  format::id_type id;
  payload<payload_size> data;
};

} // namespace can
} // namespace ucanutils
