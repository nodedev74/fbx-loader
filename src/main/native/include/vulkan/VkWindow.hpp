#ifndef VK_WINDOW_HPP
#define VK_WINDOW_HPP

#include "com_github_nodedev74_jfbx_vulkan_VkWindow.h"

#include <jni.h>

#include <SDL2/SDL.h>

class VkWindow
{
private:
    int width;
    int heigth;

    int x = 0;
    int y = 0;

    SDL_Window *window;
    SDL_Renderer *renderer;

public:
    VkWindow(int width, int height);

public:
    void createWindow();
    void show();
    void hide();
    void renderWindow();
    void destroyWindow();
};

#endif // !VK_WINDOW_HPP