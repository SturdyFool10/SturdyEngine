cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(project_name SturdyEngine)
set(libName SturdyEngineLib)
project(${project_name})

set(HAVE_LIBC TRUE)
set(src
    Main.cpp
    SturdyEngine.h
    cpuinfo/cpuinfo.h
    cpuinfo/cpuinfox86.h
    cpuinfo/cpuinfoARM.h
)
set(LIBSRC
	SturdyEngine.h
	cpuinfo/cpuinfo.h
	cpuinfo/cpuinfox86.h
 	cpuinfo/cpuinfoARM.h
)
set(headers

)
set(includes
	Libraries/glfw/include/
	Libraries/glm/
	Libraries/OpenXR/include
	Libraries/Vulkan/Include
)
set(libs  
includelib/dxcompiler  
includelib/GenericCodeGen  
includelib/glfw3  
includelib/glfw3dll  
includelib/glfw3_mt  
includelib/glslang-default-resource-limits  
includelib/glslang  
includelib/HLSL  
includelib/MachineIndependent  
includelib/OGLCompiler  
includelib/OSDependent  
includelib/shaderc    
includelib/shaderc_shared  
includelib/shaderc_util  
includelib/spirv-cross-c-shared  
includelib/spirv-cross-c  
includelib/spirv-cross-core  
includelib/spirv-cross-cpp  
includelib/spirv-cross-glsl  
includelib/spirv-cross-hlsl  
includelib/spirv-cross-msl  
includelib/spirv-cross-reflect  
includelib/spirv-cross-util  
includelib/SPIRV-Tools-link  
includelib/SPIRV-Tools-lint  
includelib/SPIRV-Tools-opt  
includelib/SPIRV-Tools-reduce  
includelib/SPIRV-Tools-shared  
includelib/SPIRV-Tools  
includelib/SPIRV  
includelib/SPVRemapper  
includelib/VkLayer_utils  
includelib/vulkan-1  

)
add_executable(${project_name} ${src} ${headers})
include_directories(${includes})
target_link_libraries(${project_name} PUBLIC ${libs})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${project_name})
file(COPY shaders/ DESTINATION shaders/)
file(COPY shaders/ DESTINATION Debug/shaders/)
file(COPY shaders/ DESTINATION Release/shaders/)
file(COPY includelib/ DESTINATION includelib/)