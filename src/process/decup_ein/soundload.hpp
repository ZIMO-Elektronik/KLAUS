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
 * DECUP_EIN SoundLoad process
 *
 * \file    process/decup_ein/soundload.hpp
 * \author  Jonas Gahlert
 * \date    18.11.2026
 */

#pragma once

#include <ulf/cpp/libulf.hpp>
#include "process/base.hpp"

namespace process::decup_ein {

/**
 * DECUP_EIN SoundLoad process
 *
 */
struct SoundLoad : public Base {
  SoundLoad(std::filesystem::path path);
  SoundLoad(std::shared_ptr<libulf::ZPP> zpp);
  virtual ~SoundLoad() final;

  virtual bool execute() override;

private:
  void load();

  void mode();
  void enter();
  void setup();
  void erase();
  void write();
  void writeCVs();

  void checkAbort();

  std::shared_ptr<libulf::ZPP> _zpp; ///< ZPP instance
};

} // namespace process::decup_ein
