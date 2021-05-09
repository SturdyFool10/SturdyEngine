cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(project_name SturdyEngine)
set(GLM_DIR D:/Libraries/glm/cmake/glm)
set(OpenXR_DIR D:/Libraries/OpenXR-SDK-master/build/x64/src/loader)
set(spirv_cross_cpp_DIR D:/Libraries/SPIRV-Cross-master)
project(${project_name})

set(HAVE_LIBC TRUE)
set(src
    Main.cpp
    SturdyEngine.h
)
 
set(headers

)
add_executable(${project_name} ${src} ${headers})

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLM CONFIG REQUIRED)
find_package(OpenXR CONFIG REQUIRED)
file(COPY shaders/ DESTINATION shaders/)

if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
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