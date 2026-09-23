#pragma once

#include <curl/curl.h>
#include <filesystem>
#include <string>
#include <vector>

namespace ui::helper {

namespace detail {

constexpr std::string_view dsw_version_url{
  "https://www.zimo.at/update/DSW_Version.txt"};

constexpr std::string_view dsw_version_path{"./.cache/dsw_version.txt"};
constexpr std::string_view zsu_file_path{"./.cache/ms.zsu"};

namespace dsw {

struct DSW {
  enum class Group {
    MX,
    MS_MN_FS,
  } group;
  std::string version{};
  std::string date{};
  std::string url{};
};

struct File {
  std::vector<DSW> dsws{};
};

File read(std::filesystem::path path);

} // namespace dsw

namespace curl {

struct Easy {
  Easy();
  ~Easy();

  void fetch(std::filesystem::path path, std::string_view url);

private:
  static size_t write_fn(void* ptr, size_t size, size_t nmemb, FILE* stream);

  CURL* curl{};
};

} // namespace curl

} // namespace detail

struct FirmwareFetcher {
  FirmwareFetcher();
  ~FirmwareFetcher();

  std::filesystem::path fetchLatest();

private:
};

} // namespace ui::helper
