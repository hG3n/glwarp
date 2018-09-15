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

#include "inc/shader.h"
#include "inc/json11.hpp"
#include "inc/input_parser.h"
#include "inc/texture.h"

// gl globals
GLFWwindow *glfw_window;
Display *display;
Window root_window;
XImage *image;

int SCREEN_WIDTH = (int) 1200;
int SCREEN_HEIGHT = (int) 1000;

bool VSYNC = false;
#if 0
bool capture_flag = false;
#else
bool capture_flag = true;
#endif

bool show_points = false;
bool show_polys = false;
bool paused = false;
bool running = true;
bool print_fps = true;

int triangle_count;
GLuint vtx_buffer;
GLuint tex_buffer;

float move_factor = 0.0001f;
float rotation_factor = 0.0001f;
glm::vec3 model_position(0.0, -0.65, 0.5);
glm::vec3 model_rotation(0.0f, 0.0f, 0.00f);
glm::mat4 MVP;

json11::Json config;
std::string mesh_file;
std::string tex_file;

void print_help();

void parseCommandLineArgs(int argc, char *argv[]);

GLuint init_dynamic_texture(Display *dis, Window win, XImage *image);

void loadTransformationValues();

void calculateView(glm::vec3, glm::vec3);

bool loadFile(const char *, std::vector<glm::vec3> *);

GLuint setup_vertices(const char *filepath, int *triangle_count);

GLuint setup_tex_coords(const char *filepath);

float mapToRange(float value, float in_min, float in_max, float out_min, float out_max);

bool initializeGLContext(bool show_polys, bool with_vsync);

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

void handleFramewiseKeyInput();

bool loadConfig(const std::string &config);

void parseConfig();

int main(int argc, char *argv[])
{
    print_help();
    parseCommandLineArgs(argc, argv);
    parseConfig();

    initializeGLContext(false, false);

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    // load shaders
    GLuint program_id = Shader::loadShaders("shader/simple.vert", "shader/simple.frag");
    GLint matrix_id = glGetUniformLocation(program_id, "MVP");

    GLuint tex;
    if (capture_flag)
        tex = init_dynamic_texture(display, root_window, image);
    else
        tex = Texture::loadBMP("tex/radialgrid.bmp");

    GLint tex_id = glGetUniformLocation(program_id, "myTextureSampler");

    calculateView(model_position, model_rotation);
    loadTransformationValues();

    // main loop
    double last_time = glfwGetTime();
    int num_frames = 0;
    while (running && glfwWindowShouldClose(glfw_window) == 0) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (!paused) {

            ///print render time per frame
            if (print_fps) {
                ++num_frames;
                double current_time = glfwGetTime();
                if (current_time - last_time >= 1.0) {
                    std::cout << "ms/frame: " << (1000.0 / double(num_frames)) << std::endl;
                    num_frames = 0;
                    last_time += 1.0;
                }
            }

            /// capture if set true
            if (capture_flag) {
                // get screenshot
                image = XGetImage(display, root_window, 420, 0, SCREEN_HEIGHT, SCREEN_HEIGHT, AllPlanes, ZPixmap);
                //image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);
                if (!image)
                    printf("Unable to create image...\n");
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
                glUniform1i(tex_id, 0);
            } else if (!capture_flag) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex);
                glUniform1i(tex_id, 0);
            }

            // specify vertex arrays of vertices and uv's
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vtx_buffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            if (show_points)
                glDrawArrays(GL_POINTS, 0, triangle_count * 3);
            else if (!show_points)
                glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);


            // draw
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);

            // important otherwise memory will be full soon
            if (capture_flag) {
                XDestroyImage(image);
            }

            // Swap buffers
            glfwSwapBuffers(glfw_window);
            glfwPollEvents();

            handleFramewiseKeyInput();
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

void print_help()
{
    std::cout << "GLWarp 1.0b" << std::endl;
    std::cout << "==========" << std::endl;
    std::cout << std::endl;

    std::cout << "Controls:" << std::endl;
    std::cout << "  general:" << std::endl;
    std::cout << "    esc - exit glwarp << std::endl;" << std::endl;
    std::cout << "    r - reload transformation settings" << std::endl;
    std::cout << "    i - print mesh position and rotation information" << std::endl;
    std::cout << "    x - reset mesh position and rotation" << std::endl;
    std::cout << "    f - activate continuous fps output" << std::endl;
    std::cout << "  mesh:" << std::endl;
    std::cout << "    w - increase distance to mesh" << std::endl;
    std::cout << "    s - decrease distance to mesh" << std::endl;
    std::cout << "    a - move mesh to the left" << std::endl;
    std::cout << "    d - move mesh to the right" << std::endl;
    std::cout << "    j - move mesh up" << std::endl;
    std::cout << "    k - move mesh down" << std::endl;
    std::cout << "  movement settings:" << std::endl;
    std::cout << "    1 - decrease movement factor" << std::endl;
    std::cout << "    2 - increase movement factor" << std::endl;
    std::cout << "  rotation settings:" << std::endl;
    std::cout << "    3 - decrease rotation factor" << std::endl;
    std::cout << "    4 - increase rotation factor" << std::endl;
    std::cout << "" << std::endl;
}

void parseCommandLineArgs(int argc, char *argv[])
{
    InputParser input_parser(argc, argv);
    if (input_parser.cmdOptionExists("-capture")) {
        std::string opt = input_parser.getCmdOption("-capture");
        if (opt == "true")
            capture_flag = true;
        else
            capture_flag = false;
    } else {
        capture_flag = false;
    }

    if (input_parser.cmdOptionExists("-config")) {
        std::string opt = input_parser.getCmdOption("-config");
        if (opt != "") {
            if (!loadConfig(opt)) {
                std::cout << "Info: There was no config file specified. Loading defaults!" << std::endl;
                loadConfig("default/model.json");
            }
        } else {
            std::cout << "Info: There was no config file specified. Loading defaults!" << std::endl;
            loadConfig("default/model.json");
        }
    } else {
        loadConfig("default/model.json");
    }

    if (input_parser.cmdOptionExists("-mesh")) {
        std::string opt = input_parser.getCmdOption("-mesh");
        if (opt != "") {
            mesh_file = opt;
        } else {
            std::cout << "Info: There was no mesh file specified. Loading default!" << std::endl;
            mesh_file = "default/default.mesh";
        }
    } else {
        mesh_file = "default/default.mesh";
    }

    if (input_parser.cmdOptionExists("-tex")) {
        std::string opt = input_parser.getCmdOption("-tex");
        if (opt != "") {
            tex_file = opt;
        } else {
            std::cout << "Info: There was no mesh file specified. Loading default!" << std::endl;
            tex_file = "default/default.tex";
        }
    } else {
        tex_file = "default/default.tex";
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        running = false;

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        loadTransformationValues();

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        move_factor /= 10;

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        move_factor *= 10;

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        rotation_factor /= 10;

    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
        rotation_factor *= 10;

    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        std::cout << "INFO: model position: " << model_position.x << " " << model_position.y << " " << model_position.z
                  << std::endl;
        std::cout << "INFO: model rotation: " << model_rotation.x << " " << model_rotation.y << " " << model_rotation.z
                  << std::endl;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (print_fps)
            print_fps = false;
        else
            print_fps = true;
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        model_position = glm::vec3(0.0f, 0.0f, 0.0f);
        model_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        calculateView(model_position, model_rotation);
    }


    if (glfwGetKey(glfw_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (paused) {
            std::cout << "Stop pause" << std::endl;
            paused = false;
        } else {
            std::cout << "Start pause" << std::endl;
            paused = true;
        }
    }
}

void handleFramewiseKeyInput()
{
    if (glfwGetKey(glfw_window, GLFW_KEY_W) == GLFW_PRESS) {
        model_position.z -= move_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_S) == GLFW_PRESS) {
        model_position.z += move_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_A) == GLFW_PRESS) {
        model_position.x -= move_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_D) == GLFW_PRESS) {
        model_position.x += move_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_J) == GLFW_PRESS) {
        model_position.y -= move_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_K) == GLFW_PRESS) {
        model_position.y += move_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_H) == GLFW_PRESS) {
        model_rotation.x += rotation_factor;
        calculateView(model_position, model_rotation);
    }

    if (glfwGetKey(glfw_window, GLFW_KEY_L) == GLFW_PRESS) {
        model_rotation.x -= rotation_factor;
        calculateView(model_position, model_rotation);
    }
}

void loadTransformationValues()
{
    triangle_count = 0;

    std::cout << "FAADADA" << mesh_file << std::endl;
    vtx_buffer = setup_vertices(mesh_file.c_str(), &triangle_count);
    tex_buffer = setup_tex_coords(tex_file.c_str());
}

void calculateView(glm::vec3 model_pos, glm::vec3 model_rot)
{
    // projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

    // camera matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(0.0, 0.0, 0.0), // camera pos world space
            glm::vec3(0.0, 0.0, 100.0), // camera lookat
            glm::vec3(0, 1, 0)  // up-vec
    );

    // model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, model_pos);
    model = glm::translate(model, glm::vec3(-model_pos));
    model = glm::rotate(model, model_rot.x, glm::vec3(1, 0, 0));
    model = glm::translate(model, glm::vec3(model_pos));

    // build mvp
    MVP = projection * view * model;
}

/**
 * Initialize GL context
 * @param show_polys
 * @param with_vsync
 * @return
 */
bool initializeGLContext(bool show_polys, bool with_vsync)
{
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
    SCREEN_WIDTH = mode->width;
    SCREEN_HEIGHT = mode->height;

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

    // input settings
    glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(glfw_window, keyCallback);

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

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

/**
 * load file from harddisk
 * @param filepath
 */
bool loadFile(const char *filepath, std::vector<glm::vec3> *to_fill)
{
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
    }

    std::cout << "Error loading file: '" << filepath << "'!" << std::endl;
    return false;
}

float mapToRange(float value, float in_min, float in_max, float out_min, float out_max)
{
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void mapVecToRange(std::vector<glm::vec3> *vec)
{
    float min_val_x = 999999;
    float max_val_x = -999999;
    float min_val_y = 999999;
    float max_val_y = -999999;
    for (auto item : *vec) {
        if (item.x < min_val_x)
            min_val_x = item.x;
        if (item.x > max_val_x && item.x < 1000.0f)
            max_val_x = item.x;
        if (item.y < min_val_y)
            min_val_y = item.y;
        if (item.y > max_val_y)
            max_val_y = item.y;
    }

    // map to dome grid to -1.0 to 1.0
    for (int i = 0; i < vec->size(); ++i) {
        float x, y, z;
        x = mapToRange(vec->at(i).x, min_val_x, max_val_x, -1.0f, 1.0f);
        y = mapToRange(vec->at(i).y, min_val_y, max_val_y, -1.0f, 1.0f);
        vec->at(i) = glm::vec3(x, y, vec->at(i).z);
    }

}

GLuint setup_vertices(const char *filepath, int *triangle_count)
{
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

GLuint setup_tex_coords(const char *filepath)
{

    std::vector<glm::vec3> uv_coords;
    // setup mesh
    loadFile(filepath, &uv_coords);
    // get meta information about calculated
    auto circle_count = (int) uv_coords.back().x;
    auto points_per_circle = (int) uv_coords.back().y;
    auto point_count = (int) uv_coords.back().z;
    uv_coords.pop_back();
    uv_coords.pop_back();

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

GLuint init_dynamic_texture(Display *dis, Window win, XImage *image)
{
    // CREATE AND INIT DYNAMIC TEXTURE FROM SCREEN
    GLuint dynamic_tex;
    // get initial image
//    image = XGetImage(dis, win, 420, 0, SCREEN_HEIGHT, SCREEN_HEIGHT, AllPlanes, ZPixmap);
    std::cout << "working" << std::endl;
    image = XGetImage(display, root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, AllPlanes, ZPixmap);

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

bool loadConfig(const std::string &file_name)
{
    std::ifstream ifs(file_name);
    if (ifs.good()) {
        std::cout << "Loaded file '" << file_name << "' successfully" << std::endl;
    } else {
        std::cout << "Config: '" << file_name << "' not found!" << std::endl;
        return false;
    }

    // parse to string
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::string error;
    auto json = json11::Json::parse(str, error);

    if (error.empty()) {
        std::cout << "Json parsing succeeded!" << std::endl;
    } else {
        std::cout << "Error loading json: " << error << std::endl;
        return false;
    }

    config = json.object_items();

    return true;
}

void parseConfig()
{
    std::cout << config.dump() << std::endl;
    float projector_x = (float) config["projector"]["position"]["x"].number_value();
    float projector_y = (float) config["projector"]["position"]["y"].number_value();
    float projector_z = (float) config["projector"]["position"]["z"].number_value();

    std::cout << "x: " << (-1 * projector_x) << " y: " << (-1 * projector_y) << " z: " << (-1 * projector_z)
              << std::endl;

    glm::vec3 projector_position(-1* projector_x, -1* projector_y, -1* projector_z);
    model_position = projector_position;
}

