/**
 * Base process
 *
 * \file    include/process/base.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <libklug/libklug.hpp>
#include "i_process.hpp"

namespace process {

/**
 * Base process
 *
 */
struct Base : IProcess {
  Base();            // Create handle
  ~Base() = default; // Destroy handle

  bool connect();
  void disconnect();

  virtual bool done() override;

  virtual void onUpdate(std::function<void(type::ProcessUpdate)>) override;

protected:
  libklug::LibKLUG _lib;

  bool _done{false};

  std::function<void(type::ProcessUpdate)> _updateCb{};
};

} // namespace process
