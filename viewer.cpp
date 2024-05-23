#include <array>
#include <cstdio>
#include <print>
#include <stdexcept>
#include <utility>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

class Window {
 public:
  explicit Window(const char* name, int width = 800, int height = 600);

  template <typename Func, typename... Args>
  void show(Func func, Args&&... args) {
    while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      func(std::forward<Args>(args)...);
      glfwPollEvents();
      glfwSwapBuffers(window_);
    }
  }

  void show() {
    show([]() {});
  }

  Window(const Window&) = delete;
  Window(Window&&) = delete;

  Window& operator=(const Window&) = delete;
  Window& operator=(Window&&) = delete;

  ~Window() {
    if (window_) glfwDestroyWindow(window_);

    glfwTerminate();
  }

  template <typename T>
  auto framebuffer_size() const {
    int width{};
    int height{};
    glfwGetFramebufferSize(window_, &width, &height);
    return std::make_pair<T, T>(width, height);
  }

 private:
  friend void glfw_key_callback(GLFWwindow* window, int key, int scancode,
                                int action, int mods);

  GLFWwindow* window_{nullptr};

  int polygon_draw_mode_{GL_FILL};
};

void glfw_error_callback(int error, const char* description) {
  printf("glfw error %d: %s\n", error, description);
}

void glfw_close_callback(GLFWwindow* window) {
  printf("window closed\n");
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action,
                       int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    return glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
    w->polygon_draw_mode_ = GL_POINT + (w->polygon_draw_mode_ + 1) % 3;
    glPolygonMode(GL_FRONT_AND_BACK, w->polygon_draw_mode_);
  }
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void opengl_debug_callback(GLenum source, GLenum type, GLuint id,
                           GLenum severity, GLsizei length, const char* message,
                           const void* userParam) {
  printf("opengl debug:(%d) %s\n, ", id, message);
  switch (source) {
    case GL_DEBUG_SOURCE_API:
      printf("source: API, ");
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      printf("source: Window System, ");
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      printf("source: Shader Compiler, ");
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      printf("source: Third Party, ");
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      printf("source: Application, ");
      break;
    case GL_DEBUG_SOURCE_OTHER:
      printf("source: Other, ");
  }

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      printf("type: Error, ");
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      printf("type: Deprecated Behaviour, ");
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      printf("type: Undefined Behaviour, ");
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      printf("type: Portability, ");
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      printf("type: Performance, ");
      break;
    case GL_DEBUG_TYPE_MARKER:
      printf("type: Marker, ");
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      printf("type: Push Group, ");
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      printf("type: Pop Group, ");
      break;
    case GL_DEBUG_TYPE_OTHER:
      printf("type: Other, ");
      break;
  }

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      printf("severity: high");
      throw std::runtime_error("opengl error");
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      printf("severity: medium");
      break;
    case GL_DEBUG_SEVERITY_LOW:
      printf("severity: low");
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      printf("severity: notification");
      break;
  }
}

Window::Window(const char* name, int width, int height) {
  if (glfwInit() == GLFW_FALSE) {
    printf("failed to initialize GLFW\n");
    throw std::runtime_error("failed to initialize GLFW");
  }
  glfwSetErrorCallback(glfw_error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  window_ = glfwCreateWindow(width, height, name, nullptr, nullptr);
  if (window_ == nullptr) {
    printf("failed to create GLFW window\n");
    throw std::runtime_error("failed to create GLFW window");
  }
  glfwSetWindowCloseCallback(window_, glfw_close_callback);
  glfwSetKeyCallback(window_, glfw_key_callback);
  glfwSetFramebufferSizeCallback(window_, glfw_framebuffer_size_callback);

  glfwMakeContextCurrent(window_);

  glfwSetWindowUserPointer(window_, this);

  if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
    printf("failed to initialize GLAD\n");
    throw std::runtime_error("failed to initialize GLAD");
  }
  printf("OpenGL: %d.%d\n", GLVersion.major, GLVersion.minor);
  printf("GLFW: %s\n", glfwGetVersionString());

  int flags{};
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    printf("opengl debug message enabled\n");
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
  }

  glfwSwapInterval(1);
  glViewport(0, 0, width, height);
  glClearColor(0.97, 0.88, 0.71, 1.0);
  glPointSize(4);
  glEnable(GL_DEPTH_TEST);
}

struct Shader {
  Shader(const char* const vertex_shader_source,
         const char* const fragment_shader_source);
  Shader(const Shader&) = delete;
  Shader(Shader&&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&&) = delete;

  ~Shader() { glDeleteProgram(shader_program_); }

  void use() const { glUseProgram(shader_program_); }

  template <typename T>
  void set_uniform(const char* name, T v0, T v1, T v2, T v3) const;

  template <typename T>
  void set_uniform(const char* name, const T& value) const;

 private:
  GLuint shader_program_{};
};

Shader::Shader(const char* const vertex_shader_source,
               const char* const fragment_shader_source) {
  GLuint vertex_shader{};
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
  glCompileShader(vertex_shader);
  int success{};
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::array<char, 512> info_log{};
    glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log.data());
    printf("compile vertex shader failed: %s\n", info_log.data());
  }

  GLuint fragment_shader{};
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::array<char, 512> info_log{};
    glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log.data());
    printf("compile fragment shader failed: %s\n", info_log.data());
  }

  shader_program_ = glCreateProgram();
  glAttachShader(shader_program_, vertex_shader);
  glAttachShader(shader_program_, fragment_shader);
  glLinkProgram(shader_program_);
  glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
  if (!success) {
    std::array<char, 512> info_log{};
    glGetProgramInfoLog(shader_program_, 512, nullptr, info_log.data());
    printf("link shader program failed: %s\n", info_log.data());
  }
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glUseProgram(shader_program_);
}

template <>
void Shader::set_uniform(const char* name, float v0, float v1, float v2,
                         float v3) const {
  glUniform4f(glGetUniformLocation(shader_program_, name), v0, v1, v2, v3);
}

template <>
void Shader::set_uniform(const char* name, const int& value) const {
  glUniform1i(glGetUniformLocation(shader_program_, name), value);
}

template <>
void Shader::set_uniform(const char* name, const float& value) const {
  glUniform1f(glGetUniformLocation(shader_program_, name), value);
}

// template <>
// void Shader::set_uniform<>(const char* name, const glm::mat4& value) const {
//   glUniformMatrix4fv(glGetUniformLocation(shader_program_, name), 1,
//   GL_FALSE,
//                      glm::value_ptr(value));
// }

// template <>
// void Shader::set_uniform<>(const char* name, const glm::vec3& value) const {
//   glUniform3fv(glGetUniformLocation(shader_program_, name), 1,
//                glm::value_ptr(value));
// }

constexpr auto* vertex_shader_source = R"(
#version 460 core

in vec4 p;
in vec2 t;
out vec2 u;

void main() {
  gl_Position = p;
  u = t;
}
)";

constexpr auto* fragment_shader_source = R"(
#version 460 core

uniform sampler2D c;
in vec2 u;
out vec4 f;

void main() {
  f = texture(c,u);
}
)";

auto main(int argc, char** argv) -> int {
  Window window("window");
  Shader shader(vertex_shader_source, fragment_shader_source);
  window.show();
  return 0;
}