#pragma once

#include <cstdint>
#include <print>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

struct Window {
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

  friend void glfw_key_callback(GLFWwindow* window, int key, int scancode,
                                int action, int mods);

  GLFWwindow* window_{nullptr};

  int polygon_draw_mode_{GL_FILL};
};

void glfw_error_callback(int error, const char* description);
void glfw_close_callback(GLFWwindow* window);
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action,
                       int mods);
void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);

void opengl_debug_callback(GLenum source, GLenum type, GLuint id,
                           GLenum severity, GLsizei length, const char* message,
                           const void* userParam);

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

template <>
void inline Shader::set_uniform(const char* name, float v0, float v1, float v2,
                                float v3) const {
  glUniform4f(glGetUniformLocation(shader_program_, name), v0, v1, v2, v3);
}

template <>
void inline Shader::set_uniform(const char* name, const int& value) const {
  glUniform1i(glGetUniformLocation(shader_program_, name), value);
}

template <>
void inline Shader::set_uniform(const char* name, const float& value) const {
  glUniform1f(glGetUniformLocation(shader_program_, name), value);
}

// 32-bit surface container
class Surface {
  enum { OWNER = 1 };

 public:
  Surface(int w, int h);
  ~Surface() = default;

  void Clear(uint32_t c);
  void Plot(int x, int y, uint32_t c);

  void InitCharset();
  void SetChar(int c, const char* c1, const char* c2, const char* c3,
               const char* c4, const char* c5);
  void Print(const char* t, int x1, int y1, uint32_t c);
  void Line(float x1, float y1, float x2, float y2, uint32_t c);
  void LoadImage(const char* file);
  void CopyTo(Surface* dst, int x, int y);
  void Box(int x1, int y1, int x2, int y2, uint32_t color);
  void Bar(int x1, int y1, int x2, int y2, uint32_t color);

  int width = 0, height = 0;
  std::vector<uint32_t> pixels;  // rgba
};

// class Texture {};
struct Texture {
  explicit Texture(int width, int height, int slot = 0);
  Texture(const Texture&) = delete;
  Texture(Texture&&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture& operator=(Texture&&) = delete;

  ~Texture() { glDeleteTextures(1, &texture_); }

  void bind() const {
    glActiveTexture(GL_TEXTURE0 + slot_);
    glBindTexture(GL_TEXTURE_2D, texture_);
  }

  void upload(const Surface&);
  void download(Surface&);

  explicit operator int() const { return slot_; }

 private:
  int32_t width_{};
  int32_t height_{};
  GLuint texture_{};
  int slot_{};
};

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
// f = vec4(0, 0, 1, 0);
}
)";

template <typename Func>
void run(const char* name, int width, int height, Func&& func) {
  Window window(name, width, height);

  GLuint vao{};
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // clang-format off
  float verts[18] = {-0.9, 0.9, 0,
                      0.9, 0.9, 0,
                     -0.9,-0.9, 0,
                      0.9, 0.9, 0,
                     -0.9,-0.9, 0,
                      0.9,-0.9, 0};
  // clang-format on
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  float uvdata[] = {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1};
  GLuint uvBuffer;
  glGenBuffers(1, &uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvdata), uvdata, GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  Texture texture{width, height};
  Shader shader{vertex_shader_source, fragment_shader_source};
  shader.set_uniform("c", static_cast<int>(texture));

  Surface canvas(width, height);

  while (glfwWindowShouldClose(window.window_) == GLFW_FALSE) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    canvas.Clear(0xffffff);
    func(canvas);
    texture.upload(canvas);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwPollEvents();
    glfwSwapBuffers(window.window_);
  }
}
