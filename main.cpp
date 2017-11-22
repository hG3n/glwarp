
// Include standard headers
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// GL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// X11
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>

static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// function callbacks
GLuint LoadShaders(const char *, const char *);

int main(void) {
    /**
     * get x11 client reference
     */

    Display *display;
    Window root_window;
    int scr;

    display = XOpenDisplay(nullptr);
    scr = DefaultScreen(display);
    root_window = DefaultRootWindow(display);
//    int screen_w = DisplayWidth(display, scr) / 2;
//    int screen_h = DisplayHeight(display, scr) / 2;
    XImage *image;


    // Initialise GLFW
    GLFWwindow *glfw_window;
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a glfw_window and create its OpenGL context
    glfw_window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GLWarp", nullptr, nullptr);
    if (glfw_window == nullptr) {
        fprintf(stderr, "Failed to open GLFW glfw_window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(glfw_window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // stuff
    glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
    glfwSwapInterval(0); // disable vsync

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);


    /**
     * load shaders
     */
    GLuint program_id = LoadShaders("../simple.vert", "../simple.frag");


    /**
     * build model view projection matrix
     */
    // Get a handle for our "MVP" uniform
    GLuint matrix_id = glGetUniformLocation(program_id, "MVP");

    // projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

    // camera matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, 2.5), // Camera is at (4,3,3), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // model
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 scale = glm::scale(model, glm::vec3(16.0f / 9.0f, 1.0f, 1.0f));

    // MVP
    glm::mat4 MVP = projection * view * model * scale; // Remember, matrix multiplication is the other way around


    /**
     * object data
     */
    static const GLfloat g_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
    };

    static const GLfloat g_uv_buffer_data[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
    };


    /**
     * create buffers
     */
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


    /**
     * build screenshot texture
     */
    // get initial image
    image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);

    // create and bind new texture
    GLuint screen_texture;
    glGenTextures(0, &screen_texture);
    glBindTexture(GL_TEXTURE_2D, screen_texture);

    // specify 2D texture image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // further settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // get uniform texture location in fragment shader
    GLuint screen_texture_id = glGetUniformLocation(program_id, "myTextureSampler");


    /**
     * main loop
     */
    bool running = true;
    double last_time = glfwGetTime();
    int num_frames = 0;
    while (running && glfwWindowShouldClose(glfw_window) == 0) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /**
         * print render time per frame
         */
        ++num_frames;
        double current_time = glfwGetTime();
        if (current_time - last_time >= 1.0) {
            std::cout << "ms/frame: " << (1000.0 / double(num_frames)) << std::endl;
            num_frames = 0;
            last_time += 1.0;
        }

        /**
         * get screenshot
         */
        image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);
        if (!image) {
            printf("Unable to create image...\n");
        }

        // use shader
        glUseProgram(program_id);

        // send transformations to shader
        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

        // create texture from captured screenshot
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
        glUniform1i(screen_texture_id, 0);

        /**
         * specify vertex arrays of vertices and uv's
         * draw finalyy
         */
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // must match shader layout
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0          // array buffer offset
        );

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                1,                  // must match shader layout
                2,                  // size : U+V => 2
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0          // array buffer offset
        );


        // draw
        glDrawArrays(GL_TRIANGLES, 0, 2*3);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(glfw_window);
        glfwPollEvents();

        // important otherwise memory will be full soon
        XDestroyImage(image);

        // check for keyboard input
        if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(glfw_window, GLFW_KEY_Q) == GLFW_PRESS) {
            running = false;
        }

    }



    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(program_id);
    glDeleteTextures(1, &screen_texture);
    glDeleteVertexArrays(1, &vertex_array_id);

    /**
     * x11 cleanup
     */

    XCloseDisplay(display);

    // Close OpenGL glfw_window and terminate GLFW
    glfwTerminate();

    return 0;
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
