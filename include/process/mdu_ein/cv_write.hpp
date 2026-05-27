/**
 * MDU_EIN CvWrite process
 *
 * \file    include/process/mdu_ein/cv_write.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <libklug/libklug.h>
#include <functional>
#include <vector>
#include "include/process/base.hpp"
#include "include/process/i_cv_process.hpp"
#include "include/type/cv/cv.hpp"
#include "include/type/step.hpp"

namespace process::mdu_ein {

/**
 * CvWrite process

 */
struct CvWrite : public Base, public ICvProcess {
  using list_type = std::vector<type::cv::Cv>;

  CvWrite();
  virtual ~CvWrite();

  virtual void execute();
  virtual void abort();

  void handle();

private:
  using list_iterator_type = list_type::const_iterator;

  void handle_result(result const& r);

  void modeAction();
  void modeResult(result const& r);

  void enterAction();
  void enterResult(result const& r);

  void cvWriteAction();
  void cvWriteResult(result const& r);

  void resetAction();
  void resetResult(result const& r);

  void (CvWrite::*_state)(result const&){CvWrite::modeResult};

  std::function<void(type::CvStep)> _updateStep{};
  std::function<void(double)> _updateProgress{};

  libklug_handle _lib{};

  bool _abort{};

  list_type _cvs{};
  list_iterator_type _current{_cvs.begin()};
};

} // namespace process::mdu_ein
