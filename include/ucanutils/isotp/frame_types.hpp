#pragma once

#include <cstdint>

namespace ucanutils {
namespace isotp {

struct SingleFrame {
  uint8_t pci;
};

struct FirstFrame {};

struct ConsecutiveFrame {};

struct FlowControlFrame {};

} // namespace isotp
} // namespace ucanutils
