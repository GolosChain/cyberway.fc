#pragma once
#include <string>
#include <vector>

namespace fc {
std::string base64_encode(const std::vector<char>& bytes_to_encodes);
std::string base64_decode( const std::string& encoded_string);
}  // namespace fc
