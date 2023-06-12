package com.github.nodedev74.jfbx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

/**
 * The NativeLoader class provides methods for loading native libraries in Java.
 */
public class NativeLoader {

    /**
     * This method copies an internal native library into the Windows temporary
     * directory and links the native library with the JVM.
     *
     * @param libName The name of the library.
     * @throws Exception If an error occurs during the loading process.
     */
    public static void load(String libName) throws Exception {
        URL path = NativeLoader.class.getClassLoader().getResource("native/" + libName + ".dll");
        File tempFile = createTempFile(path);
        System.load(tempFile.getAbsolutePath());
    }

    /**
     * This method creates a temporary file based on the given URL path.
     *
     * @param path The URL of the native library.
     * @return The created temporary file.
     * @throws IOException If an I/O error occurs during file creation.
     */
    private static File createTempFile(URL path) throws IOException {
        File tempFile = File.createTempFile("lib", ".dll");
        writeTempFile(path, tempFile);
        return tempFile;
    }

    /**
     * This method writes the contents of the given URL to the specified temporary
     * file.
     *
     * @param path     The URL of the native library.
     * @param tempFile The temporary file to write to.
     * @throws IOException If an I/O error occurs during file writing.
     */
    private static void writeTempFile(URL path, File tempFile) throws IOException {
        try (OutputStream outputStream = new FileOutputStream(tempFile);
                InputStream inputStream = path.openStream()) {
            outputStream.write(inputStream.readAllBytes());
            outputStream.close();
            inputStream.close();
        }
    }
}
