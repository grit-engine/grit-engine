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

#include <string>

/** Platform portability wrapper:  To be called at program startup. */
void clipboard_init (void);

/** Platform portability wrapper:  To be called at program shutdown. */
void clipboard_shutdown (void);

/** Platform portability wrapper:  To be called every frame. */
void clipboard_pump (void);

/** Platform portability wrapper:  Set the given text to the clipboard. */
void clipboard_set (const std::string &s);

/** Platform portability wrapper:  Set the given text to the selection buffer. */
void clipboard_selection_set (const std::string &s);

/** Platform portability wrapper:  Retrieve text from the clipboard. */
std::string clipboard_get (void);

/** Platform portability wrapper:  Retrieve text from the selection buffer. */
std::string clipboard_selection_get (void);
