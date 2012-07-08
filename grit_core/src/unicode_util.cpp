/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "unicode_util.h"

int encode_utf8 (unsigned long x, std::string &here)
{
        // 00ZZZzzz 00zzYYYY 00Yyyyxx 00xxxxxx
        long bytes = ((x & 0x1C0000) << 6) | ((x & 0x03F000) << 4) | ((x & 0x0FC0) << 2) | (x & 0x3F);
        if (x < 0x80) {
                here.push_back((char)x);
                return 1;
        } else if (x < 0x800) { // note that capital 'Y' bits must be 0
                bytes |= 0xC080;
                here.push_back((bytes >> 8) & 0xFF);
                here.push_back((bytes >> 0) & 0xFF);
                return 2;
        } else if (x < 0x10000) { // note that 'z' bits must be 0
                bytes |= 0xE08080;
                here.push_back((bytes >> 16) & 0xFF);
                here.push_back((bytes >>  8) & 0xFF);
                here.push_back((bytes >>  0) & 0xFF);
                return 3;
        } else if (x < 0x110000) { // note that capital 'Z' bits must be 0
                bytes |= 0xF0808080;
                here.push_back((bytes >> 24) & 0xFF);
                here.push_back((bytes >> 16) & 0xFF);
                here.push_back((bytes >>  8) & 0xFF);
                here.push_back((bytes >>  0) & 0xFF);
                return 4;
        } else {
                fprintf(stderr, "Codepoint out of range: %lu\n", x);
                exit(EXIT_FAILURE);
                return 0; // suppress msvc warning
        }
}

unsigned long decode_utf8 (const std::string &str, size_t &i)
{
        char c0 = str[i];
        if ((c0 & 0x80) == 0) { //0xxxxxxx
                return c0;
        } else if ((c0 & 0xE0) == 0xC0) { //110yyyxx 10xxxxxx
                char c1 = str[++i];
                if ((c1 & 0xC0) != 0x80) {
                        return 0x20; // error code
                }
                return ((c0 & 0x1F) << 6ul) | (c1 & 0x3F);
        } else if ((c0 & 0xF0) == 0xE0) { //1110yyyy 10yyyyxx 10xxxxxx
                char c1 = str[++i];
                if ((c1 & 0xC0) != 0x80) {
                        return 0x20; // error code
                }
                char c2 = str[++i];
                if ((c2 & 0xC0) != 0x80) {
                        return 0x20; // error code
                }
                return ((c0 & 0xF) << 12ul) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        } else if ((c0 & 0xF8) == 0xF) { //11110zzz 10zzyyyy 10yyyyxx 10xxxxxx
                char c1 = str[++i];
                if ((c1 & 0xC0) != 0x80) {
                        return 0x20; // error code
                }
                char c2 = str[++i];
                if ((c2 & 0xC0) != 0x80) {
                        return 0x20; // error code
                }
                char c3 = str[++i];
                if ((c3 & 0xC0) != 0x80) {
                        return 0x20; // error code
                }
                return ((c0 & 0x7) << 24ul) | ((c1 & 0x3F) << 12ul) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        } else {
                fprintf(stderr, "UTF-8 internal decoding error\n");
                exit(EXIT_FAILURE);
        }

}



//these specifically for win32, where wstring is UTF-16

int encode_utf16 (unsigned long x, std::wstring &here)
{
        if (x<0x10000) {
                here.push_back((wchar_t)x);
                return 1;
        } else if (x<0x110000) {
                unsigned long y = (x - 0x10000);
                here.push_back((wchar_t) ((y>>10) | 0xD800));
                here.push_back((wchar_t) ((y&0x03FF) | 0xDC00));
                return 2;
        } else {
                fprintf(stderr, "Codepoint out of range: %lu\n", x);
                exit(EXIT_FAILURE);
                return 0; // suppress msvc warning
        }
}


unsigned long decode_utf16 (const std::wstring &str, size_t &i)
{
        wchar_t first = str[i];
        if (first >= 0xdc00 && first < 0xe000) {
                return 0x20; // error char -- space
        }
        if (first >= 0xd800 && first < 0xdc00) {
                // first part of surrogate pair
                wchar_t second = str[++i];
                if (second >= 0xdc00 && second < 0xe000) {
                        // second part of surrogate pair
                        return (((first & 0x03FF) << 10ul) | (second & 0x03FF)) + 0x10000;
                } else {
                        // no pairing -- error, return space
                        return 0x20;
                }
        } else {
                // not a surrogate pair
                return first;
        }
}

std::string utf16_to_utf8 (const std::wstring &str)
{
        std::string r;
        for (size_t i=0 ; i<str.size() ; ++i) {
                encode_utf8(decode_utf16(str,i), r);
        }
        return r;
}

std::wstring utf8_to_utf16 (const std::string &str)
{
        std::wstring r;
        for (size_t i=0 ; i<str.size() ; ++i) {
                encode_utf16(decode_utf8(str,i), r);
        }
        return r;
}
