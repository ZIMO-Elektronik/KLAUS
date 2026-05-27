#include "include/ui/update.hpp"

namespace ui {

void Update::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_update_start_clicked([this]() { this->start(); });
  window->on_update_abort_clicked([this]() { this->abort(); });

  window->set_update_page_progress_title_text({"Update"});
  window->set_update_page_progress_step_text({"Awaiting start"});
  window->set_update_page_progress_progress(0.0);
}

void Update::start() {
  auto tmp_ = std::make_shared<process::mdu_ein::Update>();
  tmp_->setup(tmp_);
  _process = tmp_;
  _process->onUpdateStep(
    [this](type::UpdateStep const step) { this->updateStep(step); });
  _process->onUpdateProgress(
    [this](double progress) { this->updateProgress(progress); });

  _process->execute();
}

void Update::abort() { _process->abort(); }

void Update::done() { _process.reset(); }

void Update::updateProgress(double progress) {
  updateUi([this, progress]() {
    if (auto ui{this->_weakUi.lock()}) {
      (*ui)->set_update_page_progress_progress(progress);
    }
  });
}

void Update::updateStep(type::UpdateStep const step) {
  if (step == _step) return;

  switch (step) {
    case type::UpdateStep::Start:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Started"});
        }
      });
      break;
    case type::UpdateStep::Init:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Initializing"});
        }
      });
      break;
    case type::UpdateStep::Search:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Searching decoder"});
        }
      });
      break;
    case type::UpdateStep::Erase:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Erasing Flash"});
        }
      });
      break;
    case type::UpdateStep::Update:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Writing update"});
        }
      });
      break;
    case type::UpdateStep::Verify:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Verifying"});
        }
      });
      break;
    case type::UpdateStep::Cleanup:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Cleanup"});
        }
      });
      break;
    case type::UpdateStep::Done:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_update_page_progress_step_text({"Done"});
        }
      });
      break;
  }
  _step = step;
}

void Update::updateUi(std::function<void()> fn) {
  slint::invoke_from_event_loop(fn);
}

} // namespace ui
