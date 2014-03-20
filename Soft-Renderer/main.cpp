#include "SDL2/SDL.h"
#include <iostream>
#include "device.h"
#include "camera.h"
#include "mesh.h"
#include "color.h"

const int WIDTH = 640;
const int HEIGHT = 400;
const int FPS = 30;
const int SECONDS_PER_FRAME = 1000 / FPS;

//#define LOGGER

class LTimer
{
private:
    Uint32 m_startTicks;
    Uint32 m_pausedTicks;

    bool m_paused;
    bool m_started;

public:
    LTimer()
        :m_startTicks(0), m_pausedTicks(0), m_paused(false), m_started(false)
    {}

    void start()
    {
        m_started = true;
        m_paused = false;
        m_startTicks = SDL_GetTicks();
        m_pausedTicks = 0;
    }

    void stop()
    {
        m_started = false;
        m_paused = false;
        m_startTicks = 0;
        m_pausedTicks = 0;
    }

    void pause()
    {
        if(m_started && !m_paused) {
            m_paused = true;
            m_pausedTicks = SDL_GetTicks() - m_startTicks;
            m_startTicks = 0;
        }
    }

    void unpause()
    {
        if(m_started && m_paused) {
            m_paused = false;
            m_startTicks = SDL_GetTicks() - m_pausedTicks;
            m_pausedTicks = 0;
        }
    }

    Uint32 getTicks()
    {
        Uint32 time = 0;
        if(m_started) {
            if(m_paused) {
                time = m_pausedTicks;
            } else {
                time = SDL_GetTicks() - m_startTicks;
            }
        }
        return time;
    }

    bool isStarted() { return m_started; }
    bool isPaused() { return m_paused; }
};

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
    SDL_UpdateTexture(p_framebuffer, NULL, device.backBuffer(), WIDTH * sizeof(Uint32));

    SDL_RenderClear(p_renderer);
    SDL_RenderCopy(p_renderer, p_framebuffer, NULL, NULL);
    SDL_RenderPresent(p_renderer);
}

#define UNUSED(x) (void)(x);
int main(int argc, char* argv[])
{
    UNUSED(argc)
    UNUSED(argv)
    SDL_Window *p_window;

    SDL_Init(SDL_INIT_VIDEO);

    p_window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    p_renderer = SDL_CreateRenderer(p_window, -1, 0);

    SDL_SetRenderDrawColor(p_renderer, 255, 0, 0, 255);

    p_framebuffer = SDL_CreateTexture(p_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    std::vector<SoftEngine::Mesh> meshes;
//    meshes.push_back(SoftEngine::Mesh("Cube", 8, 12));

//    SoftEngine::Mesh& mesh = meshes[0];
//    mesh.vertices()[0] = glm::vec3(-1, 1, 1);
//    mesh.vertices()[1] = glm::vec3(1, 1, 1);
//    mesh.vertices()[2] = glm::vec3(-1, -1, 1);
//    mesh.vertices()[3] = glm::vec3(1, -1, 1);
//    mesh.vertices()[4] = glm::vec3(-1, 1, -1);
//    mesh.vertices()[5] = glm::vec3(1, 1, -1);
//    mesh.vertices()[6] = glm::vec3(1, -1, -1);
//    mesh.vertices()[7] = glm::vec3(-1, -1, -1);
//    mesh.faces()[0] = SoftEngine::Face{ 0, 1, 2 };
//    mesh.faces()[1] = SoftEngine::Face{ 1, 2, 3 };
//    mesh.faces()[2] = SoftEngine::Face{ 1, 3, 6 };
//    mesh.faces()[3] = SoftEngine::Face{ 1, 5, 6 };
//    mesh.faces()[4] = SoftEngine::Face{ 0, 1, 4 };
//    mesh.faces()[5] = SoftEngine::Face{ 1, 4, 5 };
//    mesh.faces()[6] = SoftEngine::Face{ 2, 3, 7 };
//    mesh.faces()[7] = SoftEngine::Face{ 3, 6, 7 };
//    mesh.faces()[8] = SoftEngine::Face{ 0, 2, 7 };
//    mesh.faces()[9] = SoftEngine::Face{ 0, 4, 7 };
//    mesh.faces()[10] = SoftEngine::Face{ 4, 5, 6 };
//    mesh.faces()[11] = SoftEngine::Face{ 4, 6, 7 };

    SoftEngine::Camera camera;
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f));

    SoftEngine::Device device(WIDTH, HEIGHT);

    device.loadJSONFile("../monkey.babylon", meshes);

    LTimer fpsTimer, capTimer;
    int countedFrames = 0;
    fpsTimer.start();

#ifdef LOGGER
    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = lastTime;
#endif

    while(running) {
#ifdef LOGGER
        currentTime = SDL_GetTicks();
        std::cout << "Last Frame took " << currentTime - lastTime << " milliseconds" << std::endl;
        lastTime = currentTime;
#endif
        capTimer.start();
        event_handle();
        float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.0f);
        if(avgFPS > 2000000)
            avgFPS = 0;

        device.clear(SoftEngine::Color::Black);

        for(SoftEngine::Mesh& mesh : meshes)
            mesh.setRotation(glm::vec3(mesh.rotation().x + 0.01f, mesh.rotation().y + 0.01f, mesh.rotation().z));

        device.render(camera, meshes);
        render(device);

        ++countedFrames;
        int frameTicks = capTimer.getTicks();
        if(frameTicks < SECONDS_PER_FRAME)
            SDL_Delay(SECONDS_PER_FRAME - frameTicks);
    }

    SDL_DestroyTexture(p_framebuffer);
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);

    SDL_Quit();

    return 0;
}
