#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define ENABLE_FRAMEBUFFER_SRGB
//#define DISABLE_SRGB_BEFORE_DEPTH_BLIT
//#define USE_SRGB_FBO
//#define SEPARATE_COLOR_AND_DEPTH_BLIT

const glm::ivec2 res = glm::ivec2(800, 600);
GLFWwindow *window;

const char* vertexShader = R"(#version 330 core
layout(location = 0) in vec2 position;
uniform mat4 modelViewProjection;
void main() {
    gl_Position = modelViewProjection * vec4(position, 0.0, 1.0);
}
)";

const char* fragmentShader = R"(#version 330 core
out vec4 fragColor;
uniform vec3 color;
void main() {
    fragColor = vec4(color, 1.0);
}
)";

void compileShader(GLuint program, GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&src, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
}

GLuint texture(GLenum internalFormat, GLenum format) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, res.x, res.y, 0, format, GL_FLOAT, nullptr); // TODO check
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return tex;
}

int main(int argc, char** argv) {
    if(!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if(!(window = glfwCreateWindow(res.x, res.y, "Depth Blit Issues", NULL, NULL))) {
        glfwTerminate();
        std::cout << "Failed to create window" << std::endl;
        return 1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return 1;
    }

    auto viewProjection = glm::perspective(glm::radians(45.0f), static_cast<float>(res.x) / res.y, 0.1f, 100.0f) *
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
    auto redModel = glm::mat4(1.0f);
    auto greenModel = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)), -glm::half_pi<float>()*0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

    GLuint prog = glCreateProgram();
    compileShader(prog, GL_VERTEX_SHADER, vertexShader);
    compileShader(prog, GL_FRAGMENT_SHADER, fragmentShader);
    glLinkProgram(prog);
    glUseProgram(prog);
    GLint colorLoc = glGetUniformLocation(prog, "color");
    GLint modelViewProjectionLoc = glGetUniformLocation(prog, "modelViewProjection");

    float vertices[8] = {-0.5f, -0.5f,  0.5f, -0.5f,  -0.5f, 0.5f,  0.5f, 0.5f};
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(float), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    #ifdef USE_SRGB_FBO
        GLenum colorTexInternalFormat = GL_SRGB8_ALPHA8;
    #else 
        GLenum colorTexInternalFormat = GL_RGBA8;
    #endif
    GLuint colorTex = texture(colorTexInternalFormat, GL_RGBA);
    GLuint depthTex = texture(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    GLenum colorAttachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, colorAttachments);

    glViewport(0, 0, res.x, res.y);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    #ifdef ENABLE_FRAMEBUFFER_SRGB
        glEnable(GL_FRAMEBUFFER_SRGB);
    #endif

    while (!glfwWindowShouldClose(window)) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // red, straight up square
        glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0)));
        glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection * redModel));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        #ifdef DISABLE_SRGB_BEFORE_DEPTH_BLIT
            GLboolean enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
            #ifdef SEPARATE_COLOR_AND_DEPTH_BLIT
                glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                glDisable(GL_FRAMEBUFFER_SRGB);
                glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                if(enabled) glEnable(GL_FRAMEBUFFER_SRGB);
            #else
                glDisable(GL_FRAMEBUFFER_SRGB);
                glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                if(enabled) glEnable(GL_FRAMEBUFFER_SRGB);
            #endif
        #else
            #ifdef SEPARATE_COLOR_AND_DEPTH_BLIT
                glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            #else
                glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            #endif
        #endif
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // green, tilted square
        glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0)));
        glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection * greenModel));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
