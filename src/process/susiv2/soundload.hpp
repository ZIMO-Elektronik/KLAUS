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
 * SUSIV2 SoundLoad process
 *
 * \file    process/susiv2/soundload.cpp
 * \author  Jonas Gahlert
 * \date    02.06.2026
 */

#pragma once

#include <functional>
#include <ulf/cpp/libulf.hpp>
#include <vector>
#include "process/base.hpp"
#include "type/step.hpp"

namespace process::susiv2 {

/**
 * SoundLoad process
 *
 * \details
 * This class handles the ZUSI SoundLoad as a background process. Each stage of
 * the process is represented by a pair of *Action and *Result methods. Each
 * Action sets its corresponding Result handler, where the Resulthandler is
 * called from the LibULF callback after each transfer.
 *
 * The process can be started using \ref SoundLoad::execute and aborted using
 * \ref SoundLoad::abort, which will quit the event loop AFTER the next result
 * is available. To change this behaviour, the LibULF interface needs to be
 * extended with an abort function.
 *
 * \note
 * The process itself is designed as an eventloop. The Action starts a transfer,
 * Result handles the result once it is available and either calls the next
 * Action or exits.
 *
 * \todo SoundLoad needs to be able to take a ZPP instance by shared_ptr.
 *
 */
struct SoundLoad : public Base {
  SoundLoad(std::filesystem::path path);
  SoundLoad(std::shared_ptr<libulf::ZPP> zpp);
  virtual ~SoundLoad() final;

  virtual bool execute();

private:
  void load();

  void mode();
  void features();
  void erase();
  void write();
  void exit();

  void checkAbort();

  std::shared_ptr<libulf::ZPP> _zpp; ///< ZPP instance

  int _err_cnt{0}; ///< Consecutive error counter
};

} // namespace process::susiv2
