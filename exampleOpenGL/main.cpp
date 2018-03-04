//
//  port of the Cherno project hello world
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

// std_image.h is another Cheron header only option - using this for udemy class
// #include "vendor/SOIL2/SOIL2.h"

#define ASSERT(x) if (!(x)) abort();
#define GlCall(x) GlClearError();\
x;\
ASSERT(GlLogCall(#x, __FILE__, __LINE__))

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// glDebugMessageCallback in 4.3 - Mac seems to stop at 4.1 - mine is 4.1

static void GlClearError() {
    while(glGetError() != GL_NO_ERROR);
}

static bool GlLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "):" << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}
static std::string getShader(const std::string& filepath) {
    
    std::ifstream stream(filepath);
    std::string line;
    std::string shaderText;
    while (getline(stream, line)) {
        std::cout << "my line is: " << line << std::endl;
        shaderText += line + '\n';
    }
    return shaderText;
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " <<  (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    } else {
        std::cout << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " compiled successfully" << std::endl;
    }
    
    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    int result;
    glGetShaderiv(program, GL_LINK_STATUS, &result);
    
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(program, GL_LINK_STATUS, &length);
        char* message = (char *)alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);
        std::cout << "Failed to link shaders!" << std::endl;
        std::cout << message << std::endl;
        glDeleteProgram(program);
        return 0;
    } else {
        std::cout << "Linked successfully" << std::endl;
    }
    
    glValidateProgram(program);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return program;
}

const GLint WIDTH = 800, HEIGHT = 600;

int main(int argc, const char * argv[]) {

    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // I can't change this to compat and remove the VAO binding (using 4.1)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // needed for mac
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    
    glfwSwapInterval(1); // was smooth before this - might be the only thing that is automatic on a mac
    
    glewExperimental = GL_TRUE; // needed for mac
    if (glewInit() != GLEW_OK) {
        std::cout << "glewInit failed" << std::endl;
        return 0;
    }

    std::cout << glfwGetVersionString() << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float positions[] = {
        0.5f, 0.5f, 0.0f,    1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f,   1.0f, 0.0f, 1.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0,1,3, // first triangle
        1,2,3  // second triangle
    };
    
    // apparently just an opengl construct
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * 4 * sizeof(float), positions, GL_STATIC_DRAW);
    
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    // index 0 of VAO is being bound to currently bound gl array buffer
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid *) (sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    // texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid *) (sizeof(float) * 6));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);

    GLuint texture;
    int width, height, bpp;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // not using Cherno's stbi_set_flip_text_on_load since it's in the fragment shader
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *image = stbi_load("res/tianjin_tower.jpg", &width, &height, &bpp, 4);
    
    GlCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image));
    glBindTexture(GL_TEXTURE_2D, 0);
  
    std::string vertexShader = getShader("res/vertex.shader");
    std::string fragmentShader = getShader("res/fragment.shader");
    
    std::cout << "shaders" << std::endl;
    std::cout << vertexShader << std::endl;
    std::cout << fragmentShader << std::endl;
    
    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader); // this must be bound to use uniform
    
    // glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
    // GlCall(glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), 1, GL_FALSE, &proj[0][0]));
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        
        glClear(GL_COLOR_BUFFER_BIT);
  
        glUseProgram(shader);
        
        glm::mat4 transform;
        
        transform = glm::translate(transform, glm::vec3(0.5f, -0.0f, 0.0f));
        transform = glm::rotate(transform, (GLfloat)glfwGetTime() * -1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        
        // probably just need to retrieve locations once.
        GlCall(glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), 1, GL_FALSE, glm::value_ptr(transform)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "ourTexture1"), 0);
        
        glBindVertexArray(VAO);
        GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
        
        GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        glBindVertexArray(0);
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    if (image) stbi_image_free(image);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &buffer);
    glDeleteBuffers(1, &EBO);
    
    glDeleteShader(shader);

    glfwTerminate();
    return 0;
    
}
