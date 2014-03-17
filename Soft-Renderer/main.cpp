#include "SDL2/SDL.h"
#include <iostream>
#include "device.h"
#include "camera.h"
#include "mesh.h"
#include "glm/ext.hpp"

bool running = true;

SDL_Texture *p_framebuffer;

SDL_Renderer *p_renderer;

void event_handle()
{
    SDL_Event event;
    while (SDL_PollEvent( &event )) {
        switch ( event.type ) {
            case SDL_KEYUP:
                if ( event.key.keysym.sym == SDLK_ESCAPE )
                    running = false;
                break;
        }
    }
}

void render(const SoftEngine::Device& device)
{
    SDL_UpdateTexture(p_framebuffer, NULL, device.backBuffer(), 640 * sizeof(Uint8) * 4);

    SDL_RenderClear(p_renderer);
    SDL_RenderCopy(p_renderer, p_framebuffer, NULL, NULL);
    SDL_RenderPresent(p_renderer);
}

int main()
{
    SDL_Window *p_window;

    SDL_Init(SDL_INIT_VIDEO);

    p_window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);

    p_renderer = SDL_CreateRenderer(p_window, -1, 0);

    SDL_SetRenderDrawColor(p_renderer, 255, 0, 0, 255);

    p_framebuffer = SDL_CreateTexture(p_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 640, 480);

    SoftEngine::Mesh mesh("Cube", 8);
    mesh.vertices()[0] = glm::vec3(-1, 1, 1);
    mesh.vertices()[1] = glm::vec3(1, 1, 1);
    mesh.vertices()[2] = glm::vec3(-1, -1, 1);
    mesh.vertices()[3] = glm::vec3(-1, -1, -1);
    mesh.vertices()[4] = glm::vec3(-1, 1, -1);
    mesh.vertices()[5] = glm::vec3(1, 1, -1);
    mesh.vertices()[6] = glm::vec3(1, -1, 1);
    mesh.vertices()[7] = glm::vec3(1, -1, -1);

    SoftEngine::Camera camera;
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f));

    SoftEngine::Device device(640, 480);

    std::vector<SoftEngine::Mesh> meshes;
    meshes.push_back(mesh);

    while(running) {
        event_handle();
        device.clear(glm::vec4(0, 0, 0, 255));
        mesh.setRotation(glm::vec3(mesh.rotation().x + 0.01f, mesh.rotation().y + 0.01f, mesh.rotation().z));
        device.render(camera, meshes);
        render(device);
    }

    SDL_DestroyTexture(p_framebuffer);
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);

    SDL_Quit();

    return 0;
}
