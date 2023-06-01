#include "com_github_nodedev74_jfbx_HelloWorldJNI.h"

#include <iostream>

JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_HelloWorldJNI_sayHello(JNIEnv *env, jobject thisObject)
{
  std::cout << "Hello from C++ !!" << std::endl;
}
