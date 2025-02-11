#include "render.h"

namespace render {

/// @desc: The openGL vertex buffer is composed of 6 verticies per cell making for
/// yDim*xDim*6 verticies in total. For each vertex, there is three colors, therefore
/// the openGL color buffer has yDim*xDim*18 elements in total.
///
void OpenGLContext::renderFrame(const GridConfig& config, const GridBuffer& buff) {

    for (int y = 0; y < config.yDim; y++) {
        for (int x = 0; x < config.xDim; x++) {
            // color index for this cell
            size_t color_index = (y * config.xDim + x) * 18; 
            
            // cell state from Halide buffer
            bool is_alive = static_cast<bool>(buff(x, y));
            
            // Set black and white values based on cell state
            float r = is_alive ? 1.0f : 0.0f;
            float g = is_alive ? 1.0f : 0.0f;
            float b = is_alive ? 1.0f : 0.0f;
            
            // Update color buffer for all 6 vertices of this cell
            for (int vertex = 0; vertex < 6; vertex++) {
                colors[color_index + vertex * 3]     = r;
                colors[color_index + vertex * 3 + 1] = g;
                colors[color_index + vertex * 3 + 2] = b;
            }
        }
    }
    
    // Update OpenGL color buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float),
                colors.data(), GL_DYNAMIC_DRAW);
    
    // Clear the screen and render
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    
    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, config.xDim * config.yDim * 6);
    
    frameCount++;
}

void OpenGLContext::initGLFW(const GridConfig& config) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(config.windowWidth, config.windowHeight, 
                            config.title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }
    glfwMakeContextCurrent(window);
}

void OpenGLContext::initGLEW() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW");
    }
}

void OpenGLContext::initShaders() {
    ShaderSources sources;
    
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &sources.vertexShader, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader, "vertex");

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &sources.fragmentShader, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompilation(fragmentShader, "fragment");

    // Create and link shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLinking(shaderProgram);

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void OpenGLContext::createGrid(const GridConfig& config) {
    vertices.clear();
    colors.clear();
    for (int y = 0; y < config.yDim; y++) {
        for (int x = 0; x < config.xDim; x++) {
            float x1 = -1.0f + x * config.cellSize;
            float y1 = -1.0f + y * config.cellSize;
            float x2 = x1 + config.cellSize;
            float y2 = y1 + config.cellSize;

            vertices.insert(vertices.end(), {
                x1, y1,  // Triangle 1
                x2, y1,
                x2, y2,
                x1, y1,  // Triangle 2
                x2, y2,
                x1, y2
            });

            for (int i = 0; i < 6; i++) {
                colors.insert(colors.end(), {
                    0.0f, 0.0f, 0.0f 
                });
            }
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &colorVBO);
    
    // Position attribute
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float),
                colors.data(), GL_DYNAMIC_DRAW);  // Note: DYNAMIC_DRAW
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

void OpenGLContext::checkShaderCompilation(GLuint shader, const std::string& type) {
    GLint success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        throw std::runtime_error("Shader compilation failed (" + type + "): " + infoLog);
    }
}

void OpenGLContext::checkProgramLinking(GLuint program) {
    GLint success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
    }
}

void OpenGLContext::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace render
