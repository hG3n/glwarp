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

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>

// gl globals
GLFWwindow * glfw_window;
Display * display;
Window root_window;
XImage *image;

int SCREEN_WIDTH = (int) 1920;
int SCREEN_HEIGHT = (int) 1080;

bool VSYNC = false;
bool capture_flag = true;
bool show_polys = false;
bool paused = false;
bool show_points = false;
bool running = true;

float move_factor = 0.0001f;
float rotation_factor = 0.0001f;
glm::vec3 model_position(0.0f, 0.0f, 1.0f);
glm::vec3 model_rotation(0.0f, 0.0f, 0.00f);
glm::mat4 MVP;

// function callbacks
GLuint LoadShaders(const char *, const char *);
bool loadFile(const char *, std::vector<glm::vec3> *);
GLuint setup_vertices(const char *filepath, int *triangle_count);
GLuint setup_tex_coords(const char *filepath);
GLuint init_static_texture();
GLuint init_dynamic_texture(Display *dis, Window win, XImage *image);
GLuint loadBMP_custom(const char *);
float mapToRange(float value, float in_min, float in_max, float out_min, float out_max);
void mapVecToRange(std::vector<glm::vec3> *vec);
bool initializeGLContext(bool show_polys, bool with_vsync);


int main(void) {

    // get system arguments
    const char *texture_file = argv[1];
    bool show_polys = (bool) argv[2];

    initializeGLContext(false, false);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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
            glm::vec3(0.0, 0.0, 3.0), // camera pos world space
            glm::vec3(0.0, 0.0, 0), // camera lookat
            glm::vec3(0, 1, 0)  // up-vec
    );

    // model
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 scale = glm::scale(model, glm::vec3(16.0f / 9.0f, 1.0f, 1.0f));

    // MVP
    glm::mat4 MVP = projection * view * model * scale; // Remember, matrix multiplication is the other way around
//    glm::mat4 MVP = projection * view * model; //* scale; // Remember, matrix multiplication is the other way around

    int triangle_count = 0;
    GLuint vtx_buffer = setup_vertices("../new_screen_points.txt", &triangle_count);
    std::cout << "TRIANGLE COUNT: " << triangle_count << std::endl;

    GLuint tex_buffer = setup_tex_coords("../new_texture_coords.txt");

    GLuint tex;
    GLint tex_id;
    if (!capture_flag) {
        tex = init_static_texture();
        tex_id = glGetUniformLocation(program_id, "myTextureSampler");
    } else if (capture_flag) {
        tex = init_dynamic_texture(display, root_window, image);
        tex_id = glGetUniformLocation(program_id, "myTextureSampler");
    }

    // main loop
    bool running = true;
    bool paused = false;
    bool reload= false;
    double last_time = glfwGetTime();
    int num_frames = 0;
    while (running && glfwWindowShouldClose(glfw_window) == 0) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (!paused) {
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
                image = XGetImage(display, root_window, 420, 0, SCREEN_HEIGHT, SCREEN_HEIGHT, AllPlanes, ZPixmap);
                //image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);
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
             * draw finally
             */
            if (capture_flag) {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_HEIGHT, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                                image->data);
                //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                //                image->data);
                glUniform1i(tex_id, 0);
            } else if (!capture_flag) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex);
                glUniform1i(tex_id, 0);
            }

            // specify vertex arrays of vertices and uv's
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vtx_buffer);
            glVertexAttribPointer(
                    0,                  // must match shader layout
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void *) 0          // array buffer offset
            );

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
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

            // important otherwise memory will be full soon
            if (capture_flag) {
                XDestroyImage(image);
            }
        }

        // Swap buffers
        glfwSwapBuffers(glfw_window);
        glfwPollEvents();

        // check for keyboard input
        if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(glfw_window, GLFW_KEY_Q) == GLFW_PRESS) {
            running = false;
        }
        if (glfwGetKey(glfw_window, GLFW_KEY_R) == GLFW_PRESS){
            if (reload){
                reload = false;
                std::cout << "Reload" << std::endl;
            } else {
                reload = true;
                std::cout << "Reload" << std::endl;
            }
        }
        if (glfwGetKey(glfw_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (paused){
                std::cout << "Stop pause" << std::endl;
                paused = false;
            } else {
                std::cout << "Start pause" << std::endl;
                paused = true;
            }
        }
    }

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vtx_buffer);
    glDeleteBuffers(1, &tex_buffer);
    glDeleteProgram(program_id);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &vertex_array_id);

    XCloseDisplay(display);

    // Close OpenGL glfw_window and terminate GLFW
    glfwTerminate();

    return 0;
}

/**
 * Initialize GL context
 * @param show_polys
 * @param with_vsync
 * @return
 */
bool initializeGLContext(bool show_polys, bool with_vsync) {

    //get x11 client reference
    display = XOpenDisplay(nullptr);
    root_window = DefaultRootWindow(display);

    // Initialise GLFW
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
//    SCREEN_WIDTH = width;
    glfw_window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GLWarp", nullptr, nullptr);
    if (glfw_window == nullptr) {
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

    // init GL settings
    glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
    glEnable(GL_PROGRAM_POINT_SIZE);

    // set show polys flag to show vertice grid
    if (show_polys) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glfwSwapInterval(0);
    if (with_vsync) {
        glfwSwapInterval(1);
    }


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

float mapToRange(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void mapVecToRange(std::vector<glm::vec3> *vec) {
    float min_val_x = 999999;
    float max_val_x = -999999;
    float min_val_y = 999999;
    float max_val_y = -999999;
    for (auto item : *vec) {
        if (item.x < min_val_x) {
            min_val_x = item.x;
        }
        if (item.x > max_val_x && item.x < 1000.0f) {
            max_val_x = item.x;
        }
        if (item.y < min_val_y) {
            min_val_y = item.y;
        }
        if (item.y > max_val_y) {
            max_val_y = item.y;
        }
    }

    // map to dome grid to -1.0 to 1.0
    for (int i = 0; i < vec->size(); ++i) {
        float x, y, z;
        x = mapToRange(vec->at(i).x, min_val_x, max_val_x, -1.0f, 1.0f);
        y = mapToRange(vec->at(i).y, min_val_y, max_val_y, -1.0f, 1.0f);
        vec->at(i) = glm::vec3(x, y, vec->at(i).z);
    }

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

GLuint setup_vertices(const char *filepath, int *triangle_count){
    std::vector<glm::vec3> mesh;

    // setup mesh
    loadFile(filepath, &mesh);
    // get meta information about calculated
    auto circle_count = (int) mesh.back().x;
    auto points_per_circle = (int) mesh.back().y;
    auto point_count = (int) mesh.back().z;
    std::cout << "circle count: " << circle_count << " points_per_circle: " << points_per_circle << std::endl;

    mesh.pop_back();
    mesh.pop_back();
    //mapVecToRange(&mesh);
    std::cout << "blue size: " << mesh.size() << " read point count: " << point_count << std::endl;
    if (mesh.size() != point_count) {
        std::cout << "Warp points do not match" << std::endl;
        return -1;
    }

    // IF HAGEN SWITCHES HIS ZERO COORDS AGAIN JUST SLAP HIM IN THE FACE
    // for (int i = 0; i < mesh.size(); ++i) {
    //    mesh[i].y = mesh[i].z;
    //    mesh[i].z = 0.0f;
    // }

    std::vector<glm::vec3> mesh_vec;
    // create mesh
    for (int circle_idx = 0; circle_idx < circle_count; ++circle_idx) {
        if (circle_idx == 0) {
            for (int t = 1; t < points_per_circle + 1; ++t) {
                // start triangles around center
                //std::cout << 0 << " " << t << " " << 1 + (t % points_per_circle) << std::endl;

                int i1 = 0;
                int i2 = t;
                int i3 = 1 + (t % points_per_circle);
                mesh_vec.push_back(mesh[i1]);
                mesh_vec.push_back(mesh[i2]);
                mesh_vec.push_back(mesh[i3]);
                *triangle_count += 1;
            }
        } else {
            int start_point = circle_idx * points_per_circle - (points_per_circle - 1);
            for (int idx = 0; idx < points_per_circle; ++idx) {
                int i1 = start_point + idx;
                int i2 = start_point + idx + points_per_circle;
                int i3 = start_point + (idx + 1) % points_per_circle;
                mesh_vec.push_back(mesh[i1]);
                mesh_vec.push_back(mesh[i2]);
                mesh_vec.push_back(mesh[i3]);
                //std::cout << i1<< " " << i2 << " " << i3 << std::endl;
                int i4 = start_point + (idx + 1) % points_per_circle;
                int i5 = start_point + idx + points_per_circle;
                int i6 = start_point + ((idx + 1) % points_per_circle) + points_per_circle;
                mesh_vec.push_back(mesh[i4]);
                mesh_vec.push_back(mesh[i5]);
                mesh_vec.push_back(mesh[i6]);
                //std::cout << i4 << " " << i5<< " " << i6 << std::endl;
                *triangle_count += 2;
            }
        }
    }

    // create buffers
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, mesh_vec.size() * sizeof(glm::vec3), &mesh_vec[0], GL_STATIC_DRAW);
    return vertex_buffer;


}


GLuint setup_tex_coords(const char *filepath){

    std::vector<glm::vec3> uv_coords;
    // setup mesh
    loadFile(filepath, &uv_coords);
    // get meta information about calculated
    auto circle_count = (int) uv_coords.back().x;
    auto points_per_circle = (int) uv_coords.back().y;
    auto point_count = (int) uv_coords.back().z;
    uv_coords.pop_back();
    uv_coords.pop_back();

    // IF HAGEN SWITCHES HIS ZERO COORDS AGAIN JUST SLAP HIM IN THE FACE
    // for (int i = 0; i < uv_coords.size(); ++i) {
    //     uv_coords[i].y = uv_coords[i].z;
    //     uv_coords[i].z = 0.0f;
    // }

    std::cout << "red size: " << uv_coords.size() << " read point count: " << point_count << std::endl;
    //mapVecToRange(&uv_coords);
    std::vector<glm::vec2> tex_vec;
    for (int circle_idx = 0; circle_idx < circle_count; ++circle_idx) {
        if (circle_idx == 0) {
            for (int t = 1; t < points_per_circle + 1; ++t) {
                // start triangles around center
                //std::cout << 0 << " " << t << " " << 1 + (t % points_per_circle) << std::endl;
                int i1 = 0;
                int i2 = t;
                int i3 = 1 + (t % points_per_circle);

                //float u = mapToRange(uv_coords[i1].x, min_pos_x, max_pos_x, 0.0f, 1.0f);
                float u = uv_coords[i1].x;
                float v = uv_coords[i1].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                u = uv_coords[i2].x;
                v = uv_coords[i2].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                u = uv_coords[i3].x;
                v = uv_coords[i3].y;
                tex_vec.emplace_back(glm::vec2(u, v));
            }
        } else {
            int start_point = circle_idx * points_per_circle - (points_per_circle - 1);
            for (int idx = 0; idx < points_per_circle; ++idx) {
                int i1 = start_point + idx;
                int i2 = start_point + idx + points_per_circle;
                int i3 = start_point + (idx + 1) % points_per_circle;
                float u = uv_coords[i1].x;
                float v = uv_coords[i1].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                u = uv_coords[i2].x;
                v = uv_coords[i2].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                u = uv_coords[i3].x;
                v = uv_coords[i3].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                //std::cout << i1<< " " << i2 << " " << i3 << std::endl;
                int i4 = start_point + (idx + 1) % points_per_circle;
                int i5 = start_point + idx + points_per_circle;
                int i6 = start_point + ((idx + 1) % points_per_circle) + points_per_circle;
                u = uv_coords[i4].x;
                v = uv_coords[i4].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                u = uv_coords[i5].x;
                v = uv_coords[i5].y;
                tex_vec.emplace_back(glm::vec2(u, v));

                u = uv_coords[i6].x;
                v = uv_coords[i6].y;
                tex_vec.emplace_back(glm::vec2(u, v));
                //std::cout << i4 << " " << i5<< " " << i6 << std::endl;
            }
        }
    }

    GLuint uv_buffer;
    glGenBuffers(1, &uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, tex_vec.size() * sizeof(glm::vec2), &tex_vec[0], GL_STATIC_DRAW);
    return uv_buffer;
}

GLuint init_static_texture(){
    // CREATE AND INIT STATIC TEXTURE FROM BMP
    GLuint static_tex;
    //texture = loadBMP_custom("../dome_coords.bmp");
    //texture = loadBMP_custom("../polar.bmp");
    //texture = loadBMP_custom("../polar_coords.bmp");
    static_tex = loadBMP_custom("../pol_coords.bmp");
    // texture = loadBMP_custom("../game_scene.bmp");
    //texture = loadBMP_custom("../tex.bmp");
    //texture = loadBMP_custom("../gradient2.bmp");
    return static_tex;
}


GLuint init_dynamic_texture(Display *dis, Window win, XImage *image){
    // CREATE AND INIT DYNAMIC TEXTURE FROM SCREEN
    GLuint dynamic_tex;
    // get initial image
    image = XGetImage(dis, win, 420, 0, SCREEN_HEIGHT, SCREEN_HEIGHT, AllPlanes, ZPixmap);
    //image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);

    // create and bind new texture
    glGenTextures(0, &dynamic_tex);
    glBindTexture(GL_TEXTURE_2D, dynamic_tex);

    // specify 2D texture image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_HEIGHT, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return dynamic_tex;
}
