#include "viewer.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <print>
#include <stdexcept>
#include <vector>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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
// 32-bit surface container

Surface::Surface(int w, int h)
    : width(w), height(h), pixels(width * height, 0) {}
void Surface::Clear(uint32_t c) { std::fill(pixels.begin(), pixels.end(), c); }
void Surface::Plot(int x, int y, uint32_t c) { pixels[y * width + x] = c; }

// class Texture {};

Texture::Texture(int w, int h, int slot) : width_(w), height_(h), slot_(slot) {
  glGenTextures(1, &texture_);
  glActiveTexture(GL_TEXTURE0 + slot_);
  glBindTexture(GL_TEXTURE_2D, texture_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
}

void Texture::upload(const Surface& s) {
  glBindTexture(GL_TEXTURE_2D, texture_);
  glActiveTexture(GL_TEXTURE0 + slot_);
  // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA,
  //                 GL_UNSIGNED_BYTE, s.pixels.data());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, s.pixels.data());
}

void Texture::download(Surface& s) {
  glBindTexture(GL_TEXTURE_2D, texture_);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, s.pixels.data());
}
