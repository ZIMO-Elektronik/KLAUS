#include "firmware_fetcher.hpp"
#include <algorithm>
#include <ctre.hpp>
#include <format>
#include <fstream>
#include <stdexcept>

namespace ui::helper {

namespace detail {

namespace dsw {

/**
 * Class start pattern, groups to <class>
 */
constexpr auto class_start_pattern{
  ctll::fixed_string{"^\"(?!/)(?!.*\",\".*)(.*)\"$"}};

/**
 * Class end pattern, groups to <class>
 */
constexpr auto class_end_pattern{
  ctll::fixed_string{"^\"/(?!.*\",\".*)(.*)\"$"}};

/**
 * Class field pattern, groups to <key>, <value>
 */
constexpr auto field_pattern{ctll::fixed_string{"^\"(.*)\",\"(.*)\""}};

File read(std::filesystem::path path) {
  using namespace ctre::literals;
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
    std::string_view vline{line};
    if (auto [whole_1, _class] = ctre::match<class_start_pattern>(line);
        whole_1) {
      // Start of a class
      if (group) throw std::format_error("Class begin inside of other class");

      if (_class == "Decodersw"sv) group = DSW::Group::MX;
      else if (_class == "DecoderswMS"sv) group = DSW::Group::MS_MN_FS;
      else throw std::format_error("Unsupported class");

      version = "";
      date = "";
      url = "";

    } else if (auto [whole_2, key, value] = ctre::match<field_pattern>(line);
               whole_2) {
      // Field of a class
      if (!group) throw std::format_error("Field defined outside of class");

      if (key == "Version"sv) {
        version = value;

      } else if (key == "Datum"sv) {
        date = value;

      } else if (key == "Location"sv) {
        url.insert(0uz, value);

      } else if (key == "Datei"sv) {
        url.append(value);

      } else throw std::format_error("Unsupported key");

    } else if (auto [whole_3, class_] = ctre::match<class_end_pattern>(line);
               whole_3) {
      // End of a class
      if (!group) throw std::format_error("Class terminator outside of class");

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

    } else throw std::format_error("Unrecognized line format");
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
