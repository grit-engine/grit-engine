#include <string>
#include "lua_util.h"

/** A stack of directories.  The top of the stack is the current working directory */
extern std::vector<std::string> pwd;

/** Pushes the given directory onto the stack, thereby setting the current working directory.
@remarks
The directory string must begin and end with a '/'.
@param dir The new directory
*/      
void pwd_push_dir (const std::string &dir);

/** Pushes the directory in which the given file resides.
@remarks
The filename must begin with a '/'.
@param filename The full path to the file
*/      
void pwd_push_file (const std::string &filename);

/** Pops and returns the current directory */
std::string pwd_pop (void);

/** Peeks the top of the directory stack */
std::string pwd_get (void);

/** Translates a relative path to an absolute path.
@remarks
If given an absolute path, simply returns it.
@param L The lua state used to return errors e.g. too many ..
@param rel The relative path or a fully qualified path
@param path The current working directory
*/
std::string pwd_full_ex (lua_State *L, std::string rel, const std::string &path);

/** Version of pwd_full_ex that does not need a lua state.  Errors are reported via CERR and the
provided string is substituted.
@remarks
If given an absolute path, simply returns it.
@param rel The relative path or a fully qualified path
@param path The current working directory
@param def The string that is used in case of error
*/
std::string pwd_full_ex (std::string rel, const std::string &path, const std::string &def);

/** Translates a relative path to an absolute path using the top of the path stack.
*/
std::string pwd_full (lua_State *L, std::string rel);

