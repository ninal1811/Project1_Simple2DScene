/*
* Author: Nina Li
* Assignment: Simple 2D Scene
* Date due: 2024-06-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
*/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH = 640,
              WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.1922f,
                BG_BLUE = 0.549f,
                BG_GREEN = 0.9059f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0f;

constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL = 0,
                TEXTURE_BORDER = 0;

//object img src
constexpr char NEMO_SPRITE_FILEPATH[] = "nemo.png",
               DORY_SPRITE_FILEPATH[] = "dory.png";

constexpr glm::vec3 INIT_SCALE_NEMO = glm::vec3(0.948f, 0.712f, 0.0f),
                    INIT_SCALE_DORY = glm::vec3(2.00f, 1.61f, 0.0f),
                    INIT_POS_NEMO = glm::vec3(-2.0f, 0.0f, 0.0f),
                    INIT_POS_DORY = glm::vec3(1.0f, 3.0f, 0.0f);

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix,
          g_nemo_matrix,
          g_dory_matrix,
          g_projection_matrix;

glm::vec3 g_rotation_nemo = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_dory = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint g_nemo_texture_id,
       g_dory_texture_id;

glm::vec3 nemo_position = INIT_POS_NEMO;
glm::vec3 dory_position = INIT_POS_DORY;

float g_previous_ticks = 0.0f;
float SPEED = 1.0f;
constexpr int SCALE_LIMIT = 15;
int g_frame_counter = 0;
bool g_ScaleDirection = true;
constexpr float GROWTH_FACTOR = 1.01f;
constexpr float SHRINK_FACTOR = 0.99f;

GLuint load_texture(const char* filepath) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
   
    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
   
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA,
                 width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}


void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Project 1: Simple 2D Scene",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
       
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
       
    g_nemo_matrix = glm::mat4(1.0f);
    g_dory_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
       
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
       
    glUseProgram(g_shader_program.get_program_id());
       
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
   
    g_nemo_texture_id = load_texture(NEMO_SPRITE_FILEPATH);
    g_dory_texture_id = load_texture(DORY_SPRITE_FILEPATH);
   
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            g_app_status = TERMINATED;
        }
    }
}

void update() {
    float ticks = SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
   
    float WAVE_OFFSET = glm::sin(ticks) * 0.5f;
   
//    game logic
    nemo_position.x += SPEED * delta_time;
    nemo_position.y += WAVE_OFFSET * delta_time;
    dory_position.y += WAVE_OFFSET * delta_time;
   
    if (nemo_position.x > 2.0f || nemo_position.x < -2.0f) {
        SPEED = -SPEED;
        g_rotation_nemo.y += glm::radians(180.0f);
    }
   
    g_nemo_matrix = glm::mat4(1.0f);
    g_dory_matrix = glm::mat4(1.0f);
   
//    transformation : rotation, translation, scaling
    g_nemo_matrix = glm::translate(g_nemo_matrix, nemo_position);
    g_nemo_matrix = glm::rotate(g_nemo_matrix, g_rotation_nemo.y, glm::vec3(0.0f, 1.0f, 0.0f));
    
    g_frame_counter++;
    if (g_frame_counter >= SCALE_LIMIT) {
        g_frame_counter = 0;
        g_ScaleDirection = !g_ScaleDirection;
    }
    
    glm::vec3 SCALE_FACTOR = glm::vec3(g_ScaleDirection ? GROWTH_FACTOR : SHRINK_FACTOR,
                                       g_ScaleDirection ? GROWTH_FACTOR : SHRINK_FACTOR,
                                       1.0f);
    
    g_nemo_matrix = glm::scale(g_nemo_matrix, SCALE_FACTOR);
   
    g_dory_matrix = glm::translate(g_nemo_matrix, dory_position);
    g_dory_matrix = glm::scale(g_dory_matrix, INIT_SCALE_DORY);

}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id) {
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
   
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };
   
//    texture
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
   
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
       
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_nemo_matrix, g_nemo_texture_id);
    draw_object(g_dory_matrix, g_dory_texture_id);
       
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
       
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

int main() {
    initialise();
   
    while (g_app_status == RUNNING) {
        process_input();
        update();
        render();
    }
   
    shutdown();
    return 0;
}


