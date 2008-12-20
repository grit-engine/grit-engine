#include <string>

void app_verbose(const char *file, int line, const std::string& msg);
void app_error(char const *file, int line,
        const std::string& i_was, const std::string& err);
void app_line(const std::string& msg);
void app_fatal();

#define APP_ERROR(i_was,err) app_error(__FILE__,__LINE__,i_was,err)
#define APP_VERBOSE(msg) app_verbose(__FILE__,__LINE__,msg)
#define APP_ASSERT(cond) do { \
        if (!(cond)) { \
                APP_ERROR("assertion failed", #cond);\
                app_fatal(); \
        } \
} while (0)

#define APP_PERROR_ASSERT(cond) do { \
        if (!(cond)) {\
                APP_ERROR("assertion failed", #cond);\
                perror("perror says"); \
                app_fatal(); \
        } \
} while (0)


// vim: shiftwidth=8:tabstop=8:expandtab
