package com.github.nodedev74.jfbx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.nio.file.Path;

public class NativeLoader {

    public static void load(String libName) {
        URL path = NativeLoader.class.getClassLoader().getResource("native/" + libName + ".dll");
        File tempFile;
        try {
            tempFile = createTempFile(path);
        } catch (IOException e) {
            System.err.println("Fehler beim Erstellen der temporären Datei: " + e.getMessage());
            return;
        }
        loadLibrary(tempFile);
    }

    private static File createTempFile(URL path) throws IOException {
        String libPath = System.getProperty("java.library.path");
        String tmpDir = System.getProperty("java.io.tmpdir");
        Path target = (libPath == null || libPath.isEmpty()) ? Path.of(libPath) : Path.of(tmpDir);

        File tempFile = File.createTempFile("lib", ".dll", target.toFile());
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

    private static void loadLibrary(File tempFile) {
        try {
            System.load(tempFile.getAbsolutePath());
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Fehler beim Laden der Bibliothek: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Sicherheitsverletzung beim Laden der Bibliothek: " + e.getMessage());
        }
    }
}
