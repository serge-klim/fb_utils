#pragma once
#include <iosfwd>
#include <vector>
#include <string>
#include <utility>
#include <filesystem>

namespace utils { inline namespace v1 { namespace ini {

using pairs = std::vector<std::pair<std::string, std::string>>;
pairs parse(std::istream& in, std::filesystem::path const& search_path = std::filesystem::path{});
pairs parse(std::filesystem::path const& filename);

namespace detail {
std::vector<std::pair<char, std::string_view>> tokenize(std::string_view input);
} // namespace detail

}}} // namespace utils::v1::ini
