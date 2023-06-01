package com.github.nodedev74.jfbx;

public class HelloJNI {
    static {
        System.loadLibrary("libhello"); // loads libhello.so
    }

    private native void sayHello(String name);

    public static void main(String[] args) {
        new HelloJNI().sayHello("Dave");
    }
}