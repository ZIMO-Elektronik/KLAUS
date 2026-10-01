/**
 * Copyright (C) 2026 ZIMO Elektronik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://gnu.org>.
 *
 *
 *
 *
 *
 * Firmware fetch helper
 *
 * \file    ui/helper/firmware_fetcher.hpp
 * \author  Jonas Gahlert
 * \date    24.09.2026
 */

#pragma once

#include <curl/curl.h>
#include <filesystem>
#include <string>
#include <vector>

namespace ui::helper {

namespace detail {

/**
 * Cache dir
 *
 * \return cache dir path
 *
 * \todo
 * Maybe we should add a fallback in case this is not writeable...
 */
constexpr std::filesystem::path cache_dir() {
  return std::filesystem::temp_directory_path() / "z-klaus";
}

/**
 * Appends the dsw version file name to the given path
 *
 * \param dir directory (including ending slash)
 *
 * \return dir path with file name appended
 */
constexpr std::filesystem::path dsw_version_path(std::filesystem::path dir) {
  return dir / "dsw_version.txt";
}

/**
 * Appends the ms decoder zsu file name to the given path
 *
 * \param dir directory (including ending slash)
 *
 * \return dir path with file name appended
 */
constexpr std::filesystem::path ms_zsu_file_path(std::filesystem::path dir) {
  return dir / "ms.zsu";
}

/**
 * Appends the mx decoder zsu file name to the given path
 *
 * \param dir directory (including ending slash)
 *
 * \return dir path with file name appended
 */
constexpr std::filesystem::path& mx_zsu_file_path(std::filesystem::path& dir) {
  return dir.append("mx.zsu");
}

/**
 * Remote URL of the dsw version file
 */
constexpr std::string_view dsw_version_url{
  "https://www.zimo.at/update/DSW_Version.txt"};

namespace dsw {

/**
 * DSW info struct
 *
 * \details
 * Represents the info about a single (remote) firmware file
 *
 */
struct DSW {
  enum class Group {
    MX,
    MS_MN_FS,
  } group;               ///< Decoder firmware group
  std::string version{}; /// Firmware version
  std::string date{};    ///< Release date
  std::string url{};     ///< Remote url
};

/**
 * DSW file struct
 *
 * \details
 * Represents the entire dsw version file
 */
struct File {
  std::vector<DSW> dsws{};
};

/**
 * Parses the given dsw version file
 *
 * \param path path to file
 *
 * \return File
 */
File read(std::filesystem::path path);

} // namespace dsw

namespace curl {

/**
 * RAII wrapper for the curl easy transfer handle
 *
 */
struct Easy {
  Easy();
  ~Easy();

  void fetch(std::filesystem::path path, std::string_view url);

private:
  static size_t write_fn(void* ptr, size_t size, size_t nmemb, FILE* stream);

  CURL* curl{}; ///< Underlying handle
};

} // namespace curl

} // namespace detail

/**
 * Firmware fetcher class
 *
 * \details
 * Static class to fetch the latest decoder firmware
 */
struct FirmwareFetcher {
  static std::filesystem::path fetchLatest();
};

} // namespace ui::helper
