#include "render.h"

namespace render {

void OpenGLContext::renderFrame(const GridConfig& config) {
    std::cout << "Rendering frame: " << frameCount << std::endl;
    
    // Update colors based on frame count
    for (size_t i = 0; i < colors.size(); i += 18) {  // 18 = 6 vertices * 3 color components
        float r = sin(frameCount * 0.01f + i * 0.1f) * 0.5f + 0.5f;
        float g = cos(frameCount * 0.02f + i * 0.05f) * 0.5f + 0.5f;
        float b = sin(frameCount * 0.03f + i * 0.15f) * 0.5f + 0.5f;
        
        // Set same color for all 6 vertices of this cell
        for (int j = 0; j < 18; j += 3) {
            colors[i + j    ] = r;
            colors[i + j + 1] = g;
            colors[i + j + 2] = b;
        }
    }

    // Update color buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float),
                colors.data(), GL_DYNAMIC_DRAW);

    // Render
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, config.size * config.size * 6);
    
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
    for (int y = 0; y < config.size; y++) {
        for (int x = 0; x < config.size; x++) {
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

            for (int i = 0; i < 6; i++) {  // 6 vertices per cell
                colors.insert(colors.end(), {
                    1.0f, 1.0f, 1.0f  // Start with white
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

void GridApplication::run(GridConfig&& config) {
    try {
        // GLContext owns the Grid Config
        OpenGLContext context(std::move(config));
        
        while (!context.shouldClose()) {
            context.renderFrame(config);
            context.swapBuffersAndPollEvents();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

} // namespace render

// int main() {
//     GridConfig config{
//         .size = 80,
//         .cellSize = 2.0f / 80,
//         .windowWidth = 800,
//         .windowHeight = 800,
//         .title = "Grid"
//     };
//
//     GridApplication::run(config);
//     return 0;
// }

