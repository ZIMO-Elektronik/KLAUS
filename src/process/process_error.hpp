/**
 * Copyright (C) 2026 ZIMO Elektronik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://gnu.org>.
 *
 *
 *
 *
 *
 * Process error exception
 *
 * \file    process/process_error.hpp
 * \author  Jonas Gahlert
 * \date    21.09.2026
 */

#pragma once

#include <stdexcept>
#include "type/step.hpp"

namespace process {

struct process_error : public std::runtime_error {
  process_error(type::ProcessUpdate const& u, char const* what_arg)
    : _update{u}, runtime_error{what_arg} {}
  process_error(process_error const&) = default;

  operator type::ProcessUpdate() const { return _update; }

private:
  type::ProcessUpdate _update{};
};

} // namespace process
