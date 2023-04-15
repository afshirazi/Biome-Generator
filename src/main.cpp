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

#define ds_rand (rand() % 3) - 1


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

std::string inputGeoShader()
{
    std::ifstream istr("shaders/g.glsl");
    std::string geo;
    std::stringstream gShaderStream;
    gShaderStream << istr.rdbuf();
    geo = gShaderStream.str();

    istr.close();
    return geo;
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


int sq_step( int r, int tl, int tr, int bl, int br)
{
    //int tl = arr[x][y];
    //int tr = arr[x][y + step_size];
    //int bl = arr[x + step_size][y];
    //int br = arr[x + step_size][y + step_size];

    int avg = tl + tr + bl + br;
    avg /= 4;
    return r + avg;
}

int di_step(int r, int t, int b, int l, int ri)
{
    //int t = arr[x][y - half_step];
    //int b = arr[x][y + half_step];
    //int l = arr[x - half_step][y];
    //int ri = arr[x + half_step][y];

    int avg = t + b + l + ri;
    if (t == 0 || b == 0 || l == 0 || ri == 0)
        avg /= 3;
    else
        avg /= 4;
    return r + avg;
}

int custom_rand_func()
{
    int r = rand();
    return r;
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
    GLuint geoShader = glCreateShader(GL_GEOMETRY_SHADER);
    std::string _vertShaderSrc = inputVertShader();
    std::string _geoShaderSrc = inputGeoShader();
    std::string _fragShaderSrc = inputFragShader();

    const char* vertShaderSrc = _vertShaderSrc.c_str();
    const char* geoShaderSrc = _geoShaderSrc.c_str();
    const char* fragShaderSrc = _fragShaderSrc.c_str();

    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    glShaderSource(geoShader, 1, &geoShaderSrc, NULL);
    glCompileShader(geoShader);

    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Attach to program
    // Link program
    GLuint shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vertShader);
    glAttachShader(shaderProg, geoShader);
    glAttachShader(shaderProg, fragShader);
    glLinkProgram(shaderProg);

    glDeleteShader(vertShader);
    glDeleteShader(geoShader);
    glDeleteShader(fragShader);
    
    // Set up indices/vertices for the 100x100 grid
    
    std::vector<glm::vec3> vertInfo;
    std::vector<glm::uvec3> indices;

    std::vector<glm::vec3> vertCopy;

    int dim = 64;

    int arr[65][65];
    arr[0][0] = 16;
    arr[0][64] = 4;
    arr[64][0] = -3;
    arr[64][64] = 15;

    int step_size = 64;

    while (step_size > 1)
    {
        int half_step = step_size / 2;
        
        for (int x = half_step; x < 64; x += step_size)
            for (int y = half_step; y < 64; y += step_size)
            {
                arr[x][y] = sq_step(ds_rand, arr[x - half_step][y - half_step], arr[x + half_step][y - half_step], arr[x - half_step][y + half_step], arr[x + half_step][y + half_step]);
            }
        
        for (int x = 0; x <= 64; x += step_size)
            for (int y = 0; y <= 64; y += step_size)
            {
                if (x + half_step < 64)
                {
                    int t = (y - half_step) >= 0 ? arr[x + half_step][y - half_step] : 0;
                    int b = (y + half_step) <= 64 ? arr[x + half_step][y + half_step] : 0;
                    int l = x >= 0 ? arr[x][y] : 0;
                    int ri = (x + step_size) <= 64 ? arr[x + step_size][y] : 0;
                    arr[x + half_step][y] = di_step(ds_rand, t, b, l, ri);
                }
                
                if (y + half_step < 64)
                {
                    int t = y >= 0 ? arr[x][y] : 0;
                    int b = (y + step_size) <= 64 ? arr[x][y + step_size] : 0;
                    int l = (x - half_step) >= 0 ? arr[x - half_step][y + half_step] : 0;
                    int ri = (x + half_step) <= 64 ? arr[x + half_step][y + half_step] : 0;
                    arr[x][y + half_step] = di_step(ds_rand, t, b, l, ri);
                }
            }

        step_size /= 2;
    }

    std::cout << ds_rand << std::endl;

    //for (int i = 0; i <= dim; i++)
    //    for (int j = 0; j <= dim; j++)
    //        std::cout << arr[i][j] << std::endl;

    for (int i = 0; i <= dim; i++)
        for (int j = 0; j <= dim; j++)
        {
            GLfloat x = (float)j / (float)dim;
            GLfloat y = (float)i / (float)dim;
            GLfloat z = arr[i][j] / 100.f;
            //GLfloat z = 0;

            vertInfo.push_back(glm::vec3(x, y, z)); // pos
            vertInfo.push_back(glm::vec3(0.8f, 0.5f, 0.f)); // col
        }

    for (int i = 0; i < dim; i+=1)
        for (int j = 0; j < dim; j+=1)
        {
            GLuint tl = i * (dim + 1) + j;
            GLuint tr = i * (dim + 1) + j + 1;
            GLuint bl = (i + 1) * (dim + 1) + j;
            GLuint br = (i + 1) * (dim + 1) + j + 1;

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
    glBufferData(GL_ARRAY_BUFFER, vertInfo.size() * sizeof(glm::vec3), glm::value_ptr(vertInfo.at(0)), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0); // pos
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3*sizeof(GLfloat))); // color
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Create an EBO for the triangles
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), glm::value_ptr(indices.at(0)), GL_STATIC_DRAW);

    std::cout << indices.size() << " " << vertInfo.size() << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProg);

        // creating perspective
        glm::mat4 proj = glm::mat4(1.0f);
        proj = glm::perspective(glm::radians(45.0f), 1.f, 1.f, 100.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.f, 0.f, 1.f));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(-0.65f, -0.3f, -2.f));

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