package com.github.nodedev74.jfbx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;

public class ShaderLoader {
    public static String load(String shaderName) {
        URL path = NativeLoader.class.getClassLoader().getResource("shaders/" + shaderName + ".spv");
        File tempFile = null;
        try {
            tempFile = createTempFile(path);
        } catch (IOException e) {
            System.err.println("Fehler beim Erstellen der temporären Datei: " + e.getMessage());
        }
        return tempFile.getAbsolutePath();
    }

    private static File createTempFile(URL path) throws IOException {
        File tempFile = File.createTempFile("shdr", ".spv");
        writeTempFile(path, tempFile);
        return tempFile;
    }

    private static void writeTempFile(URL path, File tempFile) throws IOException {
        try (OutputStream outputStream = new FileOutputStream(tempFile);
                InputStream inputStream = path.openStream()) {
            byte[] allBytes = inputStream.readAllBytes();
            outputStream.write(allBytes);

            outputStream.close();
            inputStream.close();
        }
    }

}
