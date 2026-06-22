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
#include "include/type/step.hpp"

namespace process::susiv2 {

struct SoundLoad : public Base {
  SoundLoad(std::filesystem::path path);
  virtual ~SoundLoad() final;

  virtual bool execute();
  virtual void abort();

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

  libklug::ZPP _zpp;

  int _err_cnt{0};

  bool _abort{};

  unsigned int _index{};
};

} // namespace process::susiv2
