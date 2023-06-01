package com.github.nodedev74.jfbx;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

public class HelloWorldJNITest {

    @Test
    public void testHelloWorldJNI() {
        NativeLoader.load("libhello");

        String cppOutput = new HelloWorldJNI().sayHello();
        String expectedOutput = "Hello from C++ !!";
        Assertions.assertEquals(expectedOutput, cppOutput);
    }

}
