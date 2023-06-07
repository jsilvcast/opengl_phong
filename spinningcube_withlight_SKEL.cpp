// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// GLM library to deal with matrix operations
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "textfile/textfile_ALT.h"
#include "shapes/cube.h"
#include "shapes/tetrahedron.h"

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void render(double);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0; // Vertext Array Object to set input data
GLuint tetraVAO;

GLint model_location, view_location, proj_location; // Uniforms for transformation matrices
GLint normal_matrix_location; // Uniform for normal matrix

// Shader names
const char *vertexFileName = "../spinningcube_withlight_vs_SKEL.glsl";
const char *fragmentFileName = "../spinningcube_withlight_fs_SKEL.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up(0.0f, 1.0f, 0.0f);

// Lighting
glm::vec3 light_pos(0.0f, 0.0f, 0.0f);
glm::vec3 light_pos_2(-1.0f, 0.0f, 0.0f);
glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);

// Material
glm::vec3 material_ambient(1.0f, 0.5f, 0.31f);
glm::vec3 material_diffuse(1.0f, 0.5f, 0.31f);
glm::vec3 material_specular(0.5f, 0.5f, 0.5f);
const GLfloat material_shininess = 32.0f;

int main() {
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "My spinning cube", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeCallback(window, glfw_window_size_callback);
    glfwMakeContextCurrent(window);

    // start GLEW extension handler
    // glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* vendor = glGetString(GL_VENDOR); // get vendor string
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* glversion = glGetString(GL_VERSION); // version as a string
    const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION); // version as a string
    printf("Vendor: %s\n", vendor);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", glversion);
    printf("GLSL version supported %s\n", glslversion);
    printf("Starting viewport: (width: %d, height: %d)\n", gl_width, gl_height);

    // Enable Depth test: only draw onto a pixel if fragment closer to viewer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // set a smaller value as "closer"

    // Vertex Shader
    char* vertex_shader = textFileRead(vertexFileName);

    // Fragment Shader
    char* fragment_shader = textFileRead(fragmentFileName);

    // Shaders compilation
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    free(vertex_shader);
    glCompileShader(vs);

    int  success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        printf("ERROR: Vertex Shader compilation failed!\n%s\n", infoLog);

        return(1);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    free(fragment_shader);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        printf("ERROR: Fragment Shader compilation failed!\n%s\n", infoLog);

        return(1);
    }

    // Create program, attach shaders to it and link it
    shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);

    glValidateProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        printf("ERROR: Shader Program linking failed!\n%s\n", infoLog);

        return(1);
    }

    // Release shader objects
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Cube to be rendered
    //
    //          0        3
    //       7        4 <-- top-right-near
    // bottom
    // left
    // far ---> 1        2
    //       6        5
    //

    Cube cubeInstance;
    GLfloat* cube_vertex_positions_pointer = cubeInstance.getVertices();
    GLfloat cube_vertex_positions[108];
    std::copy(cube_vertex_positions_pointer, cube_vertex_positions_pointer + 108, cube_vertex_positions);

// Vertex Buffer Object (for vertex coordinates)
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_positions), cube_vertex_positions, GL_STATIC_DRAW);

    // Vertex attributes
    // 0: vertex position (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // 1: vertex normals (x, y, z)
    GLfloat* cube_vertex_normals_pointer = cubeInstance.getNormals();
    GLfloat cube_vertex_normals[108];
    std::copy(cube_vertex_normals_pointer, cube_vertex_normals_pointer + 108, cube_vertex_normals);

    // Vertex Buffer Object (for vertex normals)
    GLuint vbo_normals = 0;
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_normals), cube_vertex_normals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

//    // Unbind vbo (it was conveniently registered by VertexAttribPointer)
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//
//
//    // Unbind vao
//    glBindVertexArray(0);

    glGenVertexArrays(1, &tetraVAO);
    glBindVertexArray(tetraVAO);

    Tetrahedron tetrahedronInstance;
    GLfloat* tetrahedron_vertex_positions_pointer = tetrahedronInstance.getVertices();
    GLfloat tetrahedron_vertex_positions[36];
    std::copy(tetrahedron_vertex_positions_pointer, tetrahedron_vertex_positions_pointer + 36, tetrahedron_vertex_positions);

    // Vertex Buffer Object (for vertex coordinates)
    GLuint vbo_tetrahedron = 0;
    glGenBuffers(1, &vbo_tetrahedron);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tetrahedron);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_vertex_positions), tetrahedron_vertex_positions, GL_STATIC_DRAW);

    // Vertex attributes
    // 0: vertex position (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // 1: vertex normals (x, y, z)
    GLfloat* tetrahedron_vertex_normals_pointer = tetrahedronInstance.getNormals();
    GLfloat tetrahedron_vertex_normals[36];
    std::copy(tetrahedron_vertex_normals_pointer, tetrahedron_vertex_normals_pointer + 36, tetrahedron_vertex_normals);

    // Vertex Buffer Object (for vertex normals)
    GLuint vbo_normals_tetrahedron = 0;
    glGenBuffers(1, &vbo_normals_tetrahedron);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_tetrahedron);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_vertex_normals), tetrahedron_vertex_normals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

//    // Unbind vbo (it was conveniently registered by VertexAttribPointer)
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);

    // Uniforms
    // - Model matrix
    model_location = glGetUniformLocation(shader_program, "model");
    // - View matrix
    view_location = glGetUniformLocation(shader_program, "view");
    // - Projection matrix
    proj_location = glGetUniformLocation(shader_program, "projection");
    // - Normal matrix: normal vectors from local to world coordinates
    normal_matrix_location = glGetUniformLocation(shader_program, "normal_to_world");
    // - Camera position
    // - Light data
    // - Material data
    // [...]

// Render loop
    while(!glfwWindowShouldClose(window)) {

        processInput(window);

        render(glfwGetTime());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void render(double currentTime) {
    float f = (float)currentTime * 0.3f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, gl_width, gl_height);

    glUseProgram(shader_program);

    glm::mat4 model_matrix, view_matrix, proj_matrix;

    // Cube
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, glm::vec3(-0.5f, 0.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, f * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, f * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    view_matrix = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
    proj_matrix = glm::perspective(glm::radians(50.0f), (float)gl_width / (float)gl_height, 0.1f, 100.0f);

    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 108);

    // Tetrahedron
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, glm::vec3(0.5f, 0.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, f * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, f * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    glBindVertexArray(tetraVAO);
    glDrawArrays(GL_TRIANGLES, 0, 12);

    glUniform3fv(glGetUniformLocation(shader_program, "lights[0].position"), 1, glm::value_ptr(light_pos));
    glUniform3fv(glGetUniformLocation(shader_program, "lights[0].ambient"), 1, glm::value_ptr(light_ambient));
    glUniform3fv(glGetUniformLocation(shader_program, "lights[0].diffuse"), 1, glm::value_ptr(light_diffuse));
    glUniform3fv(glGetUniformLocation(shader_program, "lights[0].specular"), 1, glm::value_ptr(light_specular));

    glUniform3fv(glGetUniformLocation(shader_program, "lights[1].position"), 1, glm::value_ptr(light_pos_2));
    glUniform3fv(glGetUniformLocation(shader_program, "lights[1].ambient"), 1, glm::value_ptr(light_ambient));
    glUniform3fv(glGetUniformLocation(shader_program, "lights[1].diffuse"), 1, glm::value_ptr(light_diffuse));
    glUniform3fv(glGetUniformLocation(shader_program, "lights[1].specular"), 1, glm::value_ptr(light_specular));

    glUniform3fv(glGetUniformLocation(shader_program, "material.ambient"), 1, glm::value_ptr(material_ambient));
    glUniform3fv(glGetUniformLocation(shader_program, "material.diffuse"), 1, glm::value_ptr(material_diffuse));
    glUniform3fv(glGetUniformLocation(shader_program, "material.specular"), 1, glm::value_ptr(material_specular));
    glUniform1f(glGetUniformLocation(shader_program, "material.shininess"), material_shininess);

    // Moving cube
    // model_matrix = glm::rotate(model_matrix,
    //   [...]
    //
    // Projection
    // proj_matrix = glm::perspective(glm::radians(50.0f),
    //   [...]
    //
    // Normal matrix: normal vectors to world coordinates
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
    glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

    // Added view position to specular calculation
    glUniform3fv(glGetUniformLocation(shader_program, "view_pos"), 1, glm::value_ptr(camera_pos));


}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    gl_width = width;
    gl_height = height;
    printf("New viewport: (width: %d, height: %d)\n", width, height);
}
