//
//  main.m
//  Soft Renderer
//
//  Created by Aleksandar Angelov on 3/6/14.
//  Copyright (c) 2014 Aleksandar Angelov. All rights reserved.
//
#include "SDL2/SDL.h"
#include "Camera.h"
#include <iostream>

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

Uint32 *pixels;

void render()
{
    
    SDL_UpdateTexture(p_framebuffer, NULL, pixels, 640 * sizeof(Uint32));
    
    SDL_RenderClear(p_renderer);
    SDL_RenderCopy(p_renderer, p_framebuffer, NULL, NULL);
    SDL_RenderPresent(p_renderer);
}

int main(int argc, const char * argv[])
{
    SDL_Window *p_window;
    
    SDL_Init(SDL_INIT_VIDEO);
    
    p_window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    
    p_renderer = SDL_CreateRenderer(p_window, -1, 0);
    
    SDL_SetRenderDrawColor(p_renderer, 255, 0, 0, 255);

    p_framebuffer = SDL_CreateTexture(p_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 640, 480);
    
    while(running) {
        event_handle();
        render();
    }
    
    
    SDL_DestroyTexture(p_framebuffer);
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);
    
    SDL_Quit();
    
    return 0;
}
