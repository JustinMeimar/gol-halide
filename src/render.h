#ifndef RENDER_H
#define RENDER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <math.h>
#include <Halide.h>

using GridBuffer = Halide::Buffer<uint8_t>;

namespace render {

/// @TODO: Move these into GLSL files and read into string?
struct ShaderSources {
    const char* vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec3 aColor;
        out vec3 vertexColor;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            vertexColor = aColor;
        }
    )";

    const char* fragmentShader = R"(
        #version 330 core
        in vec3 vertexColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vertexColor, 1.0);
        }
    )";
};

struct GridConfig {
    int yDim;
    int xDim;
    
    GridConfig(int x, int y, std::string title, size_t maxTicks)
        : xDim(static_cast<int>(x)),
          yDim(static_cast<int>(y)),
          title(title),
          maxTicks(maxTicks),
          windowWidth(1000),
          windowHeight(1000)
    {
        cellSize = 2.0f / std::max(xDim, yDim); 
    }

    int windowWidth;    // width in pixels
    int windowHeight;   // height in pixels
    std::string title;
    std::size_t maxTicks;
    float cellSize; 
};

class OpenGLContext {
private:
    GLFWwindow* window;
    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint colorVBO;
    std::vector<float> vertices;
    std::vector<float> colors;
    std::size_t frameCount;
    
    // An owned value of grid config
    GridConfig config;

public:
    OpenGLContext(GridConfig&& config) : frameCount(0), config(std::move(config)) {
        initGLFW(this->config);
        initGLEW();
        initShaders();
        createGrid(this->config);
    }

    ~OpenGLContext() {
        cleanup();
    }

    inline bool shouldClose() {
        return (glfwWindowShouldClose(window) || frameCount > this->config.maxTicks);
    }

    void swapBuffersAndPollEvents() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    void renderFrame(const GridConfig& config, const GridBuffer& buff);

private:
    /// boilerplate
    void initGLFW(const GridConfig& config); 
    void initGLEW(); 
    void initShaders();
    void createGrid(const GridConfig& config); 
    void checkShaderCompilation(GLuint shader, const std::string& type);  
    void checkProgramLinking(GLuint program); 
    void cleanup(); 
};

};

#endif // RENDER_H
