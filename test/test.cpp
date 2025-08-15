#include <ucanutils/can.hpp>

#include <cassert>
#include <cmath>
#include <print>

using namespace ucanutils::can;

// ============================================================================
// Signal descriptors for testing
// ============================================================================

// 8-bit unsigned Intel signal at byte 0
inline constexpr signal_desc u8_intel_desc{
    .start_bit = 0,
    .bit_length = 8,
    .order = byte_order::little_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

// 16-bit unsigned Intel signal at byte 2
inline constexpr signal_desc u16_intel_desc{
    .start_bit = 16,
    .bit_length = 16,
    .order = byte_order::little_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

// 8-bit signed Intel signal at byte 4
inline constexpr signal_desc s8_intel_desc{
    .start_bit = 32,
    .bit_length = 8,
    .order = byte_order::little_endian,
    .is_signed = true,
    .factor = 1.0,
    .offset = 0.0,
};

// 16-bit unsigned Motorola signal, MSB at DBC bit 7 (byte 0 MSB)
inline constexpr signal_desc u16_moto_desc{
    .start_bit = 7,
    .bit_length = 16,
    .order = byte_order::big_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

// 12-bit Motorola signal starting at bit 39 (byte 4 MSB)
inline constexpr signal_desc u12_moto_desc{
    .start_bit = 39,
    .bit_length = 12,
    .order = byte_order::big_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

// 1-bit signal
inline constexpr signal_desc u1_intel_desc{
    .start_bit = 5,
    .bit_length = 1,
    .order = byte_order::little_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

// Signal with factor and offset
inline constexpr signal_desc scaled_desc{
    .start_bit = 0,
    .bit_length = 16,
    .order = byte_order::little_endian,
    .is_signed = false,
    .factor = 0.25,
    .offset = 0.0,
};

// Signed signal with offset
inline constexpr signal_desc temp_desc{
    .start_bit = 16,
    .bit_length = 8,
    .order = byte_order::little_endian,
    .is_signed = true,
    .factor = 1.0,
    .offset = -40.0,
};

// ============================================================================
// Signal type aliases
// ============================================================================

using u8_intel = signal<u8_intel_desc>;
using u16_intel = signal<u16_intel_desc>;
using s8_intel = signal<s8_intel_desc>;
using u16_moto = signal<u16_moto_desc>;
using u12_moto = signal<u12_moto_desc>;
using u1_intel = signal<u1_intel_desc>;
using scaled_sig = signal<scaled_desc>;
using temp_sig = signal<temp_desc>;

// ============================================================================
// Compile-time tests (static_assert)
// ============================================================================

// raw_type deduction
static_assert(std::is_same_v<u8_intel::raw_type, uint8_t>);
static_assert(std::is_same_v<u16_intel::raw_type, uint16_t>);
static_assert(std::is_same_v<s8_intel::raw_type, int8_t>);

// Intel 8-bit round-trip
static_assert([] {
  payload<8> p{};
  u8_intel::encode_raw(p, 0xAB);
  return u8_intel::decode_raw(p) == 0xAB;
}());

// Intel 16-bit round-trip
static_assert([] {
  payload<8> p{};
  u16_intel::encode_raw(p, 0x1234);
  return u16_intel::decode_raw(p) == 0x1234;
}());

// Intel signed negative round-trip
static_assert([] {
  payload<8> p{};
  s8_intel::encode_raw(p, -42);
  return s8_intel::decode_raw(p) == -42;
}());

// Intel signed min/max
static_assert([] {
  payload<8> p{};
  s8_intel::encode_raw(p, -128);
  return s8_intel::decode_raw(p) == -128;
}());

static_assert([] {
  payload<8> p{};
  s8_intel::encode_raw(p, 127);
  return s8_intel::decode_raw(p) == 127;
}());

// Motorola 16-bit round-trip
static_assert([] {
  payload<8> p{};
  u16_moto::encode_raw(p, 0x1234);
  return u16_moto::decode_raw(p) == 0x1234;
}());

// Motorola 12-bit round-trip
static_assert([] {
  payload<8> p{};
  u12_moto::encode_raw(p, 0xABC);
  return u12_moto::decode_raw(p) == 0xABC;
}());

// 1-bit signal
static_assert([] {
  payload<8> p{};
  u1_intel::encode_raw(p, 1);
  return u1_intel::decode_raw(p) == 1;
}());

static_assert([] {
  payload<8> p{};
  u1_intel::encode_raw(p, 0);
  return u1_intel::decode_raw(p) == 0;
}());

// Multiple signals don't interfere (Intel)
static_assert([] {
  payload<8> p{};
  u8_intel::encode_raw(p, 0xFF);
  u16_intel::encode_raw(p, 0xBEEF);
  s8_intel::encode_raw(p, -1);
  return u8_intel::decode_raw(p) == 0xFF
         && u16_intel::decode_raw(p) == 0xBEEF
         && s8_intel::decode_raw(p) == -1;
}());

// Motorola: known byte layout verification
// 16-bit Motorola starting at DBC bit 7 with value 0x1234:
// Byte 0 = 0x12, Byte 1 = 0x34
static_assert([] {
  payload<8> p{};
  u16_moto::encode_raw(p, 0x1234);
  return p[0] == 0x12 && p[1] == 0x34;
}());

// Message with base frame format
using test_msg = message<base_frame_format, 0x100, 8,
                         u8_intel_desc,
                         u16_intel_desc,
                         s8_intel_desc>;

static_assert(test_msg::id == 0x100);
static_assert(test_msg::payload_size == 8);

// Message make_frame
static_assert([] {
  auto f = test_msg::make_frame();
  return f.id == 0x100 && f.data[0] == 0;
}());

// Message encode/decode
static_assert([] {
  auto f = test_msg::make_frame();
  test_msg::encode_raw<u8_intel_desc>(f, 42);
  test_msg::encode_raw<u16_intel_desc>(f, 1000);
  test_msg::encode_raw<s8_intel_desc>(f, -10);
  return test_msg::decode_raw<u8_intel_desc>(f) == 42
         && test_msg::decode_raw<u16_intel_desc>(f) == 1000
         && test_msg::decode_raw<s8_intel_desc>(f) == -10;
}());

// Extended frame format with 29-bit ID
inline constexpr signal_desc ext_sig_desc{
    .start_bit = 0,
    .bit_length = 32,
    .order = byte_order::little_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

using ext_msg = message<extended_frame_format, 0x18FEF100, 8,
                         ext_sig_desc>;

static_assert(ext_msg::id == 0x18FEF100);

// Smaller payload size
inline constexpr signal_desc small_sig_desc{
    .start_bit = 0,
    .bit_length = 16,
    .order = byte_order::little_endian,
    .is_signed = false,
    .factor = 1.0,
    .offset = 0.0,
};

using small_msg = message<base_frame_format, 0x200, 4,
                           small_sig_desc>;

static_assert([] {
  auto f = small_msg::make_frame();
  small_msg::encode_raw<small_sig_desc>(f, 0xABCD);
  return small_msg::decode_raw<small_sig_desc>(f) == 0xABCD;
}());

// ============================================================================
// Runtime tests
// ============================================================================

int main() {
  // Scaled signal test
  {
    payload<8> p{};
    scaled_sig::encode(p, 1500.0);
    double decoded = scaled_sig::decode(p);
    assert(std::abs(decoded - 1500.0) < 0.5);
    std::println("scaled signal: encode(1500.0) -> decode() = {}",
                 decoded);
  }

  // Temperature signal with offset
  {
    payload<8> p{};
    temp_sig::encode(p, 25.0);
    double decoded = temp_sig::decode(p);
    assert(std::abs(decoded - 25.0) < 1.0);
    std::println("temp signal: encode(25.0) -> decode() = {}",
                 decoded);

    // Raw value should be 25 - (-40) = 65
    auto raw = temp_sig::decode_raw(p);
    assert(raw == 65);
    std::println("  raw value = {}", raw);
  }

  // Motorola round-trip runtime
  {
    payload<8> p{};
    u16_moto::encode_raw(p, 0x1234);
    auto decoded = u16_moto::decode_raw(p);
    assert(decoded == 0x1234);
    std::println("motorola u16: 0x1234 -> bytes [{:#04x}, {:#04x}] "
                 "-> decoded {:#06x}",
                 p[0], p[1], decoded);
  }

  // Motorola 12-bit
  {
    payload<8> p{};
    u12_moto::encode_raw(p, 0xABC);
    auto decoded = u12_moto::decode_raw(p);
    assert(decoded == 0xABC);
    std::println("motorola u12: 0xABC -> decoded {:#05x}", decoded);
  }

  // Message-level test (base frame)
  {
    auto f = test_msg::make_frame();
    test_msg::encode_raw<u8_intel_desc>(f, 0xFF);
    test_msg::encode_raw<u16_intel_desc>(f, 12345);
    test_msg::encode_raw<s8_intel_desc>(f, -99);

    auto v1 = test_msg::decode_raw<u8_intel_desc>(f);
    auto v2 = test_msg::decode_raw<u16_intel_desc>(f);
    auto v3 = test_msg::decode_raw<s8_intel_desc>(f);
    assert(v1 == 0xFF);
    assert(v2 == 12345);
    assert(v3 == -99);
    std::println("message (base): u8={}, u16={}, s8={}", v1, v2, v3);
  }

  // Extended frame message test
  {
    auto f = ext_msg::make_frame();
    ext_msg::encode_raw<ext_sig_desc>(f, 0xDEADBEEF);
    auto decoded = ext_msg::decode_raw<ext_sig_desc>(f);
    assert(decoded == 0xDEADBEEF);
    std::println("message (extended): u32={:#010x}", decoded);
  }

  // Small payload test
  {
    auto f = small_msg::make_frame();
    small_msg::encode_raw<small_sig_desc>(f, 0x1234);
    auto decoded = small_msg::decode_raw<small_sig_desc>(f);
    assert(decoded == 0x1234);
    std::println("message (4-byte): u16={:#06x}", decoded);
  }

  std::println("all tests passed");
  return 0;
}
