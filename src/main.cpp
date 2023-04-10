#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Trim leading and trailing whitespace from a line (taken from HW4)
std::string trim(const std::string& line) {
    const std::string whitespace = " \t\r\n";
    auto first = line.find_first_not_of(whitespace);
    if (first == std::string::npos)
        return "";
    auto last = line.find_last_not_of(whitespace);
    auto range = last - first + 1;
    return line.substr(first, range);
}

// Reads lines from istream, stripping whitespace,
// until it finds a nonempty line (taken from HW4)
std::string getNextLine(std::istream& istr) {
    std::string line = "";
    while (line == "") {
        std::getline(istr, line);
        line = trim(line);
    }
    return line;
} // not used

std::string inputVertShader()
{
    std::ifstream istr("shaders/v.glsl");
    std::string vert;
    std::stringstream vShaderStream;
    vShaderStream << istr.rdbuf();
    vert = vShaderStream.str();

    istr.close();
    return vert;
}

std::string inputFragShader()
{
    std::ifstream istr("shaders/f.glsl");
    std::string frag;
    std::stringstream fShaderStream;
    fShaderStream << istr.rdbuf();
    frag = fShaderStream.str();

    istr.close();
    return frag;
}

int main()
{
    //window/GLFW setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Biome Gen", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Input the shaders
    // Compile them
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string _vertShaderSrc = inputVertShader();
    std::string _fragShaderSrc = inputFragShader();

    const char* vertShaderSrc = _vertShaderSrc.c_str();
    const char* fragShaderSrc = _fragShaderSrc.c_str();

    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Attach to program
    // Link program
    GLuint shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vertShader);
    glAttachShader(shaderProg, fragShader);
    glLinkProgram(shaderProg);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    // Set up indices/vertices for the 100x100 grid
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::uvec3> indices;

    int dim = 101;

    for (int i = 0; i <= dim; i++)
        for (int j = 0; j <= dim; j++)
        {
            GLfloat x = (float)j / (float)dim;
            GLfloat y = (float)i / (float)dim;
            GLfloat z = 0.f;

            //std::cout << i*(dim+1)+j << " x: " << x << " y: " << y << " z: " << z << std::endl;

            vertices.push_back(glm::vec3(x, y, z)); // pos
            //vertices.push_back(glm::vec3(x, y, (i+j)/200.f)); // color
        }

    for (int i = 0; i < dim; i+=1)
        for (int j = 0; j < dim; j+=1)
        {
            GLuint tl = i * (dim + 1) + j;
            GLuint tr = i * (dim + 1) + j + 1;
            GLuint bl = (i + 1) * (dim + 1) + j;
            GLuint br = (i + 1) * (dim + 1) + j + 1;

            //std::cout << i*(dim)+j << " tl: " << tl << " tr: " << tr << " bl: " << bl << " br: " << br << std::endl;

            indices.push_back(glm::uvec3(tl, tr, bl)); // top triangle
            indices.push_back(glm::uvec3(tr, bl, br)); // bottom triangle
        }
    
    
    // Create a VAO to store the VBO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create a VBO to store them
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), glm::value_ptr(vertices.at(0)), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // pos
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)(3*sizeof(GLfloat))); // color
    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);

    // Create an EBO for the triangles
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), glm::value_ptr(indices.at(0)), GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProg);

        // creating perspective
        glm::mat4 proj = glm::mat4(1.0f);
        proj = glm::perspective(glm::radians(45.0f), 1.f, 0.1f, 100.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(0.f, 0.f, 1.f));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(-0.7f, 0.2f, -2.f));

        GLint modelint = glGetUniformLocation(shaderProg, "model");
        glUniformMatrix4fv(modelint, 1, GL_FALSE, glm::value_ptr(model));

        GLint viewint = glGetUniformLocation(shaderProg, "view");
        glUniformMatrix4fv(viewint, 1, GL_FALSE, glm::value_ptr(view));

        GLint projint = glGetUniformLocation(shaderProg, "proj");
        glUniformMatrix4fv(projint, 1, GL_FALSE, glm::value_ptr(proj));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}