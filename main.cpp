
// Include standard headers
#include <iostream>
#include <vector>
#include <map>
#include <fstream>

// GL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// X11
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>


//static const int SCREEN_WIDTH = 500;
int SCREEN_WIDTH = 1920 / 2;
//static const int SCREEN_HEIGHT = 500;
int SCREEN_HEIGHT = 1080 / 2;

bool VSYNC = false;

bool capture_flag = false;
bool show_points = false;
bool show_polys = true;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>

// function callbacks
GLuint LoadShaders(const char *, const char *);

bool loadFile(const char *, std::vector<glm::vec3> *);

bool loadDomeMapFile(const char *filepath, std::vector<glm::vec3> *indexed_mask, std::vector<glm::vec3> *indexed_dome);

float mapToRange(float value, float in_min, float in_max, float out_min, float out_max);

GLuint loadBMP_custom(const char *);

int main(void) {

    //get x11 client reference
    Display *display;
    display = XOpenDisplay(NULL);
    Window root_window;
    root_window = DefaultRootWindow(display);

    // declare image pointer
    XImage *image; // test = XCreateImage( );

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
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int width = mode->width;
    int height = mode->height;

//    SCREEN_HEIGHT = height;
//    SCREEN_WIDTH = widthg
    glfw_window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GLWarp", NULL, NULL);
    if (glfw_window == NULL) {
        fprintf(stderr, "Failed to open GLFW glfw_window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(glfw_window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // stuff
    glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
    glEnable(GL_PROGRAM_POINT_SIZE);
    if (show_polys){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }



    glfwSwapInterval(0);
    if (VSYNC) {
        glfwSwapInterval(1);
    }

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    // load shaders
    GLuint program_id = LoadShaders("../simple.vert", "../simple.frag");

    // build model view projection matrix
    // Get a handle for our "MVP" uniform
    GLint matrix_id = glGetUniformLocation(program_id, "MVP");

    // projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

    // camera matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(0.0, 0.0, 3.0), // Camera is at (4,3,3), in World Space
            glm::vec3(0.0, 0.0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // model
    glm::mat4 model = glm::mat4(1.0f);
    //glm::mat4 scale = glm::scale(model, glm::vec3(16.0f / 9.0f, 1.0f, 1.0f));

    // MVP
//    glm::mat4 MVP = projection * view * model * scale; // Remember, matrix multiplication is the other way around
    glm::mat4 MVP = projection * view * model; //* scale; // Remember, matrix multiplication is the other way around


    /**
     * object data
     */
    int row_size = 18;
    int col_size = 32;
    int triangle_count = 0;
    float min_pos_x = -1.0f;
    float max_pos_x = 1.0f;
    float min_pos_y = -1.0f;
    float max_pos_y = 1.0f;
    std::vector<glm::vec3> blue, red;
    std::map<glm::vec3, glm::vec3> warp_map;

    bool b = loadFile("../blue.txt", &blue);
    bool r = loadFile("../red.txt", &red);


    // get meta information about calculated
    int circle_count = (int)blue.back().x;
    int points_per_circle = (int)blue.back().y;
    int point_count = (int)blue.back().z;

    blue.pop_back();
    blue.pop_back();
    std::cout << "size: " << blue.size() << " read point count: " << point_count <<  std::endl;
    //for (int i = 0; i < blue.size(); ++i) {
        //std::cout << "x:" << blue[i].x << " y: " << blue[i].y << std::endl;
    //}

    float min_val_x = 100;
    float max_val_x = -100;
    float min_val_y = 100;
    float max_val_y = -100;
    for (int i = 0; i < blue.size(); ++i) {
        if (blue[i].x < min_val_x) {
            min_val_x = blue[i].x;
        }
        if (blue[i].x > max_val_x) {
            max_val_x = blue[i].x;
        }
        if (blue[i].y < min_val_y) {
            min_val_y = blue[i].y;
        }
        if (blue[i].y > max_val_y) {
            max_val_y = blue[i].y;
        }
    }
    std::cout << "blue vector size:" << blue.size() << std::endl;
    std::cout << " blue min values:" << min_val_x << " " << min_val_y << " - max  values" << max_pos_x << " "
              << max_pos_x << std::endl;

    // map to dome grid to -1.0 to 1.0
    for (int i = 0; i < blue.size(); ++i) {
        blue[i].x = mapToRange(blue[i].x, min_val_x, max_val_x, -1.0f, 1.0f);
        blue[i].y = mapToRange(blue[i].y, min_val_y, max_val_y, -1.0f, 1.0f);
        //blue[i].z = 0.0f;
    }

    red.pop_back();
    red.pop_back();
    std::cout << "red size: " << red.size() << " read point count: " << point_count <<  std::endl;
    float red_min_val_x = 100;
    float red_max_val_x = -100;
    float red_min_val_y = 100;
    float red_max_val_y = -100;
    for (int i = 0; i < red.size(); ++i) {
        if (red[i].x < red_min_val_x) {
            red_min_val_x = red[i].x;
        }
        if (red[i].x > red_max_val_x) {
            red_max_val_x = red[i].x;
        }
        if (red[i].y < red_min_val_y) {
            red_min_val_y = red[i].y;
        }
        if (red[i].y > red_max_val_y) {
            red_max_val_y = red[i].y;
        }
    }
    std::cout << "red vector size:" << red.size() << std::endl;
    std::cout << " red min values:" << red_min_val_x << " " << red_min_val_y << " - max  values" << max_pos_x << " "
              << max_pos_x << std::endl;

    // map to dome grid to -1.0 to 1.0
    for (int i = 0; i < red.size(); ++i) {
        red[i].x = mapToRange(red[i].x, red_min_val_x, red_max_val_x, -1.0f, 1.0f);
        red[i].y = mapToRange(red[i].y, red_min_val_y, red_max_val_y, -1.0f, 1.0f);
        red[i].z = 0.0f;
    }

    std::vector<glm::vec3> mesh_vec;
    // create mesh
    for (int circle_idx = 0; circle_idx < circle_count; ++circle_idx) {
        if (circle_idx == 0){
            for (int t = 1; t < points_per_circle + 1; ++t){
                // start triangles around center
                //std::cout << 0 << " " << t << " " << 1 + (t % points_per_circle) << std::endl;
                int i1 = 0;
                int i2 = t;
                int i3 = 1 + (t % points_per_circle);
                if (blue[i1].z == 1) {
                    mesh_vec.push_back(blue[i1]);
                } else{
                    mesh_vec.push_back(red[i1]);
                }
                if (blue[i2].z == 1) {
                    mesh_vec.push_back(blue[i2]);
                } else{
                    mesh_vec.push_back(red[i2]);
                }
                if (blue[i3].z == 1) {
                    mesh_vec.push_back(blue[i3]);
                } else{
                    mesh_vec.push_back(red[i3]);
                }
                triangle_count += 1;
            }
        } else {
            int start_point = circle_idx * points_per_circle - (points_per_circle - 1);
            for (int idx = 0; idx < points_per_circle; ++idx){
                int i1 = start_point + idx;
                int i2 = start_point + idx + points_per_circle;
                int i3 = start_point + (idx + 1) % points_per_circle;
                // create quad
                if (blue[i1].z == 1) {
                    mesh_vec.push_back(blue[i1]);
                } else{
                    mesh_vec.push_back(red[i1]);
                }
                if (blue[i2].z == 1) {
                    mesh_vec.push_back(blue[i2]);
                } else{
                    mesh_vec.push_back(red[i2]);
                }
                if (blue[i3].z == 1) {
                    mesh_vec.push_back(blue[i3]);
                } else{
                    mesh_vec.push_back(red[i3]);
                }
                //std::cout << i1<< " " << i2 << " " << i3 << std::endl;
                int i4 = start_point + (idx + 1) % points_per_circle;
                int i5 = start_point + idx + points_per_circle;
                int i6 = start_point + ((idx + 1) % points_per_circle) + points_per_circle;
                if (blue[i4].z == 1) {
                    mesh_vec.push_back(blue[i4]);
                } else{
                    mesh_vec.push_back(red[i4]);
                }
                if (blue[i5].z == 1) {
                    mesh_vec.push_back(blue[i5]);
                } else{
                    mesh_vec.push_back(red[i5]);
                }
                if (blue[i6].z == 1) {
                    mesh_vec.push_back(blue[i6]);
                } else{
                    mesh_vec.push_back(red[i6]);
                }
                //std::cout << i4 << " " << i5<< " " << i6 << std::endl;
                triangle_count += 2;
            }
        }
    }

    std::vector<glm::vec2> tex_vec;
    for (int i = 0; i < mesh_vec.size(); ++i) {
        float u, v;
        if (blue[i].z > 0.0) {
//            u = mapToRange(dome_vec[i].x, min_val_x, max_val_x, 0.0f, 1.0f);
//            v = mapToRange(dome_vec[i].z, min_val_y, max_val_y, 0.0f, 1.0f);
            u = mapToRange(blue[i].x, min_pos_x, max_pos_x, 0.0f, 1.0f);
            v = mapToRange(blue[i].y, min_pos_y, max_pos_y, 0.0f, 1.0f);
        } else {
            u = mapToRange(mesh_vec[i].x, min_pos_x, max_pos_x, 0.0f, 1.0f);
            v = mapToRange(mesh_vec[i].y, min_pos_y, max_pos_y, 0.0f, 1.0f);
        }
        tex_vec.push_back(glm::vec2(u, v));
    }

    std::cout << "Number of mesh points: " << mesh_vec.size() << std::endl;
    std::cout << "Number of tex points: " << tex_vec.size() << std::endl;


    // create buffers
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, mesh_vec.size() * sizeof(glm::vec3), &mesh_vec[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, tex_vec.size() * sizeof(glm::vec2), &tex_vec[0], GL_STATIC_DRAW);

    GLuint texture;
    GLint texture_id;
    GLuint screen_texture;
    GLint screen_texture_id;
    if (!capture_flag) {
        texture = loadBMP_custom("../tex.bmp");
        texture_id = glGetUniformLocation(program_id, "myTextureSampler");
    } else if (capture_flag) {
        // create texture from ximage
        // get initial image
        image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);

        // create and bind new texture
        glGenTextures(0, &screen_texture);
        glBindTexture(GL_TEXTURE_2D, screen_texture);

        // specify 2D texture image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // get uniform texture location in fragment shader
        screen_texture_id = glGetUniformLocation(program_id, "myTextureSampler");
    }

    // main loop
    bool running = true;
    double last_time = glfwGetTime();
    int num_frames = 0;
    while (running && glfwWindowShouldClose(glfw_window) == 0) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /**
         * print render time per frame
         */
        // print ms per frame
        ++num_frames;
        double current_time = glfwGetTime();
        if (current_time - last_time >= 1.0) {
            std::cout << "ms/frame: " << (1000.0 / double(num_frames)) << std::endl;
            num_frames = 0;
            last_time += 1.0;
        }

        if (capture_flag) {
            // get screenshot
            image = XGetImage(display, root_window, 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);
            if (!image) {
                printf("Unable to create image...\n");
            }
        }

        // use shader
        glUseProgram(program_id);

        // send transformations to shader
        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

        /**
         * specify vertex arrays of vertices and uv's
         * draw finalyy
         */
        if (capture_flag) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                            image->data);

            glUniform1i(screen_texture_id, 0);
        } else if (!capture_flag) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(texture_id, 0);
        }

        // specify vertex arrays of vertices and uv's
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

        if (show_points) {
            glDrawArrays(GL_POINTS, 0, triangle_count * 3);
        } else if (!show_points) {
            glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);
        }

        // draw
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(glfw_window);
        glfwPollEvents();

        // important otherwise memory will be full soon
        if (capture_flag) {
            XDestroyImage(image);
        }

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


/**
 * load file from harddisk
 * @param filepath
 */
bool loadFile(const char *filepath, std::vector<glm::vec3> *to_fill) {

    std::ifstream f;
    std::string s;

    f.open(filepath, std::ios::in);

    if (f.is_open()) {

        std::cout << "Loading file: '" << filepath << "'!" << std::endl;
        while (!f.eof()) {

            getline(f, s);
            std::istringstream iss(s);

            float x, y, z;
            iss >> x >> y >> z;
            // std::cout << x << " " << y << " " << z << std::endl;

            // append to input vector
            to_fill->push_back(glm::vec3(x, y, z));
        }

        return true;
    } else {
        std::cout << "Error loading file: '" << filepath << "'!" << std::endl;
        return false;
    }
}

bool loadDomeMapFile(const char *filepath, std::vector<glm::vec3> *indexed_mask, std::vector<glm::vec3> *indexed_dome) {

    std::ifstream f;
    std::string s;

    f.open(filepath, std::ios::in);

    if (f.is_open()) {

        std::cout << "Loading file: '" << filepath << "'!" << std::endl;
        int idx = 0;
        while (!f.eof()) {

            getline(f, s);
            std::istringstream iss(s);

            float x1, y1, z1, x2, y2, z2;
            iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2;
            //std::cout << x << " " << y << " " << z << std::endl;

            // add to map
            indexed_mask->push_back(glm::vec3(x1, y1, z1));
            indexed_dome->push_back(glm::vec3(x2, y2, z2));
            idx++;
        }

        return true;
    } else {
        std::cout << "Error loading file: '" << filepath << "'!" << std::endl;
        return false;
    }
}

float mapToRange(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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


GLuint loadBMP_custom(const char *imagepath) {

    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char *data;

    // Open the file
    FILE *file = fopen(imagepath, "rb");
    if (!file) {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        getchar();
        return 0;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if (fread(header, 1, 54, file) != 54) {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    // A BMP files always begins with "BM"
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    // Make sure this is a 24bpp file
    if (*(int *) &(header[0x1E]) != 0) {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    if (*(int *) &(header[0x1C]) != 24) {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }

    // Read the information about the image
    dataPos = *(int *) &(header[0x0A]);
    imageSize = *(int *) &(header[0x22]);
    width = *(int *) &(header[0x12]);
    height = *(int *) &(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0) imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos == 0) dataPos = 54; // The BMP header is done that way

    // Create a buffer
    data = new unsigned char[imageSize];

    // Read the actual data from the file into the buffer
    fread(data, 1, imageSize, file);

    // Everything is in memory now, the file can be closed.
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete[] data;

    // Poor filtering, or ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // ... nice trilinear filtering ...
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;
}




