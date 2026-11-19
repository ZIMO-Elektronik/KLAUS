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
 * \file    process/decup_ein/update.hpp
 * \author  Jonas Gahlert
 * \date    18.11.2026
 */

#pragma once

#include <ulf/cpp/libulf.hpp>
#include "process/base.hpp"

namespace process::decup_ein {

/**
 * DECUP_EIN Update process
 */
struct Update : public Base {
  Update(std::filesystem::path path);
  Update(std::shared_ptr<libulf::ZSU> zsu);
  virtual ~Update() final;

  virtual bool execute() override;

private:
  void update();

  void mode();
  void enter();
  libulf::ZSU::Firmware search();
  void setup(libulf::ZSU::Firmware& fw);
  void write(libulf::ZSU::Firmware& fw);
  void end();

  void checkAbort();

  std::shared_ptr<libulf::ZSU> _zsu; ///< ZSU instance
};

} // namespace process::decup_ein
