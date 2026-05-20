/**
 * Cv
 *
 * \file    include/type/cv/cv.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <cstdint>

namespace type::cv {

/**
 * Cv
 *
 */
struct Cv {
  uint16_t address{}; ///< Cv address
  uint8_t value{};    ///< Cv value
};

} // namespace type::cv
