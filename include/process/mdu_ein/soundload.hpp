/**
 * MDU_EIN Update process
 *
 * \file    include/process/mdu_ein/update.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <functional>
#include <klug/cpp/libklug.hpp>
#include <vector>
#include "include/process/base.hpp"
#include "include/type/step.hpp"

namespace process::mdu_ein {

/**
 * SoundLoad process
 *
 * \details
 * This class handles the MDU SoundLoad as a background process. Each stage of
 * the process is represented by a pair of *Action and *Result methods. Each
 * Action sets its corresponding Result handler, where the Resulthandler is
 * called from the LibKLUG callback after each transfer.
 *
 * The process can be started using \ref SoundLoad::execute and aborted using
 * \ref SoundLoad::abort, which will quit the event loop AFTER the next result
 * is available. To change this behaviour, the LibKLUG interface needs to be
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
  SoundLoad(std::shared_ptr<libklug::ZPP> zpp);
  virtual ~SoundLoad() final;

  virtual bool execute();

private:
  void load();

  void mode();
  void enter();
  void config();
  void search();
  void init();
  void erase();
  void write();
  void end();
  void exit();

  std::shared_ptr<libklug::ZPP> _zpp; ///< ZPP instance

  int _err_cnt{0}; ///< Consecutive error counter
};

} // namespace process::mdu_ein
