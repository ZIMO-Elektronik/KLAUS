/**
 *
 *
 * \file    include/process/susiv2/soundload.cpp
 * \author  Jonas Gahlert
 * \date    02.06.2026
 */

#pragma once

#include <functional>
#include <libklug/libklug.hpp>
#include <vector>
#include "include/process/base.hpp"
#include "include/process/i_soundload_process.hpp"
#include "include/type/step.hpp"

namespace process::susiv2 {

struct SoundLoad : public Base, public ISoundLoadProcess {
  SoundLoad(std::filesystem::path path);
  virtual ~SoundLoad();

  virtual bool execute();
  virtual void abort();

  virtual void onUpdateProgress(std::function<void(double)>);
  virtual void onUpdateStep(std::function<void(type::SoundLoadStep)>);

private:
  void handle_result(res::Result r);

  void modeAction();
  void modeResult(res::Result const& r);

  void featuresAction();
  void featuresResult(res::Result const& r);

  void eraseAction();
  void eraseResult(res::Result const& r);

  void loadAction();
  void loadResult(res::Result const& r);

  void endAction();
  void endResult(res::Result const& r);

  void resetAction();
  void resetResult(res::Result const& r);

  void (SoundLoad::*_state)(res::Result const&){};

  std::function<void(type::SoundLoadStep)> _stepCb{};
  std::function<void(double)> _progressCb{};

  libklug::ZPP _zpp;

  int _err_cnt{0};

  bool _abort{};

  unsigned int _index{};
};

} // namespace process::susiv2
