/**
 * MDU_EIN CvRead process
 *
 * \file    include/process/mdu_ein/cv_read.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <libklug/libklug.h>
#include <functional>
#include <vector>
#include <ztl/ztl.hpp>
#include "include/process/base.hpp"
#include "include/type/cv/cv.hpp"
#include "include/type/step.hpp"

namespace process::mdu_ein {

/**
 * CvRead process

 */

struct CvRead : public Base {
  using list_type = std::vector<type::cv::Cv>;

  CvRead();
  virtual ~CvRead();

  virtual void execute();
  virtual void abort();

  void handle(result const r);

private:
  using list_iterator_type = list_type::iterator;

  void modeAction();
  void modeResult(result const r);

  void enterAction();
  void enterResult(result const r);

  void cvReadAction();
  void cvReadResult(result const r);

  void resetAction();
  void resetResult(result const r);

  void (CvRead::*_state)(result const){CvRead::modeResult};

  std::function<void(type::CvStep)> _updateStep{};
  std::function<void(double)> _updateProgress{};

  libklug_handle _lib{};

  bool _abort{};

  list_type _cvs{};
  list_iterator_type _current{_cvs.begin()};
};

} // namespace process::mdu_ein
