// Include standard headers
#include <iostream>
#include <vector>
#include <fstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>

GLFWwindow *window;

enum ShaderType {
    VERTEX = 0,
    FRAGMENT = 1
};

// function callbacks
bool readShader(const char *, std::string &);

bool loadShader(const char *, ShaderType, GLuint &);

GLuint LoadShaders(const char *, const char *);

/**
 * main
 * @return
 */
int main() {

    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // get fullscreen resolution
//    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
//    int width = mode->width;
//    int height = mode->height;

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(800, 600, "GLWarp", nullptr, nullptr);
//    window = glfwCreateWindow(width, height, "GLWarp", glfwGetPrimaryMonitor(), NULL);
    if (window == nullptr) {
        fprintf(stderr,
                "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    // don't on OSX according to https://stackoverflow.com/a/27048287
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // create vertex array object (VAO)
    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    // create array containing vertex information
    static const GLfloat vertexbuffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
    };

    // create vertex buffer
    GLuint vertexbuffer_id;
    glGenBuffers(1, &vertexbuffer_id); // generate one buffer represented by vertexbuffer_id
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexbuffer_data), vertexbuffer_data, GL_STATIC_DRAW);


    // generate shaders
    GLuint program_id = glCreateProgram();
    if (!loadShader("../simple.vert", VERTEX, program_id)) {
        return 0;
    }
    if (!loadShader("../simple.frag", FRAGMENT, program_id)) {
        return 0;
    }

    std::cout << program_id << std::endl;

//    GLuint program_id = LoadShaders("../simple.vert", "../simple.frag");


    // draw loop
    bool running = true;
    while (running && glfwWindowShouldClose(window) == 0) {

        // Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
        glClear(GL_COLOR_BUFFER_BIT);

        // use shader
        glUseProgram(program_id);

        // draw
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_id);
        glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );

        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // check for keyboard input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            running = false;
        }
    }


    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer_id);
    glDeleteVertexArrays(1, &vertex_array_id);
    glDeleteProgram(program_id);

// Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


/**
 * read shader from hd
 * @param file_path
 * @param out_stream
 * @return
 */
bool readShader(const char *file_path, std::string &out_stream) {
    std::ifstream in_stream(file_path, std::ios::in);
    if (in_stream.is_open()) {
        std::string line = "";
        while (getline(in_stream, line))
            out_stream += "\n" + line;
        in_stream.close();
    } else {
        std::cout << "Could not open file'" << file_path << "'!" << std::endl;
        getchar();
        return false;
    }
    return true;
}


/**
 * load shaders from file
 * @param file_path
 * @param fragment_file_path
 * @return
 */
bool loadShader(const char *file_path, ShaderType type, GLuint &program_id) {

    std::cout << program_id << std::endl;

    GLuint shader_id;
    switch (type) {
        case VERTEX:
            shader_id = glCreateShader(GL_VERTEX_SHADER);
            break;
        case FRAGMENT:
            shader_id = glCreateShader(GL_FRAGMENT_SHADER);
            break;
    }

    // Read the Vertex Shader code from the file
    std::string shader_code;
    if (!readShader(file_path, shader_code)) {
        return false;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", file_path);
    char const *source_pointer = shader_code.c_str();
    glShaderSource(shader_id, 1, &source_pointer, NULL);
    glCompileShader(shader_id);

    // Check Vertex Shader
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> shader_error_message(InfoLogLength + 1);
        glGetShaderInfoLog(shader_id, InfoLogLength, NULL, &shader_error_message[0]);
        printf("%s\n", &shader_error_message[0]);
    }

    // Link the program
    printf("Linking program\n");
    glAttachShader(program_id, shader_id);
    glLinkProgram(program_id);

    // Check the program
    glGetProgramiv(program_id, GL_LINK_STATUS, &Result);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(program_id, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(program_id, shader_id);
    glDeleteShader(shader_id);

    return true;
}

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    } else {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n",
               vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }



    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }



    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }


    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

