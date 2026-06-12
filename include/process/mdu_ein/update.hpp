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
#include "include/process/i_update_process.hpp"
#include "include/type/cv/cv.hpp"
#include "include/type/step.hpp"

namespace process::mdu_ein {

/**
 * Update process

 */
struct Update : public Base, public IUpdateProcess {
  Update(std::filesystem::path path);
  virtual ~Update();

  virtual bool execute();
  virtual void abort();

  virtual void onUpdateProgress(std::function<void(double)>);
  virtual void onUpdateStep(std::function<void(type::UpdateStep)>);

  virtual void onUpdate(std::function<void(type::ProcessUpdate)>) override;

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

  void verifyAction();
  void verifyResult(res::Result const r);

  void endAction();
  void endResult(res::Result const r);

  void resetAction();
  void resetResult(res::Result const r);

  void (Update::*_state)(res::Result const){&Update::modeResult};

  std::function<void(type::UpdateStep)> _updateStep{};
  std::function<void(double)> _updateProgress{};

  std::function<void(type::ProcessUpdate)> _updateCb{};

  libklug::ZSU _zsu;
  libklug::ZSU::FirmwareIterator _fwIt{_zsu.begin()};
  libklug::ZSU::FirmwareIterator const _fwItEnd{_zsu.end()};

  int _err_cnt{0};

  bool _abort{};

  unsigned int _index{};
};

} // namespace process::mdu_ein
