#include "include/ui/process_manager.hpp"
#include "include/ui/helper/message_id_to_string.hpp"

namespace ui {

/**
 * Connect UI events to methods
 */
void ProcessManager::connect(slint::ComponentHandle<AppWindow> ui) {
  _weakUi = slint::ComponentWeakHandle(ui);

  ui->global<ProgressViewContext>().set_has_process(_process != nullptr);
  ui->global<ProgressViewContext>().set_is_process_running(
    _process != nullptr ? !_process->done() : false);
  ui->global<ProgressViewContext>().on_abort([this]() { this->abort(); });
}

/**
 * Execute Process
 *
 * \warning It is neither checked, if a process exists, nor if it is already
 * running
 *
 * \todo Check for the main edge cases
 *
 * \return true   Executed
 * \return false  Not Executed
 */
bool ProcessManager::execute() {
  _process->onUpdate([this](type::ProcessUpdate update) {
    this->updateText(update.id, false);
    if (std::holds_alternative<bool>(update.payload)) done();

    if (update.progress) this->updateProgress(*update.progress);
  });

  auto success = _process->execute();
  if (!success) _process.reset();

  _tracker.reset();

  if (auto ui{_weakUi.lock()}) {
    (*ui)->global<ProgressViewContext>().set_has_process(_process == nullptr);
    (*ui)->global<ProgressViewContext>().set_is_process_running(success);
    (*ui)->global<ProgressViewContext>().set_is_open(true);
    (*ui)->global<ProgressViewContext>().set_progress(0.0);
    (*ui)->global<ProgressViewContext>().set_time_estimate(_tracker.estimate());

    if (!success)
      // Push Error when we cant execute
      // TODO: This shoud come from the process, for now we just assume the most
      // likely cause: The device was not found.
      (*ui)->global<ProgressViewContext>().set_step(
        "Unable to find Update Device");
  }

  return true;
}

/**
 * Abort process
 *
 * \warning The existence of a process is not checked
 */
void ProcessManager::abort() { _process->abort(); }

void ProcessManager::done() {
  slint::invoke_from_event_loop([this]() {
    if (auto ui{_weakUi.lock()}) {
      // Mark process as ended and popup progress
      (*ui)->global<ProgressViewContext>().set_is_process_running(false);
      (*ui)->global<ProgressViewContext>().set_is_open(true);
    }
  });
}

/**
 * Update Progress
 *
 * Updates the ProgressView with the current progress. Also makes use of the
 * process time tracker
 *
 * \todo Maybe move tracker update out of lambda
 *
 * \param progress  Progress
 */
void ProcessManager::updateProgress(double progress) {
  using std::operator""sv;
  slint::invoke_from_event_loop([this, progress]() {
    std::string_view str{_tracker.update(progress) ? _tracker.estimate()
                                                   : ""sv};
    if (auto ui{_weakUi.lock()}) {
      (*ui)->global<ProgressViewContext>().set_progress(progress);
      if (!str.empty())
        (*ui)->global<ProgressViewContext>().set_time_estimate({str.data()});
    }
  });
}

/**
 * Update Text
 *
 * Updates the ProgressView with the current message id. If the id given is
 * the same as the last id, no update is performed unless `force` is `true`.
 *
 * \param id    Message ID
 * \param force Force update
 */
void ProcessManager::updateText(type::MessageID id, bool force) {
  if (!force)
    if (id == _lastId) return;

  auto const str{helper::message_id_to_string(id)};
  slint::invoke_from_event_loop([this, str]() {
    if (auto ui{this->_weakUi.lock()}) {
      (*ui)->global<ProgressViewContext>().set_step(str.data());
    }
  });
}

} // namespace ui
