package com.github.nodedev74.jfbx;

public class HelloWorldJNI {

    static {
        System.loadLibrary("libhello");
    }
    
    public static void main(String[] args) {
        new HelloWorldJNI().sayHello();
    }

    // Declare a native method sayHello() that receives no arguments and returns void
    private native void sayHello();
}