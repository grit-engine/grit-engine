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
#include <cstdio>
#include <cassert>

#include <iostream>
#include <string>

extern "C" {
        #include "lua.h"
}

#include <unicode/unistr.h>
#include <unicode/regex.h>

#include "lua_utf8.h"
#include "lua_wrappers_common.h"
#include "lua_util.h"

// UTILITIES

static std::string to_utf8 (const UnicodeString &ustr) {
        struct Sink : public ByteSink {
                std::string str;
                virtual void Append (const char* bytes, int32_t n) {
                        str.append(bytes, n);
                }
        };
        Sink sink;
        ustr.toUTF8(sink);
        return sink.str;
}

static void my_lua_error (lua_State *L, const UnicodeString &str)
{ my_lua_error(L, to_utf8(str)); }

//static void my_lua_error (lua_State *L, const UnicodeString &str, unsigned long level)
//{ my_lua_error(L, to_utf8(str), level); }

static UnicodeString checkustring (lua_State *L, int index)
{
        if (lua_type(L,index) != LUA_TSTRING) {
                std::stringstream ss;
                ss << "Expected a string at index " << index;
                my_lua_error(L, ss.str());
        }
        const char *str = luaL_checkstring(L,index);
        return UnicodeString::fromUTF8(StringPiece(str));
}

static void pushustring (lua_State *L, const UnicodeString &str)
{ lua_pushstring(L, to_utf8(str).c_str()); }



// THE LUA API FUNCTIONS

static int lua_utf8_reverse (lua_State *L)
{
        check_args(L,1);
        UnicodeString ustr = checkustring(L,1);
        ustr.reverse();
        pushustring(L, ustr);
        return 1;
}
        
static int lua_utf8_len (lua_State *L)
{
        check_args(L,1);
        UnicodeString ustr = checkustring(L,1);
        lua_pushnumber(L, ustr.countChar32());
        return 1;
}
        
static int lua_utf8_mt_len (lua_State *L)
{
        check_args(L,2);
        UnicodeString ustr = checkustring(L,1);
        lua_pushnumber(L, ustr.countChar32());
        return 1;
}
        
static int lua_utf8_upper (lua_State *L)
{
        check_args(L,1);
        UnicodeString ustr = checkustring(L,1);
        ustr.toUpper();
        pushustring(L, ustr);
        return 1;
}
        
static int lua_utf8_lower (lua_State *L)
{
        check_args(L,1);
        UnicodeString ustr = checkustring(L,1);
        ustr.toLower();
        pushustring(L, ustr);
        return 1;
}
        

/*     string.sub (s, i [, j])
 * Returns the substring of s that starts at i and continues until j; i and j can
 * be negative. If j is absent, then it is assumed to be equal to -1 (which is the
 * same as the string length). In particular, the call string.sub(s,1,j) returns a
 * prefix of s with length j, and string.sub(s, -i) returns a suffix of s with
 * length i.
 */
static int lua_utf8_sub (lua_State *L)
{
        long start = 1; 
        long limit = -1;

        switch (lua_gettop(L)) {
                case 3:
                if (!lua_isnil(L,3))
                        limit = check_t<long>(L,3);
                case 2:
                if (!lua_isnil(L,2))
                        start = check_t<long>(L,2);
                break;

                default:
                check_args(L,1);
        }
        UnicodeString ustr = checkustring(L,1);

        long len = ustr.countChar32();

        if (start < 0) start += len+1;
        if (limit < 0) limit += len+1;
/*
        if (start > len) start = len+1;
        if (start < 1) start = 1;

        if (limit > len) limit = len;
        if (limit < 0) limit = 0;

        if (limit < start) limit = start-1;
*/

        start--;
        //APP_ASSERT(start>=0); assert(len==0 || start<len); assert(limit>=0); assert(limit<=len); assert(start<=limit);
        UnicodeString target;
        ustr.extractBetween(start, limit, target);
        pushustring(L, target);
        return 1;
}
        

/*     string.find (s, pattern [, init [, plain]])
 * Looks for the first match of pattern in the string s. If it finds a match, then
 * find returns the indices of s where this occurrence starts and ends; otherwise,
 * it returns nil. A third, optional numerical argument init specifies where to
 * start the search; its default value is 1 and can be negative. A value of true
 * as a fourth, optional argument plain turns off the pattern matching facilities,
 * so the function does a plain "find substring" operation, with no characters in
 * pattern being considered "magic". Note that if plain is given, then init must
 * be given as well.
 * 
 * If the pattern has captures, then in a successful match the captured values are
 * also returned, after the two indices.
 */
static int lua_utf8_find (lua_State *L)
{
        int32_t init = 1; 
        bool plain = false; 

        switch (lua_gettop(L)) {
                case 4:
                if (!lua_isnil(L,4))
                        plain = check_bool(L, 4);
                case 3:
                if (!lua_isnil(L,3))
                        init = check_t<int32_t>(L,3);
                break;

                default:
                check_args(L,2);
        }
        UnicodeString haystack = checkustring(L,1);
        UnicodeString needle = checkustring(L,2);

        long haystack_len = haystack.countChar32();
        long needle_len = needle.countChar32();

        if (init>haystack_len) init = haystack_len;
        if (init<0) init += haystack_len+1;
        if (init<1) init = 1;
        init--;

        if (plain) {
                long r = haystack.indexOf(needle, init);
                if (r==-1) {
                        lua_pushnil(L);
                        return 1;
                } else {
                        lua_pushnumber(L, r+1); // account for lua 1-based strings
                        lua_pushnumber(L, r+needle_len);
                        return 2;
                }
        } else {
                UErrorCode status = U_ZERO_ERROR;

                RegexMatcher matcher(needle, 0, status);
                if (U_FAILURE(status)) {
                        my_lua_error(L, "Syntax error in regex: \""+needle+"\": "+u_errorName(status));
                        return 0; //silence compiler
                }

                matcher.reset(haystack);

                bool found = matcher.find(init, status);
                if (U_FAILURE(status)) {
                        my_lua_error(L, u_errorName(status));
                        return 0; //silence compiler
                }
                if (found) {
                        // We found a match.
                        int start = matcher.start(status);
                        int end = matcher.end(status);
                        lua_pushnumber(L, start+1); // account for lua 1-based strings
                        lua_pushnumber(L, end);
                        long matches = matcher.groupCount();
                        for (long i=1 ; i<=matches ; ++i) {
                                UnicodeString match = matcher.group(i, status);
                                if (U_FAILURE(status)) {
                                        my_lua_error(L, u_errorName(status));
                                        return 0; //silence compiler
                                }
                                pushustring(L, match);
                        }
                        return 2+matches;
                } else {
                        lua_pushnil(L);
                        return 1;
                }
        }

}
        

static int aux_match (lua_State *L, RegexMatcher &matcher)
{
        UErrorCode status = U_ZERO_ERROR;
        if (matcher.find()) {
                long matches = matcher.groupCount();
                if (matches == 0) {
                        UnicodeString match = matcher.group(status);
                        if (U_FAILURE(status)) {
                                my_lua_error(L, u_errorName(status));
                                return 0; //silence compiler
                        }
                        pushustring(L, match);
                        return 1;
                } else {
                        for (long i=1 ; i<=matches ; ++i) {
                                UnicodeString match = matcher.group(i, status);
                                if (U_FAILURE(status)) {
                                        my_lua_error(L, u_errorName(status));
                                        return 0; //silence compiler
                                }
                                pushustring(L, match);
                        }
                        return matches;
                }
        } else {
                lua_pushnil(L);
                return 1;
        }
}

/*     string.match (s, pattern [, init])
 * Looks for the first match of pattern in the string s. If it finds one, then
 * match returns the captures from the pattern; otherwise it returns nil. If
 * pattern specifies no captures, then the whole match is returned. A third,
 * optional numerical argument init specifies where to start the search; its
 * default value is 1 and can be negative. 
 */
static int lua_utf8_match (lua_State *L)
{
        long init = 1; 

        switch (lua_gettop(L)) {
                case 3:
                if (!lua_isnil(L,3))
                        init = check_t<long>(L,3);
                break;

                default:
                check_args(L,2);
        }
        UnicodeString haystack = checkustring(L,1);
        UnicodeString needle = checkustring(L,2);

        long haystack_len = haystack.countChar32();

        if (haystack_len == 0) {
            // avoid a U_INDEX_OUTOFBOUNDS_ERROR when resetting the matcher below...
            lua_pushnil(L);
            return 1;
        }

        if (init>haystack_len) init = haystack_len;
        if (init<0) init += haystack_len+1;
        if (init<1) init = 1;
        init--;

        UErrorCode status = U_ZERO_ERROR;

        RegexMatcher matcher(needle, 0, status);
        if (U_FAILURE(status)) {
                my_lua_error(L, "Syntax error in regex: \""+needle+"\": "+u_errorName(status));
                return 0; //silence compiler
        }

        matcher.reset(haystack);
        matcher.reset(init, status);
        if (U_FAILURE(status)) {
                my_lua_error(L, u_errorName(status));
                return 0; //silence compiler
        }

        return aux_match(L, matcher);
}


#define REGEX_MATCHER_TAG "Grit/RegexMatcher"

TOSTRING_ADDR_MACRO(regex_matcher,RegexMatcher,REGEX_MATCHER_TAG)

static int regex_matcher_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(RegexMatcher,self,1,REGEX_MATCHER_TAG,0);
        if (self==NULL) return 0;
        delete &self->input();
        delete self;
        return 0;
TRY_END
}


static int regex_matcher_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(RegexMatcher,self,1,REGEX_MATCHER_TAG);
        std::string key  = check_string(L,2);
        if (key=="input") {
                pushustring(L,self.input());
        } else if (key=="pattern") {
                pushustring(L,self.pattern().pattern());
        } else {
                my_lua_error(L, "Not a readable RegexMatcher member: "+key);
        }
        return 1;
TRY_END
}

EQ_PTR_MACRO(RegexMatcher,regex_matcher,REGEX_MATCHER_TAG)

MT_MACRO(regex_matcher);


static int lua_utf8_gmatch_iter (lua_State *L)
{
        // ignore all args as they are just the previous matches
        GET_UD_MACRO(RegexMatcher,self,lua_upvalueindex(1),REGEX_MATCHER_TAG);
        return aux_match(L, self);
}
        
/*     string.gmatch (s, pattern)
 * Returns an iterator function that, each time it is called, returns the next
 * captures from pattern over string s. If pattern specifies no captures, then the
 * whole match is produced in each call.
 * 
 * As an example, the following loop
 * 
 *      s = "hello world from Lua"
 *      for w in string.gmatch(s, "%a+") do
 *        print(w)
 *      end
 * 
 * will iterate over all the words from string s, printing one per line. The next
 * example collects all pairs key=value from the given string into a table:
 * 
 *      t = {}
 *      s = "from=world, to=Lua"
 *      for k, v in string.gmatch(s, "(%w+)=(%w+)") do
 *        t[k] = v
 *      end
 * 
 * For this function, a '^' at the start of a pattern does not work as an anchor,
 * as this would prevent the iteration.
 */
static int lua_utf8_gmatch (lua_State *L)
{
        check_args(L,2);
        UnicodeString haystack = checkustring(L,1);
        UnicodeString needle = checkustring(L,2);

        UErrorCode status = U_ZERO_ERROR;

        RegexMatcher *matcher = new RegexMatcher(needle, 0, status);
        if (U_FAILURE(status)) {
                delete matcher;
                my_lua_error(L, "Syntax error in regex: \""+needle+"\": "+u_errorName(status));
                return 0; //silence compiler
        }

        matcher->reset(*new UnicodeString(haystack));

        push(L,matcher,REGEX_MATCHER_TAG);
        lua_pushcclosure(L, lua_utf8_gmatch_iter, 1);
        return 1;
}

/*     string.gsub (s, pattern, repl [, n])
 * Returns a copy of s in which all (or the first n, if given) occurrences of the
 * pattern have been replaced by a replacement string specified by repl, which can
 * be a string, a table, or a function. gsub also returns, as its second value,
 * the total number of matches that occurred.
 *
 * If repl is a string, then its value is used for replacement. The character %
 * works as an escape character: any sequence in repl of the form %n, with n
 * between 1 and 9, stands for the value of the n-th captured substring (see
 * below). The sequence %0 stands for the whole match. The sequence %% stands for
 * a single %.
 *
 * If repl is a table, then the table is queried for every match, using the first
 * capture as the key; if the pattern specifies no captures, then the whole match
 * is used as the key.
 *
 * If repl is a function, then this function is called every time a match occurs,
 * with all captured substrings passed as arguments, in order; if the pattern
 * specifies no captures, then the whole match is passed as a sole argument.
 *
 * If the value returned by the table query or by the function call is a string or
 * a number, then it is used as the replacement string; otherwise, if it is false
 * or nil, then there is no replacement (that is, the original match is kept in
 * the string).
 */
static int lua_utf8_gsub (lua_State *L)
{
        int32_t n = -1; 

        switch (lua_gettop(L)) {
                case 4:
                if (!lua_isnil(L,4))
                        n = check_t<int32_t>(L,4);
                break;

                default:
                check_args(L,3);
        }
        UnicodeString haystack = checkustring(L,1);
        UnicodeString needle = checkustring(L,2);
        UnicodeString repl;
        int mode;
        if (lua_isfunction(L,3)) {
                mode = 0;
        } else if (lua_istable(L,3)) {
                mode = 1;
        } else {
                mode = 2;
                repl = checkustring(L,3);
        }

        UErrorCode status = U_ZERO_ERROR;

        RegexMatcher matcher(needle, 0, status);
        if (U_FAILURE(status)) {
                my_lua_error(L, "Syntax error in regex: \""+needle+"\": "+u_errorName(status));
                return 0; //silence compiler
        }

        matcher.reset(haystack);

        UnicodeString r;

        int counter = 0;
        while (matcher.find() && counter++!=n) {
                long matches = matcher.groupCount();
                if (mode==0) {
                        if (matches == 0) {
                                UnicodeString match = matcher.group(status);
                                if (U_FAILURE(status)) {
                                        my_lua_error(L, u_errorName(status));
                                        return 0; //silence compiler
                                }
                                pushustring(L, match);
                                lua_pushvalue(L,3);
                                lua_call(L, 1, 1);
                        } else {
                                for (long i=1 ; i<=matches ; ++i) {
                                        UnicodeString match = matcher.group(i, status);
                                        if (U_FAILURE(status)) {
                                                my_lua_error(L, u_errorName(status));
                                                return 0; //silence compiler
                                        }
                                        pushustring(L, match);
                                }
                                lua_pushvalue(L,3);
                                lua_call(L, matches, 1);
                        }
                        repl = checkustring(L,-1);
                } else if (mode==1) {
                        if (matches == 0) {
                                UnicodeString match = matcher.group(status);
                                if (U_FAILURE(status)) {
                                        my_lua_error(L, u_errorName(status));
                                        return 0; //silence compiler
                                }
                                pushustring(L, match);
                        } else {
                                UnicodeString match = matcher.group(1, status);
                                if (U_FAILURE(status)) {
                                        my_lua_error(L, u_errorName(status));
                                        return 0; //silence compiler
                                }
                                pushustring(L, match);
                        }
                        lua_gettable(L,3);
                        repl = checkustring(L,-1);
                }
                matcher.appendReplacement(r, repl, status);
                if (U_FAILURE(status)) {
                        my_lua_error(L, u_errorName(status));
                        return 0; //silence compiler
                }
        }
        matcher.appendTail(r);

        pushustring(L, r);
        return 1;
}


// CALLED DURING INIT OF APP

void utf8_lua_init (lua_State *L)
{
        lua_getglobal(L, "string");

        lua_getfield(L, -1, "reverse"); lua_setfield(L, -2, "_reverse"); push_cfunction(L, lua_utf8_reverse); lua_setfield(L, -2, "reverse");
        lua_getfield(L, -1, "len"); lua_setfield(L, -2, "_len"); push_cfunction(L, lua_utf8_len); lua_setfield(L, -2, "len");
        lua_getfield(L, -1, "upper"); lua_setfield(L, -2, "_upper"); push_cfunction(L, lua_utf8_upper); lua_setfield(L, -2, "upper");
        lua_getfield(L, -1, "lower"); lua_setfield(L, -2, "_lower"); push_cfunction(L, lua_utf8_lower); lua_setfield(L, -2, "lower");
        lua_getfield(L, -1, "sub"); lua_setfield(L, -2, "_sub"); push_cfunction(L, lua_utf8_sub); lua_setfield(L, -2, "sub");
        lua_getfield(L, -1, "find"); lua_setfield(L, -2, "_find"); push_cfunction(L, lua_utf8_find); lua_setfield(L, -2, "find");
        lua_getfield(L, -1, "match"); lua_setfield(L, -2, "_match"); push_cfunction(L, lua_utf8_match); lua_setfield(L, -2, "match");
        lua_getfield(L, -1, "gmatch"); lua_setfield(L, -2, "_gmatch"); push_cfunction(L, lua_utf8_gmatch); lua_setfield(L, -2, "gmatch");
        lua_getfield(L, -1, "gsub"); lua_setfield(L, -2, "_gsub"); push_cfunction(L, lua_utf8_gsub); lua_setfield(L, -2, "gsub");
        lua_pop(L,1);

        lua_pushstring(L, "");
        lua_getmetatable(L,-1);
        push_cfunction(L, lua_utf8_mt_len);
        lua_setfield(L,-2,"__len");
        lua_pop(L,1);

        luaL_newmetatable(L, REGEX_MATCHER_TAG);
        luaL_register(L, NULL, regex_matcher_meta_table); 
        lua_pop(L,1);
}


/*
int main(void)
{
        srand(time(NULL));
        lua_State *L = NULL;
        place_stack("←aBc→", 1);
        lua_utf8_rev(L);
        lua_utf8_len(L);
        lua_utf8_upper(L);
        lua_utf8_lower(L);

        place_stack((random() % 6) - 1, 2);
        place_stack((random() % 6) - 1, 3);
        lua_utf8_sub(L);

        place_stack("2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm", 1);
        place_stack("₂", 2);
        place_stack(4l, 3);
        place_stack(true, 4);
        lua_utf8_find(L);

        place_stack("2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm", 1);
        place_stack("H₂O", 2);
        place_stack(3l, 3);
        place_stack(true, 4);
        lua_utf8_find(L);

        place_stack("2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm", 1);
        place_stack("([0-9.]*) (k.)", 2);
        place_stack(1l, 3);
        place_stack(false, 4);
        lua_utf8_find(L);

        place_stack("2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm", 1);
        place_stack("([0-9.]*) (k.)", 2);
        place_stack(23l, 3);
        lua_utf8_match(L);

        place_stack("2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm", 1);
        place_stack("([0-9.]+) (k.)", 2);
        place_stack("($1,$2)", 3);
        place_stack(-1l, 4);
        lua_utf8_gsub(L);

        place_stack("2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm", 1);
        place_stack("([0-9.]+)", 2);
        place_stack("(number)", 3);
        place_stack(3l, 4);
        lua_utf8_gsub(L);

        return EXIT_SUCCESS;
}
*/

// vim: shiftwidth=8:tabstop=8:expandtab

