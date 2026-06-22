/**
 * CvProgrammer backend
 *
 * \file    include/ui/cv/cv_programmer.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <slint.h>
#include "include/ui/i_backend.hpp"

namespace ui::cv {

/**
 * CvProgrammer backend
 *
 */
struct CvProgrammer : public IBackend {
  virtual void connect(slint::ComponentHandle<AppWindow> window) override {
    _cvs->push_back({.address = 1,
                     .value = 3,
                     .name = "Address",
                     .description = "Address description"});
    _cvs->push_back(
      {.address = 3, .value = 145, .name = "Vendor ID", .description = ""});

    window->set_cv_list(_cvs);

    window->on_cv_data_changed([&](int const index, Cv const& cv) {
      return this->onCvDataChanged(index, cv);
    });
  }

  void onCvDataChanged(int index, Cv const& cv) {
    if (index < _cvs->row_count()) return;
    _cvs->set_row_data(index, cv);
  }

  void check() {
    if (auto const maybe_data{_cvs->row_data(0)}) {
      auto const data{*maybe_data};
      if (data.selected) {
        std::cout << "Selected" << std::endl;
      } else {
        std::cout << "Not selected" << std::endl;
      }
    }
  }

private:
  std::shared_ptr<slint::VectorModel<Cv>> _cvs{
    std::make_shared<slint::VectorModel<Cv>>()};
};

} // namespace ui::cv
