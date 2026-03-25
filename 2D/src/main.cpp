#include <iostream>
#include <chrono>
#include <functional>
#include <algorithm>
#include <map>
#include <string>
#include <string.h>
#include <format>

#include <signal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/gl.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <GLFW/glfw3.h>

namespace car {

enum Error : int {
    E_OK = 0,
    E_ERR = 1,
};

struct Log {
    enum Type : int {
        INFO = 0,
        ERROR = 1,
    };

    const Type type;
    std::ostream &stream;

    Log(const Type &type, std::ostream &stream = std::cout):
        type(type),
        stream(stream)
    {}

    template<typename T>
    friend Log &operator<<(Log &log, const T &v) {
        log.stream << v;
        return log;
    }
};

struct Program {
    Error handle_signal(int signal);
    Error handle_error(const char *str, int errcode = 1);
    Error reset();
    Error destroy();
    Error init();
    Error hint_exit();
    Error safe_exit(int errcode = 0);
};

struct GraphicsContext {
    Error init();
    Error destroy();

    GLFWwindow *window;
    glm::ivec4 initial_window;

    GraphicsContext():
        window(nullptr),
        initial_window(0, 0, 1024, 600)
    {}
};

Log log(Log::INFO);
Log error(Log::ERROR, std::cerr);
Program program;
GraphicsContext ctx;

void handle_signal(int sig) {
    program.handle_signal(sig);
}

Error Program::init() {
    signal(SIGINT, car::handle_signal);

    return E_OK;
}

Error Program::destroy() {
    ctx.destroy();

    return E_OK;
}

Error Program::safe_exit(int errcode) {
    destroy();

    return E_OK;
}

Error Program::handle_error(const char *str, int errcode) {
    car::error << "Error: " << str << "\n";
    safe_exit(errcode);

    return E_OK;
}

Error Program::hint_exit() {
    return E_OK;
}

Error GraphicsContext::init() {
    if (!glfwInit())
        program.handle_error("Failed to initialize GLFW");
}

}


int main() {
    car::log << "Hello World!\n";
    car::log << 1;

    return 0;
}