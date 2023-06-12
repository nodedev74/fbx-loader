# JVulkan Triangle

This project provides a method for creating a Vulkan window in Java that renders a "Hello-Triangle".

## Environment

* Windows (x64) supported only
* Apache Maven 3.9.1
* Java Runtime Environment 19
* VulkanSDK 1.3.250.0
  * SDL2 libraries and headers
  * Volk header, source and library
  * Vulkan Memory Allocator header

```shell
mvn clean test
```

## Known issues

* The JNILoader & ShaderLoader are creating files in the Windows temporary directory that are not automatically deleted. This issue arises due to the lack of support in JNI for unlinking libraries at runtime. Migrating to JNA would resolve this problem, as JNA supports library unlinking. This issue leads to multiple unused temporary files that will be removed by Windows at some point.
