#include <cstdlib>

#include <string>

int encode_utf8 (unsigned long x, std::string &here);
int encode_utf16 (unsigned long x, std::wstring &here);

unsigned long decode_utf8 (const std::string &str, size_t &i);
unsigned long decode_utf16 (const std::wstring &str, size_t &i);

std::string utf16_to_utf8 (const std::wstring &str);
std::wstring utf8_to_utf16 (const std::string &str);