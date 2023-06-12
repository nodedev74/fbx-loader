#include "com_github_nodedev74_jfbx_vulkan_VkWindow.h"
#include <jni.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define APP_NAME "JFXB Viewer"

/**
 * @brief
 *
 * @param env
 * @param obj
 * @param width
 * @param height
 */
JNIEXPORT jlong JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_create(JNIEnv *env, jobject obj, jint width, jint height)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);

    SDL_Window *window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);

    jlong sdlWindowPtr = reinterpret_cast<jlong>(window);
    return sdlWindowPtr;
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_destroy(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);

    jfieldID sdlWindowPtrFieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, sdlWindowPtrFieldID);
    SDL_Window *window = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    jfieldID vkHandlerFieldID = env->GetFieldID(cls, "handler", "Lcom/github/nodedev74/jfbx/vulkan/VkHandler;");
    jobject vkHandlerObject = env->GetObjectField(obj, vkHandlerFieldID);
    jclass vkHandlerClass = env->GetObjectClass(vkHandlerObject);

    jmethodID dmethodID = env->GetMethodID(vkHandlerClass, "destroy", "()V");
    env->CallVoidMethod(vkHandlerObject, dmethodID);

    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    jmethodID methodID = env->GetMethodID(cls, "delete", "()V");
    env->CallVoidMethod(obj, methodID);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_show(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);

    jfieldID sdlWindowPtrFieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, sdlWindowPtrFieldID);
    SDL_Window *window = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    SDL_ShowWindow(window);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_hide(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);

    jfieldID sdlWindowPtrFieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, sdlWindowPtrFieldID);
    SDL_Window *window = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    SDL_HideWindow(window);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 * @param sdlWindowPtr
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_run(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            jclass cls = env->GetObjectClass(obj);
            jmethodID methodID = env->GetMethodID(cls, "destroy", "()V");

            env->CallVoidMethod(obj, methodID);
        }

        if (event.type == SDL_WINDOWEVENT)
        {
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
            {
                jclass cls = env->GetObjectClass(obj);
                jmethodID methodID = env->GetMethodID(cls, "destroy", "()V");

                env->CallVoidMethod(obj, methodID);
                break;
            }
            }
        }
    }
}