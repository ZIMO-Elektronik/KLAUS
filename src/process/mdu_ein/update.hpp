/**
 * MDU_EIN Update process
 *
 * \file    include/process/mdu_ein/update.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <functional>
#include <ulf/cpp/libulf.hpp>
#include <vector>
#include "process/base.hpp"
#include "type/step.hpp"

namespace process::mdu_ein {

/**
 * SoundLoad process
 *
 * \details
 * This class handles the MDU Update as a background process. Each stage of
 * the process is represented by a pair of *Action and *Result methods. Each
 * Action sets its corresponding Result handler, where the Resulthandler is
 * called from the LibULF callback after each transfer.
 *
 * The process can be started using \ref Update::execute and aborted using
 * \ref Update::abort, which will quit the event loop AFTER the next result
 * is available. To change this behaviour, the LibULF interface needs to be
 * extended with an abort function.
 *
 * \note
 * The process itself is designed as an eventloop. The Action starts a transfer,
 * Result handles the result once it is available and either calls the next
 * Action or exits.
 *
 */
struct Update : public Base {
  Update(std::filesystem::path path,
         type::MDUEntryType entry_type,
         std::vector<uint32_t> decoder_ids = {});
  Update(std::shared_ptr<libulf::ZSU> zsu,
         type::MDUEntryType entry_type,
         std::vector<uint32_t> decoder_ids = {});
  virtual ~Update() final;

  virtual bool execute();

private:
  void update();

  void mode();
  void enter();
  void config();
  void search();
  void init();
  void erase();
  void write();
  void verify();
  void end();

  void lifeSign();

  void checkAbort();

  std::shared_ptr<libulf::ZSU> _zsu;                  ///< ZSU instance
  libulf::ZSU::FirmwareIterator _fwIt{_zsu->begin()}; ///< Firmware iterator

  type::MDUEntryType _entryType; ///< Entry type (MDU or DCC)

  std::vector<uint32_t> _decoderIDs; ///< List of selected IDs
  decltype(_decoderIDs)::iterator _iter{_decoderIDs.begin()}; ///< Current ID
  decltype(_decoderIDs)::const_iterator _lastIter{_decoderIDs.end() -
                                                  1}; ///< Last ID

  int _err_cnt{0}; ///< Consecutive error counter
};

} // namespace process::mdu_ein
