#include "vulkan/VkWindow.hpp"

VkWindow::VkWindow(int _width, int _heigth) : width(_width), heigth(_heigth)
{
}

void VkWindow::createWindow()
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, heigth, SDL_WINDOW_HIDDEN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void VkWindow::show()
{
    SDL_ShowWindow(window);
}

void VkWindow::hide()
{
    SDL_HideWindow(window);
}

void VkWindow::renderWindow()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);
    SDL_Rect rect = {x, y, 200, 200};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderPresent(renderer);

    x += 1;
    y += 1;
}

void VkWindow::destroyWindow()
{
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;

    SDL_DestroyWindow(window);
    window = nullptr;

    SDL_Quit();
}

/*
    JNI
*/

extern "C" JNIEXPORT jlong JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_createSDLWindow(JNIEnv *env, jobject obj, jint width, jint height)
{
    VkWindow *sdlWindow = new VkWindow(width, height);
    sdlWindow->createWindow();
    return reinterpret_cast<jlong>(sdlWindow);
}

extern "C" JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_showSDLWindow(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    VkWindow *sdlWindow = reinterpret_cast<VkWindow *>(sdlWindowPtr);
    sdlWindow->show();
}

extern "C" JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_hideSDLWindow(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    VkWindow *sdlWindow = reinterpret_cast<VkWindow *>(sdlWindowPtr);
    sdlWindow->hide();
}

extern "C" JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_renderSDLWindow(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    VkWindow *sdlWindow = reinterpret_cast<VkWindow *>(sdlWindowPtr);
    sdlWindow->renderWindow();
}

extern "C" JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_destroySDLWindow(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    VkWindow *sdlWindow = reinterpret_cast<VkWindow *>(sdlWindowPtr);
    sdlWindow->destroyWindow();
    delete sdlWindow;
}

extern "C" JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_cycleSDLWindow(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    VkWindow *sdlWindow = reinterpret_cast<VkWindow *>(sdlWindowPtr);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            jclass javaClass = env->GetObjectClass(obj);
            jmethodID javaMethod = env->GetMethodID(javaClass, "close", "()V");
            env->CallVoidMethod(obj, javaMethod);
        }

        if (event.type == SDL_WINDOWEVENT)
        {
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                jclass javaClass = env->GetObjectClass(obj);
                jmethodID javaMethod = env->GetMethodID(javaClass, "close", "()V");
                env->CallVoidMethod(obj, javaMethod);
                break;
            }
        }
    }
}