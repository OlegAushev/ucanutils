#pragma once

#include <cstdint>

namespace ucanutils {
namespace can {

enum class byte_order : uint8_t {
  little_endian,
  big_endian,
};

} // namespace can
} // namespace ucanutils
