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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace common {

enum Error : int {
    E_OK = 0,
    E_ERR = 1,
};

struct Log {
    enum Type : int {
        INFO = 0,
        ERROR = 1,
        DEBUG = 2,
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

Log log(Log::INFO);
Log debug(Log::DEBUG);
Log error(Log::ERROR, std::cerr);

}

namespace gfx {

using namespace common;

const GLuint gfx_null = -1;

struct Texture {
    GLuint texture_id;

    Texture():
        texture_id(gfx_null)
    {}

    bool is_loaded() { return texture_id != gfx_null; }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }

    void use(const int &unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
    }

    void use() {
        bind();
    }

    Error load(const char *path) {
        int width, height, channels;

        unsigned char* image = stbi_load(path, &width, &height, &channels, 0);

        if (!image) {
            common::error << "Error opening image file: " << path << "\n";
            return E_ERR;
        }

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
        stbi_image_free(image);
    
        return E_OK;
    }
};

}

namespace car {

using namespace common;

struct Program {
    Error handle_error(const char *str, int errcode = 1);
    Error reset();
    Error destroy();
    Error init();
    Error hint_exit();
    Error safe_exit(int errcode = 0);

    static void handle_signal(int signal);
};

struct GraphicsContext {
    Error init();
    Error destroy();

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

    GLFWwindow *window;
    glm::ivec4 initial_window, current_window;

    GraphicsContext():
        window(nullptr),
        initial_window(0, 0, 1024, 600)
    {}
};

Program program;
GraphicsContext ctx;

Error Program::init() {
    signal(SIGINT, Program::handle_signal);

    ctx.init();

    return E_OK;
}

Error Program::destroy() {
    ctx.destroy();

    return E_OK;
}

Error Program::safe_exit(int errcode) {
    destroy();

    car::debug << "safe_exit: " << errcode << "\n";

    return E_OK;
}

Error Program::handle_error(const char *str, int errcode) {
    car::error << "Error: " << str << "\n";
    safe_exit(errcode);

    return E_OK;
}

void Program::handle_signal(int signal) {
    car::error << "Signal: " << signal << "\n";
    program.hint_exit();
}

Error Program::hint_exit() {
    glfwSetWindowShouldClose(ctx.window, 1);
    
    car::debug << "hint_exit\n";

    return E_OK;
}

void GraphicsContext::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    ctx.current_window.z = width;
    ctx.current_window.w = height;
    glViewport(0, 0, width, height);
}

Error GraphicsContext::init() {
    if (!glfwInit())
        return program.handle_error("Failed to initialize GLFW");

    window = glfwCreateWindow(initial_window.z, initial_window.w, "Car Sim", nullptr, nullptr);

    if (!window)
        return program.handle_error("Failed to create window");

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, GraphicsContext::framebuffer_size_callback);

    glfwSwapInterval(1);

    glfwGetWindowPos(window, &initial_window.x, &initial_window.y);
    glfwGetWindowSize(window, &initial_window.z, &initial_window.w);

    current_window = initial_window;

    return E_OK;
}

Error GraphicsContext::destroy() {
    glfwTerminate();

    return E_OK;
}

}


int main() {
    using namespace car;

    program.init();

    while (!glfwWindowShouldClose(ctx.window)) {
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(ctx.window);
        glfwPollEvents();
    }

    program.safe_exit(0);

    return 0;
}