#pragma once

#include <stdexcept>
#include "type/step.hpp"

namespace process {

struct process_error : public std::runtime_error {
  process_error(type::ProcessUpdate const& u, char const* what_arg)
    : _update{u}, runtime_error{what_arg} {}
  process_error(process_error const&) = default;

  operator type::ProcessUpdate() const { return _update; }

private:
  type::ProcessUpdate _update{};
};

} // namespace process
