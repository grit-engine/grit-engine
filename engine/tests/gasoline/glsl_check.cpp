#include <cstdlib>
#include <cerrno>
#include <cstring> // for strerror
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GLFW/glfw3.h>


void glfw_error_handler(int code, const char *msg)
{
        std::cerr << "ERROR " << code << ": " << msg << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
}
 

bool check_log(GLuint obj)
{
        int sz;
        
        if (glIsShader(obj))
                glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &sz);
        else
                glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &sz);
                        
        int log_used;
        char *log = new char[sz];
 
        if (glIsShader(obj))
                glGetShaderInfoLog(obj, sz, &log_used, log);
        else
                glGetProgramInfoLog(obj, sz, &log_used, log);
 
        if (log_used > 0)
                std::cerr << log << std::endl;

        delete [] log;

        return log_used == 0;
}

int main (int argc, char** argv)
{
        if (argc != 3) {
                std::cerr << "Usage: " << argv[0] << " <vert|frag> <filename>" << std::endl;
                return EXIT_FAILURE;
        }
        std::string typestr = argv[1];
        std::string filename = argv[2];

        GLenum type;
        if (typestr=="vert") {
                type = GL_VERTEX_SHADER;
        } else if (typestr=="frag") {
                type = GL_FRAGMENT_SHADER;
        } else {
                std::cerr<<"Unrecognised type: \""<<typestr<<"\", use vert or frag."<<std::endl;
                return EXIT_FAILURE;
        }

        if (!glfwInit()) {
                std::cerr << "Failed to initialise glfw." << std::endl;
                return EXIT_FAILURE;
        }

        glfwSetErrorCallback(glfw_error_handler);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *window = glfwCreateWindow(640, 480, "glsl_check", NULL, NULL);
        if (!window) {
                std::cerr << "Failed to create window." << std::endl;
                glfwTerminate();
                return -1;
        }
        glfwMakeContextCurrent(window);

        std::ifstream fstream;
        std::istream *stream = &fstream;

        if (filename == "-") {
                stream = &std::cin;
        } else {
                fstream.open (filename.c_str());
                if (!fstream.good()) {
                        std::cerr << "While reading: \""<<filename<<"\": "<<strerror(errno)<<std::endl;
                        glfwTerminate();
                        return EXIT_FAILURE;
                }
                if (fstream.fail() || fstream.bad()) {
                        std::cerr << filename << ": IO Error: "
                                  << strerror(errno) << std::endl;
                        glfwTerminate();
                        return EXIT_FAILURE;
                }
        }


        // Allocate
        GLuint shader = glCreateShader(type);

        std::string file;
        for (std::string line ; std::getline(*stream,line) ; ) file+=line+"\n";

        // Compile
        const char *file_[] = {file.c_str()};
        glShaderSource(shader, 1, file_, NULL);
        glCompileShader(shader);
        if (check_log(shader)) {
                std::cout << "OK!" << std::endl;
        } else {
                glfwTerminate();
                return EXIT_FAILURE;
        }

        glfwTerminate();
        return EXIT_SUCCESS;
}

// vim: ts=8:sw=8:expandtab
