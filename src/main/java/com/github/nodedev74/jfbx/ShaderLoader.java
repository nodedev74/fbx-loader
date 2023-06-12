package com.github.nodedev74.jfbx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

/**
 * The ShaderLoader class provides methods for loading shader files in Java.
 */
public class ShaderLoader {

    /**
     * Loads a shader file and returns the absolute path of the temporary file
     * created.
     *
     * @param shaderName The name of the shader file.
     * @return The absolute path of the loaded shader file.
     * @throws IOException If an I/O error occurs during the loading process.
     */
    public static String load(String shaderName) throws Exception {
        URL path = NativeLoader.class.getClassLoader().getResource("shaders/" + shaderName + ".spv");
        File tempFile = createTempFile(path);
        return tempFile.getAbsolutePath();
    }

    /**
     * Creates a temporary file based on the given URL path.
     *
     * @param path The URL of the shader file.
     * @return The created temporary file.
     * @throws IOException If an I/O error occurs during file creation.
     */
    private static File createTempFile(URL path) throws IOException {
        File tempFile = File.createTempFile("shdr", ".spv");
        writeTempFile(path, tempFile);
        return tempFile;
    }

    /**
     * Writes the contents of the given URL to the specified temporary file.
     *
     * @param path     The URL of the shader file.
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
