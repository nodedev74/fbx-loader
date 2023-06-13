/**
 * @file VkWindow.cpp
 * @author Lenard Büsing (nodedev74@gmail.com)
 * @brief Vulkan window based on SDLWindow.
 * @version 0.1
 * @date 2023-06-13
 *
 * @copyright Copyright (c) 2023 Lenard Büsing
 *
 */

#include "com_github_nodedev74_jfbx_vulkan_VkWindow.h"
#include <jni.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

/**
 * @brief JNI function to create a Vulkan window.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 * @param width The width of the window.
 * @param height The height of the window.
 * @return The pointer to the created SDLWindow as a jlong value.
 */
JNIEXPORT jlong JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkWindow_create(JNIEnv *env, jobject obj, jint width, jint height)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);

    SDL_Window *window = SDL_CreateWindow("JVulkan Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);

    jlong sdlWindowPtr = reinterpret_cast<jlong>(window);
    return sdlWindowPtr;
}

/**
 * @brief JNI function to destroy a Vulkan window.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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
 * @brief JNI function to show a Vulkan window.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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
 * @brief JNI function to hide a Vulkan window.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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
 * @brief JNI function run the lifecycle of the SDLWindow
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 * @param sdlWindowPtr The pointer to the SDLWindow as a jlong value.
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