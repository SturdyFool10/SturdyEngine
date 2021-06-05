cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(project_name SturdyEngine)
set(GLM_DIR D:/Libraries/glm/cmake/glm)
set(OpenXR_DIR D:/Libraries/OpenXR-SDK-master/build/x64/src/loader)
set(CpuFeatures_DIR D:/Libraries/cpu_features-master/cmake)
project(${project_name})

set(HAVE_LIBC TRUE)
set(src
    Main.cpp
    SturdyEngine.h
    cpuinfo/cpuinfo.h
    cpuinfo/cpuinfox86.h
    cpuinfo/cpuinfoARM.h
)
 
set(headers

)
add_executable(${project_name} ${src} ${headers})

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLM CONFIG REQUIRED)
find_package(OpenXR CONFIG REQUIRED)

file(COPY shaders/ DESTINATION shaders/)
file(COPY shaders/ DESTINATION Debug/shaders/)
file(COPY shaders/ DESTINATION Release/shaders/)

target_link_directories(${project_name} PUBLIC D:/Libraries/RayTracingDenoiser-master/Include)
target_link_directories(${project_name} PUBLIC D:/Libraries/NSight/NvPerf/include/windows-desktop-x64)
if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/spirv-cross-cpp.lib)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/spirv-cross-glsl.lib)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/spirv-cross-hlsl.lib)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/spirv-cross-msl.lib)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/spirv-cross-util.lib)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/spirv-cross-reflect.lib)
	target_link_libraries(${project_name} D:/Libraries/VulkanSDK/1.2.170.0/Lib/glslang.lib)
	
endif()
if (glfw3_FOUND)
	target_link_libraries(${project_name} glfw)
endif()
if (GLM_FOUND)
	target_link_libraries(${project_name} glm::glm)
endif()
if (OpenXR_FOUND)
    target_link_libraries(${project_name} OpenXR::openxr_loader)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${project_name})