//
//  port of the Cherno project hello world
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#define ASSERT(x) if (!(x)) abort();
#define GlCall(x) GlClearError();\
x;\
ASSERT(GlLogCall(#x, __FILE__, __LINE__))

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

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
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    
    float positions[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0,1,2
    };
    
    // apparently just an opengl construct
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), positions, GL_STATIC_DRAW);
    
    // index 0 of VAO is being bound to currently bound gl array buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid *) (sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), positions, GL_STATIC_DRAW);

    unsigned int IBA;
    glGenBuffers(1, &IBA);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBA);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    std::string vertexShader = getShader("res/vertex.shader");
    std::string fragmentShader = getShader("res/fragment.shader");
    
    std::cout << "shader dude" << std::endl;
    std::cout << vertexShader << std::endl;
    
    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader); // this must be bound to use uniform
    
    // ask for where u_Color is
    // GlCall(int location = glGetUniformLocation(shader, "u_Color"));
    // ASSERT(location != -1);
    // GlCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));
    
    // unbind buffers after initial setup
//    glBindVertexArray(0);
//    glUseProgram(0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // float r = 0.0f;
    // float increment = 0.01f;
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        
        // GlCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glUseProgram(shader);
        // GlCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        // GlCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
        glBindVertexArray(VAO);
        GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBA));
        
        // these may have changed if another vertex array was drawn
        // might have different VAO for each object drawn
        // GlCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
        // GlCall(glEnableVertexAttribArray(0));
        
        GlCall(glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr));
        
//        r += increment;
//        if (r > 1.0f) {
//            increment = -0.01;
//            r = 1.0f;
//        } else if (r < 0.0f) {
//            increment = 0.01;
//            r = 0.0f;
//        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glDeleteShader(shader);

    glfwTerminate();
    return 0;
    
}
