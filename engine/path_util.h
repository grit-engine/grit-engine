/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include <vector>
#include <string>

/** Translates a relative path to an absolute path.
@remarks
If given an absolute path, simply returns it.
@param L The lua state used to return errors e.g. too many ..
@param rel The relative path or a fully qualified path
@param path The current working directory
*/
std::string pwd_full_ex (std::string rel, const std::string &path);

/** Version of pwd_full_ex that does not need a lua state.  Errors are reported via CERR and the
provided string is substituted.
@remarks
If given an absolute path, simply returns it.
@param rel The relative path or a fully qualified path
@param path The current working directory
@param def The string that is used in case of error
*/
std::string pwd_full_ex (std::string rel, const std::string &path, const std::string &def);

std::string grit_dirname (const std::string &p);
