#include "com_github_nodedev74_jfbx_HelloWorldJNI.h"

#include <iostream>
#include <sstream>

JNIEXPORT jstring JNICALL Java_com_github_nodedev74_jfbx_HelloWorldJNI_sayHello(JNIEnv *env, jobject thisObject)
{
  std::ostringstream oss;
  oss << "Hello from C++ !!";
  std::string output = oss.str();
  return env->NewStringUTF(output.c_str());
}
