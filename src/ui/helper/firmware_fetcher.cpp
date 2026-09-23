#include "firmware_fetcher.hpp"
#include <algorithm>
#include <format>
#include <fstream>
#include <regex>
#include <stdexcept>

namespace ui::helper {

namespace detail {

namespace dsw {

/**
 * Class start pattern, groups to <class>
 */
std::regex class_start_pattern{"^\"(?!/)(?!.*\",\".*)([^\"]*)\"$"};

/**
 * Class end pattern, groups to <class>
 */
std::regex class_end_pattern{"^\"/(?!.*\",\".*)([^\"]*)\"$"};

/**
 * Class field pattern, groups to <key>, <value>
 */
std::regex field_pattern{"^\"([^\"]*)\",\"([^\"]*)\""};

File read(std::filesystem::path path) {
  using std::operator""sv;

  if (!exists(path))
    throw std::filesystem::filesystem_error(
      "DSW file does not exist",
      path,
      std::make_error_code(std::errc::file_exists));

  std::ifstream fs{path};
  std::istream& input{fs};

  std::optional<DSW::Group> group{};
  std::string version{};
  std::string date{};
  std::string url{};

  File output{};

  std::string line{};
  while (std::getline(input, line)) {
    if (!line.empty() && line.back() == '\r') { line.pop_back(); }

    std::smatch words{};
    std::regex_search(line, words, class_start_pattern);
    if (!words.empty() && words.size() == 2uz) {
      // Start of a class
      if (group) throw std::format_error("Class begin inside of other class");

      auto iter{++words.begin()};         // Skip whole match
      auto const _class{(iter++)->str()}; // First submatch

      if (_class == "Decodersw"sv) group = DSW::Group::MX;
      else if (_class == "DecoderswMS"sv) group = DSW::Group::MS_MN_FS;
      else throw std::format_error("Unsupported class");

      version = "";
      date = "";
      url = "";

      continue;
    }

    std::regex_search(line, words, field_pattern);
    if (!words.empty() && words.size() == 3uz) {
      // Field of a class
      if (!group) throw std::format_error("Field defined outside of class");

      auto iter{++words.begin()};      // Skip whole match
      auto const key{(iter++)->str()}; // First submatch
      auto value{(iter++)->str()};     // Second submatch

      if (key == "Version"sv) {
        version = value;

      } else if (key == "Datum"sv) {
        date = value;

      } else if (key == "Location"sv) {
        if (!value.starts_with("https")) {
          value.insert(value.find_first_of(':'), 1u, 's');
        }
        url.insert(0uz, value);

      } else if (key == "Datei"sv) {
        url.append(value);

      } else throw std::format_error("Unsupported key");

      continue;
    }

    std::regex_search(line, words, class_end_pattern);
    if (!words.empty() && words.size() == 2uz) {
      // End of a class
      if (!group) throw std::format_error("Class terminator outside of class");

      auto iter{++words.begin()};         // Skip whole match
      auto const class_{(iter++)->str()}; // First submatch

      switch (*group) {
        case DSW::Group::MX:
          if (class_ != "Decodersw"sv)
            throw std::format_error(
              "Class terminator does not match current class");
          break;

        case DSW::Group::MS_MN_FS:
          if (class_ != "DecoderswMS"sv)
            throw std::format_error(
              "Class terminator does not match current class");
          break;

        default: throw std::format_error("Unsupported class terminator");
      }

      output.dsws.push_back(DSW{.group = *group,
                                .version = std::move(version),
                                .date = std::move(date),
                                .url = std::move(url)});
      group.reset();

      continue;
    }

    throw std::format_error("Unrecognized line format");
  }

  return output;
}
} // namespace dsw

namespace curl {

Easy::Easy() { curl = curl_easy_init(); }

Easy::~Easy() {
  curl_easy_cleanup(curl);
  curl = nullptr;
}

void Easy::fetch(std::filesystem::path path, std::string_view url) {
  auto fp = fopen(path.string().data(), "wb");
  curl_easy_setopt(curl, CURLOPT_URL, url.data());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  auto const res = curl_easy_perform(curl);
  fclose(fp);
}

size_t Easy::write_fn(void* ptr, size_t size, size_t nmemb, FILE* stream) {
  return fwrite(ptr, size, nmemb, stream);
}

} // namespace curl

} // namespace detail

std::filesystem::path FirmwareFetcher::fetchLatest() {
  detail::curl::Easy easy{};

  std::filesystem::create_directory("./.cache");

  // Fetch version info
  easy.fetch(detail::dsw_version_path, detail::dsw_version_url);

  // Parse version info
  auto const file{detail::dsw::read(detail::dsw_version_path)};
  auto const dsw{std::ranges::find_if(file.dsws, [](auto const& dsw) {
    return dsw.group == detail::dsw::DSW::Group::MS_MN_FS;
  })};

  // Fetch zsu file
  easy.fetch(detail::zsu_file_path, dsw->url.data());

  return detail::zsu_file_path;
}

} // namespace ui::helper
