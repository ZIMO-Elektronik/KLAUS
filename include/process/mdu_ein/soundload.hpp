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
#include "include/process/i_soundload_process.hpp"
#include "include/type/cv/cv.hpp"
#include "include/type/step.hpp"

namespace process::mdu_ein {

/**
 * Update process

 */
struct SoundLoad : public Base, public ISoundLoadProcess {
  SoundLoad(std::filesystem::path path);
  virtual ~SoundLoad();

  virtual void execute();
  virtual void abort();

  virtual void onUpdateProgress(std::function<void(double)>);
  virtual void onUpdateStep(std::function<void(type::SoundLoadStep)>);

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

  void (SoundLoad::*_state)(res::Result const){&SoundLoad::modeResult};

  std::function<void(type::SoundLoadStep)> _updateStep{};
  std::function<void(double)> _updateProgress{};

  libklug::ZPP _zpp;

  int _err_cnt{0};
  unsigned int _index{};

  bool _abort{};
};

} // namespace process::mdu_ein
