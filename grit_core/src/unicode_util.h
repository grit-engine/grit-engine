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

#include <string>

/** Convert the given unicode codepoint x to utf8 form, using the given string
 * as a recepticle. */
int encode_utf8 (unsigned long x, std::string &here);
/** Convert the given unicode codepoint x to utf16 form, using the given
 * std::wstring as a recepticle.  This function is specifically for Windows,
 * where std::wstring is 16 bit. */
int encode_utf16 (unsigned long x, std::wstring &here);

/** Convert the utf8 byte sequence in the given string to a unicode code point.
 * \param str The string.
 * \param i The index of the string from which to start
 *          decoding, and returns the index after the last byte of the encoded
 *          codepoint.
 * \returns The decoded unicode codepoint.
 */
unsigned long decode_utf8 (const std::string &str, size_t &i);

/** Convert the utf16 byte sequence in the given wstring to a unicode
 * codepoint.  This function is specifically for Windows, where std::wstring is 16 bit.
 * \param str The string.
 * \param i The index of the string from which to start decoding, and
 *          returns the index after the last byte of the encoded codepoint.
 * \returns The decoded unicode codepoint.
 */
unsigned long decode_utf16 (const std::wstring &str, size_t &i);

/** Convert strings of multiple unicode codepoints between utf16 and utf8. */
std::string utf16_to_utf8 (const std::wstring &str);

/** Convert strings of multiple unicode codepoints between utf16 and utf8. */
std::wstring utf8_to_utf16 (const std::string &str);
