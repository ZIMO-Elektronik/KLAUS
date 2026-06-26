/**
 * MDU_EIN Update process
 *
 * \file    include/process/mdu_ein/update.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <functional>
#include <libklug/libklug.hpp>
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
  virtual ~SoundLoad() final;

  virtual bool execute();
  virtual void abort();

private:
  void handle_result(res::Result r);

  void modeAction();
  void modeResult(res::Result const r);

  void enterAction();
  void enterResult(res::Result const r);

  void configAction();
  void configResult(res::Result const r);

  void searchAction();
  void searchResult(res::Result const r);

  void initAction();
  void initResult(res::Result const r);

  void eraseAction();
  void eraseResult(res::Result const r);

  void waitAction();
  void waitResult(res::Result const r);

  void updateAction();
  void updateResult(res::Result const r);

  void endAction();
  void endResult(res::Result const r);

  void exitAction();
  void exitResult(res::Result const r);

  void resetAction();
  void resetResult(res::Result const r);

  void (SoundLoad::*_state)(res::Result const){
    &SoundLoad::modeResult}; ///< State (next *Result handler)

  libklug::ZPP _zpp; ///< ZPP instance

  int _err_cnt{0};       ///< Consecutive error counter
  unsigned int _index{}; ///< Block index withing the ZPP

  bool _abort{}; ///< Abort flag
};

} // namespace process::mdu_ein
