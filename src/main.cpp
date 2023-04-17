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

char plane_view = 'A';

void change_view_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        plane_view = 'A';
    else if (key == GLFW_KEY_B && action == GLFW_PRESS)
        plane_view = 'B';
    else if (key == GLFW_KEY_C && action == GLFW_PRESS)
        plane_view = 'C';
}

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

int get_biome(int x, int y, std::vector<glm::uvec3> coords)
{
    double dist = 100000;
    int biome = 0;
    for (glm::uvec3 coord : coords)
    {
        double newdist = glm::sqrt((x / 129.0 - coord.x / 100.0) * (x / 129.0 - coord.x / 100.0) + (y / 129.0 - coord.y / 100.0) * (y / 129.0 - coord.y / 100.0));
        biome = newdist < dist ? coord.z : biome;
        dist = newdist < dist ? newdist : dist;
    }

    return biome;
}

int custom_rand_func(int x, int y, std::vector<glm::uvec3> coords)
{
    int biome = get_biome(x, y, coords);
    
    int r;
    switch (biome)
    {
    case 'M':
        r = (rand() % 71) - 35;
        if (r < -3 || r > 3)
        {
            r = -r * r / 200 + 3;
        }
        else
        {
            r = -4 * r * r + 100;
        }
        break;
    case 'D':
        r = (rand() % 71) - 35;
        r = -r * r / 200 + 3;
        break;
    case 'F':
        r = (rand() % 71) - 35;
        r = -r * r / 200 + 3;
        break;
    default:
        r = (rand() % 71) - 35;
        if (r < -3 || r > 3)
        {
            r = -r * r / 200 + 3;
        }
        else
        {
            r = -4 * r * r + 80;
        }
    }
    
    return r;
}

glm::vec3 get_grid_col(float x, float y, float z, std::vector<glm::uvec3> coords)
{
    double dist = 100000;
    int biome = 0;
    for (glm::uvec3 coord : coords)
    {
        double newdist = glm::sqrt((x - coord.x / 100.0) * (x - coord.x / 100.0) + (y - coord.y / 100.0) * (y - coord.y / 100.0));
        biome = newdist < dist ? coord.z : biome;
        dist = newdist < dist ? newdist : dist;
    }

    switch (biome)
    {
    case 'M':
        if (z < 0.15f)
            return glm::vec3(0.27f, 0.62f, 0.28f);
        return glm::vec3(1.f, 0.98f, 0.98f);
    case 'D':
        return glm::vec3(0.8f, 0.5f, 0.f);
    case 'O':
        return glm::vec3(0.05f, 0.28f, 0.63f);
    case 'F':
        return glm::vec3(0.06f, 0.14f, 0.063f);
    default:
        return glm::vec3(0.8f, 0.5f, 0.f);
    }
}

int lerp(int a, int b, int step)
{
    return (a * (1 - step/9.0)) + (b * step/9.0);
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
    glfwSetKeyCallback(window, change_view_callback);

    glEnable(GL_DEPTH_TEST);

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
    

    // vector to store chosen biome coordinates
    std::vector<glm::uvec3> coords;

    coords.push_back(glm::uvec3(41, 58, 'D'));
    coords.push_back(glm::uvec3(34, 37, 'M'));
    coords.push_back(glm::uvec3(52, 87, 'O'));
    coords.push_back(glm::uvec3(98, 99, 'F'));

    // Set up indices/vertices for the grid
    std::vector<glm::vec3> vertInfo;
    std::vector<glm::uvec3> indices;

    const int dim = 128;

    std::vector<std::vector<int>> arr(dim + 1, std::vector<int> (dim + 1));
    arr[0][0] = 60;
    arr[0][dim] = 10;
    arr[dim][0] = 0;
    arr[dim][dim] = 50;

    int step_size = dim;

    while (step_size > 1)
    {
        int half_step = step_size / 2;
        
        for (int x = half_step; x < dim; x += step_size)
            for (int y = half_step; y < dim; y += step_size)
            {
                arr[x][y] = sq_step(custom_rand_func(x, y, coords), arr[x - half_step][y - half_step], arr[x + half_step][y - half_step], arr[x - half_step][y + half_step], arr[x + half_step][y + half_step]);
            }
        
        for (int x = 0; x <= dim; x += step_size)
            for (int y = 0; y <= dim; y += step_size)
            {
                if (x + half_step < dim)
                {
                    int t = (y - half_step) >= 0 ? arr[x + half_step][y - half_step] : 0;
                    int b = (y + half_step) <= dim ? arr[x + half_step][y + half_step] : 0;
                    int l = x >= 0 ? arr[x][y] : 0;
                    int ri = (x + step_size) <= dim ? arr[x + step_size][y] : 0;
                    arr[x + half_step][y] = di_step(custom_rand_func(x, y, coords), t, b, l, ri);
                }
                
                if (y + half_step < dim)
                {
                    int t = y >= 0 ? arr[x][y] : 0;
                    int b = (y + step_size) <= dim ? arr[x][y + step_size] : 0;
                    int l = (x - half_step) >= 0 ? arr[x - half_step][y + half_step] : 0;
                    int ri = (x + half_step) <= dim ? arr[x + half_step][y + half_step] : 0;
                    arr[x][y + half_step] = di_step(custom_rand_func(x, y, coords), t, b, l, ri);
                }
            }

        step_size /= 2;
    }

    // make sure oceans are set to sea level (0)
    for (int i = 0; i <= dim; i++)
        for (int j = 0; j <= dim; j++)
        {
            int biome = get_biome(j, i, coords);
            if (biome == 'O')
                arr[j][i] = 0;
        }

    // lerp 
    for (int i = 0; i < dim; i++) // over x
    {
        for (int j = 0; j < dim - 9; j+=10)
        {
            for (int k = 0; k < 10; k++)
                arr[j + k][i] = lerp(arr[j][i], arr[j + 9][i], k);
        }
    }
    for (int i = 0; i < dim - 9; i+=10) // over y
    {
        for (int j = 0; j < dim; j++)
        {
            for (int k = 0; k < 10; k++)
                arr[j][i + k] = lerp(arr[j][i], arr[j][i + 9], k);
        }
    }
        

    for (int i = 0; i <= dim; i++)
        for (int j = 0; j <= dim; j++)
        {
            GLfloat x = (float)j / (float)dim;
            GLfloat y = (float)i / (float)dim;
            GLfloat z = glm::max(arr[j][i] / 1000.f, 0.f);

            vertInfo.push_back(glm::vec3(x, y, z)); // pos
            glm::vec3 col = get_grid_col(x, y, z, coords);
            vertInfo.push_back(col); // col
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
    
    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProg);

        // creating perspective
        if (plane_view == 'A')
        {
            proj = glm::mat4(1.0f);
            model = glm::mat4(1.0f);
            view = glm::mat4(1.0f);
            proj = glm::perspective(glm::radians(45.0f), 1.f, 1.f, 100.0f);
            model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(1.f, 0.f, 0.f));
            model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.f, 0.f, 1.f));
            view = glm::translate(view, glm::vec3(-0.65f, -0.3f, -2.f));
        }
        else if (plane_view == 'B')
        {
            proj = glm::mat4(1.0f);
            model = glm::mat4(1.0f);
            view = glm::mat4(1.0f);
            view = glm::translate(view, glm::vec3(-0.5f, -0.5f, 0.f));
        }
        else if (plane_view == 'C')
        {
            proj = glm::mat4(1.0f);
            model = glm::mat4(1.0f);
            view = glm::mat4(1.0f);
            proj = glm::perspective(glm::radians(45.0f), 1.f, 1.f, 100.0f);
            model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(1.f, 0.f, 0.f));
            model = glm::rotate(model, glm::radians(120.0f), glm::vec3(0.f, 0.f, 1.f));
            view = glm::translate(view, glm::vec3(0.65f, -0.3f, -2.f));
        }
        
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