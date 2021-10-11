#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <chrono>
#include <memory> //does java like gc for us... todo: use this in games
#include <thread>
#include "cpuinfo/cpuinfo.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <OpenXR/openxr.h> //TODO: actually use this
#include <glm/glm.hpp>
constexpr auto procInfoMode = 1;
/*
TODO list:
- OpenXR Vulkan Pipelines
- Dynamic Shader Support
- Shader Buffers
- Multi-window Support
- Camera API
- Default Shaders(2D/3D, RTX, Mesh Shader, VR)
- Basic effect shaders
- Render Pipeline Hotswap(Swap between 2d, 3d, and all supported pipelines in the update function
- APIs for gamepad, keyboard, and mouse support(GLFW Abstraction)
- Shader-accessible uniforms for depth buffer and screen-space + worldspace movement vectors per pixel
- get legal documents for engine
- add engine-wide support for compute shaders
- add engine-wide support for hdr10
- basic button api
- Code Generators for scenes to improve load times
- Focus listeners(window defocus and focus)
*/

using vec2 = glm::vec2;
using vec3 = glm::vec3;
namespace SFT {
    namespace Scene {
        namespace Camera {
            class Camera2D {
            public:

            private:
                vec2 pos;
                glm::mat4x4 projectionMatrix;
            };
            class Camera3D {
            public:
                void setPosition(vec3 position) {
                    this->pos = position;
                }
                template<typename T>
                void setPosition(T x, T y, T z) {
                    this->pos = vec3(x, y, z);
                }
                void setRotationDeg(vec3 rotation) {
                    this->rot = vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
                }
                void setRotationRad(vec3 rotation) {
                    this->rot = rotation;
                }
                template<typename T>
                void setRotationDeg(T x, T y, T z) {
                    this->rot = vec3(glm::radians(x), glm::radians(y), glm::radians(z));
                }
                template<typename T>
                void setRotationRad(T x, T y, T z) {
                    this->rot = vec3(x, y, z);
                }
                vec3 getRotationRad() {
                    return this->rot;
                }
                vec3 getRotationDeg() {
                    return vec3(glm::degrees(this->rot.x), glm::degrees(this->rot.y), glm::degrees(this->rot.z));
                }
                vec3 getPosition() {
                    return this->pos;
                }
                enum ProjectionMatrixes {
                    Perspective, Orthographic
                };
                //this overload is a very good way to break shit, using the other one is recommended, this is only here to allow for some very advanced tricks, and for inheritance
                void setProjectionMatrix(glm::mat4x4 matrix) {
                    this->projectionMatrix = matrix;
                }
            private:
                vec3 pos;
                vec3 rot;
                glm::mat4x4 projectionMatrix;
            };
            //NOTE: the perspective camera's far poroperty will be null if it is set to be infinite, for most 3D programs this is your go to camera type
            class PerspectiveCamera3D : Camera3D {
            public:

                double fov, aspect, near, far;
                void setProjectionParameters(double fov = 45.0, double aspect = 16.0 / 9.0, double near = 0.000000002, double far = 1000000.0) {
                    this->setProjectionMatrix(glm::perspective(glm::radians(fov), aspect, near, far));
                    setMainProperties(fov, aspect, near);
                    this->far = far;
                }
                //sets the perspective camera to only have a near clipping plane, and will not ever cull what is infront of it ever
                void setProjectionParametersINF(double fov = 45.0, double aspect = 16.0 / 9.0, double near = 0.000000002) {
                    this->setProjectionMatrix(glm::infinitePerspective(glm::radians(fov), aspect, near));
                    setMainProperties(fov, aspect, near);
                    this->far = NULL;
                }
            private:
                void setMainProperties(double fov, double aspect, double near) {
                    this->fov = fov;
                    this->aspect = aspect;
                    this->near = near;
                }
            };
            //NOTE: near and far will be set to null in the case of an infinite ortho camera
            class OrthographicCamera : Camera3D {
            public:
                double left, right, bottom, top, near, far;
                void setProjectionParameters(double left, double right, double bottom, double top, double near, double far) {
                    this->setProjectionMatrix(glm::ortho(left, right, bottom, top, near, far));
                    this->setMainProperties(left, right, bottom, top, near, far);
                }
                void setProjectionParametersINF(double left, double right, double bottom, double top) {
                    this->setProjectionMatrix(glm::ortho(left, right, bottom, top));
                    this->setMainProperties(left, right, bottom, top);
                }
            private:
                void setMainProperties(double left, double right, double bottom, double top, double near = NULL, double far = NULL) {
                    this->left = left;
                    this->right = right;
                    this->bottom = bottom;
                    this->top = top;
                    this->near = near;
                    this->far = far;
                }
            };
        }
        struct Vertex {
            glm::vec2 pos;
            glm::vec3 color;

            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(Vertex, pos);
                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(Vertex, color);
                return attributeDescriptions;
            }
        };
        class World2D {
        private:
            glm::mat3x3 transformationMatrix;
        };
        class World3D {
        private:
            glm::mat4x4 transformationMatrix;
        };
        struct Mesh {
        public:
            std::vector<Vertex> verticies;
            std::vector<int> indicies;
        };
        class Scene {
        public:
            std::vector<Vertex> getVerts() {
                return verts;
            }
            //I have hidden access to the raw scene description to make it possible to keep tabs on when the vertex buffer needs updating
            void addVert(Vertex v) {
                this->verts.push_back(v);
                this->updateGPUBuffer = true;
            }
            void setVerts(std::vector<Vertex> v) {
                this->verts = v;
                this->updateGPUBuffer = true;
            }
            void removeUpdateFlag() {
                this->updateGPUBuffer = false;
            }
            void setVertexBuffer(VkBuffer& buffer) {
                this->vertexBuffer = buffer;
                this->vertexBufferSet = true;
            }
            VkBuffer& getVertexBuffer() {
                return this->vertexBuffer;
            }
            VkBuffer& getIndexBuffer() {
                return this->indexBuffer;
            }
            void destroyVertexBuffer(VkDevice& device) {
                if (this->vertexBufferSet == true) {
                    vkDestroyBuffer(device, this->indexBuffer, nullptr);
                    vkFreeMemory(device, this->indexBufferMemory, nullptr);

                    vkFreeMemory(device, this->vertexBufferMemory, nullptr);
                    vkDestroyBuffer(device, this->vertexBuffer, nullptr);
                    this->vertexBufferSet = false;
                }
            }
            void setVertexBufferMemory(VkDeviceMemory& memory) {
                this->vertexBufferMemory = memory;
                this->vertexBufferSet = true;
            }
            void setDevice(VkDevice& device) {
                this->device = device;

            }
            ~Scene() {
                destroyVertexBuffer(this->device);
            }
            void addMesh(Mesh& m) {
                this->meshes.push_back(m); //you might want to make sure this object has lifetime, since the engine relies on it
            }
            void generateLists() {
                this->verts.clear();
                this->indicies.clear();
                uint32_t totalVerts = 0;
                uint32_t totalInd = 0;
                for (auto i : meshes) {
                    totalVerts += i.verticies.size();
                    totalInd += i.indicies.size();
                }
                uint32_t totalDone[2] = { 0, 0 };
                this->verts.resize(totalVerts);
                this->indicies.resize(totalInd);
                for (auto i : meshes) {
                    int indOffset = totalDone[1];
                    for (auto j : i.verticies) {
                        this->verts[totalDone[0]] = j;
                        totalDone[0] += 1;
                    }
                    for (auto j : i.indicies) {
                        uint32_t newInd = j + indOffset;
                        this->indicies[totalDone[1]] = newInd;
                        totalDone[1] += 1;
                    }

                }
            }

            std::vector<uint32_t> getIndicies() {
                return this->indicies;
            }
            void setIndexBuffer(VkBuffer& buffer) {
                this->indexBuffer = buffer;
            }
            void setIndexBufferMemory(VkDeviceMemory& mem) {
                this->indexBufferMemory = mem;
            }
        private:
            std::vector<Mesh> meshes = { {
                        {
                            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
                        },
                        {
                            0, 1, 2, 2, 3, 0
                        }

            } };
            std::vector<Vertex> verts;
            std::vector<uint32_t> indicies;
            //todo: update this and make it so that we only need an update when changes are made here
            bool updateGPUBuffer = false;
            bool vertexBufferSet = false;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            VkBuffer indexBuffer;
            VkDeviceMemory indexBufferMemory;
            VkDevice device;
        };
    }
    namespace Input {
        namespace Keyboard {
            enum Modifiers {
                SHIFT, CTRL, ALT, CAPSLOCK, NUMLOCK, SUPER
            };
        };
        namespace Mouse {
            enum Modes {
                NORMAL, HIDDEN, POINTERLOCKED
            };
            class Button {
            public:
                //this is just here ot allow you ot tell when a button event has been handled, useful for button apis
                bool wasHandled;
                vec2 position;
                int button;
                int lastAction;
                bool pressed;
                Button(int b = NULL, vec2 pos = vec2(0.0), bool handled = true, int lastAction = GLFW_RELEASE) {
                    this->wasHandled = handled;
                    this->button = b;
                    this->position = pos;
                    this->lastAction = lastAction;
                    this->pressed = false;
                }
                void setVars(vec2 pos, bool handled, int lastAction) {
                    this->position = pos;
                    this->wasHandled = handled;
                    this->lastAction = lastAction;
                    if (lastAction == GLFW_PRESS) {
                        this->pressed = true;
                    }
                    else {
                        this->pressed = false;
                    }
                }
            };
        };

        struct KeyboardInput {
            bool modifiers[6];
            bool keys[GLFW_KEY_LAST];
            KeyboardInput() {
                for (size_t i = 0; i < GLFW_KEY_LAST; ++i) {
                    this->keys[i] = false;
                }
                for (size_t i = 0; i < 6; ++i) {
                    this->modifiers[i] = false;
                }
            }
            //wordy as hell, but works dynamically, cannot complain
            void setMods(int mods) {
                this->modifiers[Keyboard::Modifiers::SHIFT] = (mods & GLFW_MOD_SHIFT) != 0x0000;
                this->modifiers[Keyboard::Modifiers::ALT] = (mods & GLFW_MOD_ALT) != 0x0000;
                this->modifiers[Keyboard::Modifiers::CTRL] = (mods & GLFW_MOD_CONTROL) != 0x0000;
                this->modifiers[Keyboard::Modifiers::SUPER] = (mods & GLFW_MOD_SUPER) != 0x0000;
                this->modifiers[Keyboard::Modifiers::CAPSLOCK] = (mods & GLFW_MOD_CAPS_LOCK) != 0x0000;
                this->modifiers[Keyboard::Modifiers::NUMLOCK] = (mods & GLFW_MOD_NUM_LOCK) != 0x0000;
            }
        };
        using mouseButton = SFT::Input::Mouse::Button;
        class MouseInput {
        public:
            MouseInput() {
                for (size_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i) {
                    this->buttons[i] = Mouse::Button(i, vec2(0.0), true, GLFW_RELEASE);
                }
                this->mousePos = vec2(0.0);
            }
            mouseButton buttons[7];
            vec2 mousePos;
        };
        namespace Gamepad {

        };
    };
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    namespace RequiredExtensions {
        namespace DeviceExtensions {
            const std::vector<const char*> Rasterized = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };

            const std::vector<const char*> Raytraced = {
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                VK_KHR_MAINTENANCE3_EXTENSION_NAME,
                VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
                VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
        };
        namespace InstanceExtensions {
            const std::vector<const char*> standard = {
            };
        }
    };


#ifndef enableValidationLayers
    const bool enableValidationLayers = false;
#endif

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    using string = std::string;


    bool firstLoop = true;
    enum class renderTypes {
        Rasterized, Raytraced, MeshShaded, VRRasterized, VRMesh, VRRaytraced
    };
    class ColorDepthDescriptor {
    public:
        int r, g, b;
        ColorDepthDescriptor(int r, int g, int b) {
            this->r = r;
            this->g = g;
            this->b = b;
        }
        void logInfo() {
            std::cout << "\t\t\tRGB: " << r << ", " << g << ", " << b << std::endl;
        }
    };
    class VideoModeOrganizer {
    public:
        int width;
        int height;
        std::vector<ColorDepthDescriptor> colorDepths;
        std::vector<int> framerates;
        std::vector<VideoModeOrganizer> exclusions;
        VideoModeOrganizer(int w = 0, int h = 0, std::vector<ColorDepthDescriptor> colorDepthList = {}, std::vector<int> framerates = {}, std::vector<VideoModeOrganizer> exclusions = {}) {
            this->width = w;
            this->height = h;
            this->colorDepths = colorDepthList;
            this->framerates = framerates;
            this->exclusions = exclusions; // this is a list of incompatible formats, such as if such a framerate cannot be achieved, I have no idea if this will ever be used though
        }
        void addFramerateMode(int framerate) {
            this->framerates.push_back(framerate);
        }
        void addColorDepthMode(ColorDepthDescriptor desc) {
            this->colorDepths.push_back(desc);
        }
        void addExclusion(VideoModeOrganizer exclusion) {
            this->exclusions.push_back(exclusion);
        }
        bool hasMode(ColorDepthDescriptor desc, int framerate) {
            bool found = false;
            for (auto i : colorDepths) {
                for (auto j : framerates) {
                    if (i.r == desc.r && (i.g == desc.g && (i.b == desc.b && (j == framerate)))) {
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            return found;
        }
        bool hasColorDepth(ColorDepthDescriptor desc) {
            bool found = false;
            for (auto i : colorDepths) {
                if (i.r == desc.r && (i.g == desc.g && (i.b == desc.b))) {
                    found = true;
                    break;
                }
            }
            return found;
        }
        bool hasFramerate(int framerate) {
            bool found = false;
            for (auto i : framerates) {
                if (i == framerate) {
                    found = true;
                    break;
                }
            }
            return found;
        }
        void logInfo(std::ostream outputLocation) {
            outputLocation << "\tMode {" << std::endl;
            outputLocation << "\t\tResolution: " << this->width << "X" << this->height << std::endl;
            outputLocation << "\t\tColorDepths: {" << std::endl;
            for (auto i : this->colorDepths) {
                outputLocation << "\t\t\tRGB: " << i.r << ", " << i.g << ", " << i.b << std::endl;
            }
            outputLocation << "\t\t}" << std::endl;
            outputLocation << "\t\tFramerates: {" << std::endl;
            for (auto i : framerates) {
                std::cout << "\t\t\t" << i << std::endl;
            }
            outputLocation << "\t\t}" << std::endl;
            outputLocation << "\t}" << std::endl;
        }
        void logInfo() {
            std::cout << "\tMode {" << std::endl;
            std::cout << "\t\tResolution: " << this->width << "X" << this->height << std::endl;
            std::cout << "\t\tColorDepths: {" << std::endl;
            for (auto i : this->colorDepths) {
                i.logInfo();
            }
            std::cout << "\t\t}" << std::endl;
            std::cout << "\t\tFramerates: {" << std::endl;
            for (auto i : framerates) {
                std::cout << "\t\t\t" << i << std::endl;
            }
            std::cout << "\t\t}" << std::endl;
            std::cout << "\t}" << std::endl;
        }
    };
    class MonitorDescriptor {
    public:
        glm::vec4 bounds;
        GLFWmonitor* obj;
        const char* name;
        std::vector<VideoModeOrganizer> vidOrganizers;
        MonitorDescriptor(glm::vec4 bounds, GLFWmonitor* mon) {
            this->bounds = bounds;
            this->obj = mon;
            const char* n = glfwGetMonitorName(mon);
            getVideoModes(mon);
            this->name = n;
        }
        VideoModeOrganizer getVideoModeOrganizer(int resolutionWidth, int resolutionHeight) {
            VideoModeOrganizer toRet = NULL;
            for (auto or : vidOrganizers) {
                if (resolutionWidth == or.width && (resolutionHeight == or.height)) {
                    toRet = or ;
                }
            }
            return toRet;
        }
        void setVideoModeOrganizer(VideoModeOrganizer organizer) {
            for (int i = 0; i < vidOrganizers.size(); ++i) {
                VideoModeOrganizer or = vidOrganizers[i];
                if (organizer.width == or.width && (organizer.height == or.height)) {
                    vidOrganizers[i] = organizer;
                }
            }
        }
        void getVideoModes(GLFWmonitor* mon) {
            int modeCount;
            const GLFWvidmode* videoModes = glfwGetVideoModes(mon, &modeCount);
            for (int i = 0; i < modeCount; ++i) {
                GLFWvidmode mode = videoModes[i];
                if (mode.width == 0 || (mode.height == 0 || (mode.redBits == 0 || (mode.greenBits == 0 || (mode.blueBits == 0 || (mode.refreshRate == 0)))))) continue;
                bool found = false;
                for (auto t : vidOrganizers) {
                    if (mode.width == t.width && (mode.height == t.height)) {
                        found = true;
                        break;
                    }
                }
                VideoModeOrganizer organizer = getVideoModeOrganizer(mode.width, mode.height);
                if (!found) {
                    organizer = VideoModeOrganizer(mode.width, mode.height);
                    vidOrganizers.push_back(organizer);
                }

                ColorDepthDescriptor d = ColorDepthDescriptor(mode.redBits, mode.greenBits, mode.blueBits);
                if (organizer.hasColorDepth(d) == false) {
                    organizer.addColorDepthMode(d);
                }

                if (organizer.hasFramerate(mode.refreshRate) == false) {
                    organizer.addFramerateMode(mode.refreshRate);
                }
                setVideoModeOrganizer(organizer);
            }
        }
        void logInfo(std::ostream outputLocation) {
            outputLocation << "Display Name: " << this->name << ", bounds: [" << bounds.x << ", " << bounds.y << ", " << bounds.z << ", " << bounds.w << "], Modes: {" << std::endl;
            for (auto m : this->vidOrganizers) {

                outputLocation << "\tMode {" << std::endl;
                outputLocation << "\t\tResolution: " << m.width << "X" << m.height << std::endl;
                outputLocation << "\t\tColorDepths: {" << std::endl;
                for (auto i : m.colorDepths) {
                    outputLocation << "\t\t\tRGB: " << i.r << ", " << i.g << ", " << i.b << std::endl;
                }
                outputLocation << "\t\t}" << std::endl;
                outputLocation << "\t\tFramerates: {" << std::endl;
                for (auto i : m.framerates) {
                    outputLocation << "\t\t\t" << i << std::endl;
                }
                outputLocation << "\t\t}" << std::endl;
                outputLocation << "\t}" << std::endl;
            }
            outputLocation << "}" << std::endl;
        }
        void logInfo() {
            std::cout << "Display Name: " << this->name << ", bounds: [" << bounds.x << ", " << bounds.y << ", " << bounds.z << ", " << bounds.w << "], Modes: {" << std::endl;
            for (auto m : this->vidOrganizers) {
                m.logInfo();
            }
            std::cout << "}" << std::endl;
        }
        bool isPointInMonitor(glm::vec2 pos) {
            int x2 = bounds.x + bounds.z;
            int y2 = bounds.y + bounds.w;
            if (pos.x >= bounds.x && (pos.y >= bounds.y && (pos.x <= x2 && (pos.y <= y2)))) {
                return true;
            }
            return false;
        }
        MonitorDescriptor() {
            this->bounds = glm::vec4(0);
            this->obj = NULL;
        }
    };
    class SturdyEngine {
    public:
        CPUInfo::CPU processor;
        void run() {
            bool rtCapable = false;
            initWindow();
            this->setup();
            switch (renderer) {
            case renderTypes::Rasterized:
                initVulkanRasterized();
                break;
            case renderTypes::Raytraced:
                renderer = renderTypes::Raytraced;
                initVulkanRaytraced();
                break;
            }
            mainLoop(); //Raytracing causes nullpointer exceptions with these two, TODO: either segment or error-proof
            cleanup();
        }
        void draw() {
            //call this function to actually draw a frame, this will be put on the developer to decide when this is called in update()
            drawFrame();
        }
        void setRenderer(renderTypes type) {
            this->renderer = type;
        }
        double getFrameRate() {
            return this->frameRate;
        }
        double getFrameTime() {
            return this->frameTime;
        }
        VkDevice getDevice() {
            return this->device;
        }
        VkPhysicalDevice getGPU() {
            return this->physicalDevice;
        }
        VkPhysicalDeviceProperties getGPUProperties(VkPhysicalDevice& dev) {
            VkPhysicalDeviceProperties p;
            vkGetPhysicalDeviceProperties(dev, &p);
            return p;
        }
        VkPhysicalDeviceLimits getGPULimits(VkPhysicalDevice& dev) {
            VkPhysicalDeviceProperties prop = getGPUProperties(dev);
            VkPhysicalDeviceLimits limits = prop.limits;
            return limits;
        }
        //returns true if the physical device supports raytracing
        bool doesGPUSupportRT(VkPhysicalDevice& d) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(d, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(d, nullptr, &extensionCount, availableExtensions.data());
            std::set<std::string> requiredExtensions(RequiredExtensions::DeviceExtensions::Raytraced.begin(), RequiredExtensions::DeviceExtensions::Raytraced.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();

        }
        //requests focus if your OS supports it, if used in setup it queues the request up for after the window is created
        void requestFocus(GLFWwindow* window) {
            if (windowExists) {
                glfwRequestWindowAttention(window);
                return;
            }
            if (window == this->window) {
                this->windowShouldRequestFocus = true;
            }
        }
        //requests focus if your OS supports it, if used in setup it queues the request up for after the window is created.
        void requestFocus() {
            if (windowExists) {
                glfwRequestWindowAttention(window);
                return;
            }
            this->windowShouldRequestFocus = true;
        }
        virtual void setup() {
            //runs once per instance, use to load files and other setup tasks, the vulkan pipeline has yet to be initialized so you can decide which renderer and which extensions you want to use
        }
        virtual void update() {
            //runs continuously until the window closes... be careful of logging if you're at high framerates, call draw() to make the frame update
        }
        virtual void clean() {
            //runs on cleanup, and is the first thing to happen, so vulkan will be completely intact at the time of this call
        }

        //begin event handlers

        int getSuggestedMaxThreadCount() {
            return std::thread::hardware_concurrency();
        }

        //handles all key events raw
        virtual void onKey(int key, int scancode, int action, int mods) {

        }


        //handles text input as expected of a good text box
        virtual void onChar(char key) {

        }

        virtual void onMouseMove(vec2 pos) {

        }

        //calls back every time there is a change of state of a mouse button
        virtual void onClick(Input::Mouse::Button button) {

        }

        virtual void onWindowResize(GLFWwindow* window, int width, int height) {

        }

        virtual void onScroll(int xDel, int yDel) {

        }

        GLFWwindow* getWindow() {
            return window;
        }
        //call in setup, dynamic enable of extensions is not supported, what you have enabled to start is what you have to work with!
        void addDeviceExtension(const char* ext) {
            requiredDeviceExtensions.push_back(ext);
        }
        //call in setup, dynamic enable of extensions is not supported, what you have enabled to start is what you have to work with!
        void addDeviceExtension(std::vector<const char*> exts) {
            for (auto ext : exts) {
                requiredDeviceExtensions.push_back(ext);
            }
        }
        void addInstanceExtension(const char* ext) {
            requiredInstanceExtensions.push_back(ext);
        }
        void addInstanceExtension(std::vector<const char*> exts) {
            for (auto ext : exts) {
                requiredInstanceExtensions.push_back(ext);
            }
        }
        //use this in update or you might have a bad time, 
        void setCursorMode(std::optional<GLFWwindow*> window, int mode) {
            if (windowExists) {
                switch (mode) {
                case Input::Mouse::POINTERLOCKED:
                    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    break;
                case Input::Mouse::HIDDEN:
                    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                    break;
                case Input::Mouse::NORMAL:
                    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    break;
                }
            }
            else {
                //this is what happens during setup()
                this->startingCursorMode = mode;
                return;
            }
        }
        void setWindowTitle(const char* title) {
            this->windowTitle = title;
            std::optional<GLFWwindow*> w = this->window;
            if (!customWindowName) {
                customWindowName = true;
            }
            if (w) {
                glfwSetWindowTitle(*w, this->windowTitle);
            }
        }
        //exists for the explicit purpose of figuring out whether or not the window is fullscreen, more like a double check internally but is useful for developers to test for fullscreen
        bool isWindowFullscren() {
            return (glfwGetWindowMonitor(this->getWindow()) != NULL);
        }
        //returns monitor id for a vec2, returns 0 if out of bounds
        int getMonitorID(vec2 pos) {
            int id = 0;
            for (size_t i = 0; i < monitors.size(); ++i) {
                vec2 pos2{ pos.x, pos.y };
                MonitorDescriptor monitor = monitors[i];
                if (monitor.isPointInMonitor(pos2)) {
                    id = i;
                    break;
                }
            }
            return id;
        }
        MonitorDescriptor getMonitorByPoint(vec2 pos) {
            int id = 0;
            for (size_t i = 0; i < monitors.size(); ++i) {
                if (pos.x == NULL || pos.y == NULL) {
                    break;
                }
                vec2 posi = vec2(pos.x, pos.y);
                MonitorDescriptor monitor = monitors[i];
                if (monitor.isPointInMonitor(posi)) {
                    id = i;
                    break;
                }
            }
            return monitors[id];
        }
        void setFullscreen(bool tf, int monitor = NULL) {
            if (fullscreen == tf || (isWindowFullscren() == tf)) {
                return;
            }
            fullscreen = tf;
            if (tf == true) {
                int x, y;
                glfwGetWindowPos(getWindow(), &x, &y);
                int id = 0;
                for (size_t i = 0; i < monitors.size(); ++i) {
                    vec2 pos = vec2(x, y);
                    MonitorDescriptor monitor = monitors[i];
                    if (monitor.isPointInMonitor(pos)) {
                        id = i;
                        break;
                    }
                }
                glfwSetWindowMonitor(this->getWindow(), monitors[id].obj, monitors[id].bounds.x, monitors[id].bounds.y, monitors[id].bounds.z, monitors[id].bounds.w, getMonitorMaxRefreshRate(monitors[id].obj));
            }
            else {
                GLFWmonitor* mon = glfwGetWindowMonitor(window);
                if (mon == NULL) {
                    return; // is just here in case someone decides to use glfw directly in-engine, this will prevent crashes
                }
                int x, y, width, height;
                glfwGetMonitorWorkarea(mon, &x, &y, &width, &height);
                glfwSetWindowMonitor(this->getWindow(), nullptr, x + ((double)width / 2.0 - (windowedSize.x / 2.0)), y + ((double)height / 2.0 - (windowedSize.y / 2.0)), windowedSize.x, windowedSize.y, GLFW_DONT_CARE);
            }
        }
        vec2 getWindowedSize() {
            return windowedSize;
        }
        int getMonitorMaxRefreshRate(GLFWmonitor* mon) {
            const GLFWvidmode* currentMode = glfwGetVideoMode(mon);
            return currentMode->refreshRate;
        }
        void setWindowPosition(vec2 pos) {
            glfwSetWindowPos(this->getWindow(), pos.x, pos.y);
        }
        vec2 getWindowPosition() {
            int x, y;
            glfwGetWindowPos(this->getWindow(), &x, &y);
            vec2 pos(x, y);
            return pos;
        }
        void setWindowDecorated(bool enable) {
            if (enable == true) {
                glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            }
            else {
                glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            }
        }
        int getCurrentWindowMonitorID() {
            int x, y;
            glfwGetWindowPos(getWindow(), &x, &y);
            int id = 0;
            for (size_t i = 0; i < monitors.size(); ++i) {
                vec2 pos = vec2(x, y);
                MonitorDescriptor monitor = monitors[i];
                if (monitor.isPointInMonitor(pos)) {
                    id = i;
                    break;
                }
            }
            return id;
        }
        bool checkHDR10Support() {
            const GLFWvidmode* currentMode = glfwGetVideoMode(monitors[getCurrentWindowMonitorID()].obj);
            if (currentMode->blueBits > 8 && currentMode->redBits > 8 && currentMode->greenBits > 8) {
                return true;
            }
            return false;
        }
        void setHDR10Support(bool enable) {
            if (enable == true && checkHDR10Support()) {
                glfwWindowHint(GL_RED_BITS, 10);
                glfwWindowHint(GL_GREEN_BITS, 10);
                glfwWindowHint(GL_BLUE_BITS, 10);
                glfwWindowHint(GL_ALPHA_BITS, 2);
            }
            else {
                if (enable == true) {
                    std::cout << "SturdyEngine could not enable HDR, either your monitor is not setup for HDR, or it is not supported at all." << std::endl;
                }
                glfwWindowHint(GLFW_RED_BITS, 8);
                glfwWindowHint(GLFW_GREEN_BITS, 8);
                glfwWindowHint(GLFW_BLUE_BITS, 8);
                glfwWindowHint(GL_ALPHA_BITS, 2);
            }
        }

        int getDeviceMemoryCapacityMB(VkPhysicalDevice device) {
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(device, &memProps);
            auto heapsPointer = memProps.memoryHeaps;
            auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memProps.memoryHeapCount);
            double totalSize = heaps[0].size / 1000000.0;
            return round(totalSize);
        }
        renderTypes renderer;
        Input::KeyboardInput keyboard;
        Input::MouseInput mouse;
        Scene::Scene getScene() {
            return this->scene;
        }
        //Anything below this point is heavy on API calls and is recommended for seasoned engine devs only, you have been warned, any and all modifications to this file do not void the license terms, as I do need to make money
    private:
        GLFWwindow* window;
        vec2 windowedSize;
        bool fullscreen;
        std::vector<VkBuffer> buffers;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        int startingCursorMode = Input::Mouse::NORMAL;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        double frameRate = 0;
        bool framebufferResized = false;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
        double frame = 0;
        std::chrono::steady_clock::time_point lastTimeLogged = std::chrono::high_resolution_clock::now();
        double frameTime = 0;
        const char* windowTitle = "SturdyEngine";
        bool windowExists = false;
        bool customWindowName = false;
        bool windowFullscreen = false;
        //custom engine code
        std::vector<const char*> requiredDeviceExtensions;
        std::vector<const char*> requiredInstanceExtensions;
        bool windowShouldRequestFocus = false;
        std::vector<MonitorDescriptor> monitors = {};

        Scene::Scene scene;

        void initWindow() {
            glfwInit();
            //Init monitors list
            int count;
            GLFWmonitor** mons = glfwGetMonitors(&count);
            monitors.resize(count);
            monitors.clear();

            for (size_t i = 0; i < count; ++i) {
                int xpos, ypos, width, height;
                glfwGetMonitorWorkarea(mons[i], &xpos, &ypos, &width, &height);
                glfwSetMonitorUserPointer(mons[i], this);
                monitors.push_back(MonitorDescriptor(glm::vec4((double)xpos, (double)ypos, (double)width, (double)height), mons[i]));
                std::cout << "found monitor " << monitors[i].name << ", ID: " << i << " at: (X: " << xpos << ", Y: " << ypos << ", Width: " << width << ", Height: " << height << ")" << std::endl;

            }

            //end monitor list init

            glfwSetMonitorCallback(privateMonitorCallback);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            window = glfwCreateWindow(WIDTH, HEIGHT, this->windowTitle, nullptr, nullptr);
            windowedSize = vec2(WIDTH, HEIGHT);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
            if (glfwRawMouseMotionSupported())
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            glfwSetKeyCallback(window, privateKeyCallback);
            glfwSetCharCallback(window, privateCharCallback);
            glfwSetCursorPosCallback(window, privateCursorPosCallback);
            glfwSetMouseButtonCallback(window, privateClickCallback);
            glfwSetScrollCallback(window, privateScrollCallback);
            glfwSetJoystickCallback(privateJoystickCallbackNC);
            this->windowExists = true;
            //this call may look stupid, but there was a alternate case where window is not initialized, so this needs to be called to actually *set* the cursor how we did during setup...
            setCursorMode(window, this->startingCursorMode);
            //setting window focus for the setup-state of it
            if (windowShouldRequestFocus) {
                glfwRequestWindowAttention(window);
            }
        }

        static void privateMonitorCallback(GLFWmonitor* monitor, int event)
        {
            SturdyEngine* a = reinterpret_cast<SturdyEngine*>(glfwGetMonitorUserPointer(monitor));
            int count;
            GLFWmonitor** mons = glfwGetMonitors(&count);
            a->monitors.resize(count);
            a->monitors.clear();

            for (size_t i = 0; i < count; ++i) {
                int xpos, ypos, width, height;
                SturdyEngine* app = reinterpret_cast<SturdyEngine*>(glfwGetMonitorUserPointer(mons[i]));
                glfwGetMonitorWorkarea(mons[i], &xpos, &ypos, &width, &height);
                app->monitors.push_back(MonitorDescriptor(glm::vec4((double)xpos, (double)ypos, (double)width, (double)height), mons[i]));
            }
        }

        static void privateKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            SturdyEngine* app = reinterpret_cast<SturdyEngine*>(glfwGetWindowUserPointer(window));
            //app->keyboard.setMods(mods);
            switch (action) {
            case GLFW_PRESS:
                app->keyboard.keys[key] = true;
                break;
            case GLFW_RELEASE:
                app->keyboard.keys[key] = false;
                break;
            case GLFW_REPEAT:
                app->keyboard.keys[key] = true;
                break;
            }
            app->onKey(key, scancode, action, mods);
        }

        static void privateCharCallback(GLFWwindow* window, unsigned int codepoint) {
            SturdyEngine* app = reinterpret_cast<SturdyEngine*>(glfwGetWindowUserPointer(window));
            char c = *"";
            c += (unsigned char)codepoint;
            app->onChar(c);
        }

        static void privateCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
            SturdyEngine* app = reinterpret_cast<SturdyEngine*>(glfwGetWindowUserPointer(window));
            vec2 pos = vec2(xpos, ypos);
            app->mouse.mousePos = pos;
            app->onMouseMove(pos);
        }

        static void privateClickCallback(GLFWwindow* window, int button, int action, int mods) {
            SturdyEngine* app = reinterpret_cast<SturdyEngine*>(glfwGetWindowUserPointer(window));
            app->mouse.buttons[button].setVars(vec2(app->mouse.mousePos), false, action);
            app->onClick(app->mouse.buttons[button]);
        }

        static void privateScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            auto app = reinterpret_cast<SturdyEngine*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
            app->onScroll(xoffset, yoffset);
        }

        //unfortunately without the existance of a exit point, this one cannot have a callback for user definition... static funcs suck, atleast we can print a nice message
        static void privateJoystickCallbackNC(int id, int action) {
            if (action == GLFW_CONNECTED) {
                if (glfwJoystickIsGamepad(id) == GLFW_TRUE) {
                    GLFWgamepadstate state;
                    glfwGetGamepadState(id, &state);
                    std::cout << "Gamepad Connected: Name: " << glfwGetGamepadName(id) << ", ID: " << id << ", Buttons: " << 15 << ", Axes: " << 6 << std::endl; //state button and axes counts are hard coded, even if I added dynamic length getter it would always be these values, might as well hard code it myself too
                }
                else {
                    std::cout << "Unkown Joystick Connected on ID: " << id << std::endl;
                }
            }
            else if (action == GLFW_DISCONNECTED) {
                std::cout << "Joystick Disconnected at ID: " << id << std::endl;
            }
        }
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
            auto app = reinterpret_cast<SturdyEngine*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
            app->onWindowResize(window, width, height);
            if (!app->fullscreen) {
                app->windowedSize = vec2(width, height);
            }
        }



        void initVulkanRasterized() {
            createInstance();
            setupDebugMessenger();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createSwapChain();
            createImageViews();
            createRenderPass();
            createGraphicsPipeline();
            createFramebuffers();
            createCommandPool();
            createVertexBuffer();
            createIndexBuffer();
            createCommandBuffers();
            createSyncObjects();
        }

        void createIndexBuffer() {
            auto indices = this->scene.getIndicies();
            VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);


            VkBuffer indexBuffer;
            VkDeviceMemory indexBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

            copyBuffer(stagingBuffer, indexBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            this->scene.setIndexBuffer(indexBuffer);
            this->scene.setIndexBufferMemory(indexBufferMemory);
        }

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphicsQueue);
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(device, buffer, bufferMemory, 0);
        }
        void createVertexBuffer() {
            scene.generateLists();
            auto vertices = scene.getVerts();
            VkDeviceSize bufferSize = sizeof(Scene::Vertex) * vertices.size();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

            copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            scene.setVertexBuffer(vertexBuffer);
            scene.setVertexBufferMemory(vertexBufferMemory);
        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }


        void initVulkanRaytraced() {
            //default extensions logic
            addDeviceExtension(RequiredExtensions::DeviceExtensions::Raytraced);
            //initializing H.A.R pipeline
            createInstance(renderer, requiredInstanceExtensions);
            setupDebugMessenger();
            createSurface();
            pickPhysicalDevice(requiredDeviceExtensions);
            createLogicalDevice(requiredDeviceExtensions);
            createAccelerationStructures();
        }
        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                this->update();
                if (customWindowName == false) {
                    std::string str = "SturdyEngine, Frame Time: ";
                    double time = getFrameTime();
                    if (time < 0.0000005) {
                        str += std::to_string(time * 1000000000.0);
                        str += "ns";
                    }
                    else if (time < 0.5) {
                        str += std::to_string(time * 1000.0);
                        str += "ms";
                    }
                    else {
                        str += std::to_string(time);
                        str += "s";
                    }
                    const char* title = str.c_str();
                    this->windowTitle = title;
                    std::optional<GLFWwindow*> w = this->window;
                    if (w) {
                        glfwSetWindowTitle(*w, this->windowTitle);
                    }
                }
            }

            vkDeviceWaitIdle(device);
        }
        void createAccelerationStructures() {
            VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature;
        }
        void cleanupSwapChain() {
            for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
                vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
            }

            vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

            vkDestroyPipeline(device, graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            vkDestroyRenderPass(device, renderPass, nullptr);

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                vkDestroyImageView(device, swapChainImageViews[i], nullptr);
            }

            vkDestroySwapchainKHR(device, swapChain, nullptr);
        }

        //cleans up memory used in default rasterizer pipeline, any additional extensions / objects may require cleanup in clean().
        void destroyVulkanRasterizer() {
            cleanupSwapChain();

            scene.destroyVertexBuffer(device);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(device, inFlightFences[i], nullptr);
            }

            vkDestroyCommandPool(device, commandPool, nullptr);

            vkDestroyDevice(device, nullptr);

            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            }

            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
        }

        //cleans up memory used by default raytracing implimentation, any additional extensions may require user-defined cleanup in clean().
        void destroyVulkanRaytracer() {
            vkDestroyDevice(device, nullptr);
            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
        }
        void cleanup() {
            clean();
            switch (renderer) {
            case renderTypes::Rasterized:
                destroyVulkanRasterizer();
                break;
            case renderTypes::Raytraced:
                destroyVulkanRaytracer();
                break;
            }

            glfwDestroyWindow(window);

            glfwTerminate();
        }

        void recreateSwapChain() {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(device);

            cleanupSwapChain();

            createSwapChain();
            createImageViews();
            createRenderPass();
            createGraphicsPipeline();
            createFramebuffers();
            createCommandBuffers();
        }

        void createInstance(renderTypes type = renderTypes::Rasterized, std::vector<const char*> instanceExts = RequiredExtensions::InstanceExtensions::standard) {
            std::vector<const char*> layers;
            if (enableValidationLayers == true) {
                if (checkValidationLayerSupport() == true) {
                    for (auto layer : validationLayers) {
                        layers.push_back(layer);
                    }
                }
                else {
                    throw std::runtime_error("validation layers requested, but not available!");
                }
            }
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "SturdyEngine Application";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "SturdyEngine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            auto extensions = getRequiredExtensions();
            std::vector<const char*> additionalExtensions;
            additionalExtensions.resize(instanceExts.size());
            int neededSize = 0;
            for (auto newExt : instanceExts) {
                bool found = false;
                for (auto extension : extensions) {
                    if (extension == newExt) {
                        found = true;
                        break;
                    }
                }
                if (found == false) {
                    additionalExtensions.push_back(newExt);
                    neededSize += 1;
                }
            } //checks to make sure we do not have duplicates on the extension list
            additionalExtensions.resize(neededSize);
            extensions.resize(extensions.size() + neededSize); //resize the extensions pool to accept the new extensions
            for (auto ext : additionalExtensions) {
                extensions.push_back(ext);
            } // add non-duplicate extensions to instance extension list


            for (auto ext : extensions) {
                std::cout << "Instance Extension: " << ext << " Enabled!" << std::endl;
            }
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
                createInfo.ppEnabledLayerNames = layers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else {
                createInfo.enabledLayerCount = 0;

                createInfo.pNext = nullptr;
            }

            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }
        }

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }

        void setupDebugMessenger() {
            if (!enableValidationLayers) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        //binds vulkan to the glfw window instance, this is absolutely necissary to view anything at all theoretically, I am not an expert with glfw, nor do I know the underlying windows platform api
        void createSurface() {
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
        }

        void pickPhysicalDevice(std::vector<const char*> requirements = deviceExtensions) {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }
            else {
                std::cout << "Found: " << deviceCount << " potential GPU(s), we need to do some selecting!!!" << std::endl;
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            //vector is here to catch any devices, if it is more than one, we need to do some kind of scoring to pick the right one
            std::vector<VkPhysicalDevice> validDevices;

            for (const auto& device : devices) {
                if (isDeviceSuitable(device, requirements)) {
                    validDevices.push_back(device);
                    break;
                }
            }
            size_t dev = validDevices.size();
            bool vkSuccess = false;
            if (dev == 1) {
                physicalDevice = validDevices[0];
                vkSuccess = true;
            }
            else if (dev > 1) {
                handleMultipleGPUs(validDevices);
                vkSuccess = true;
            }
            else {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
            if (vkSuccess) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(physicalDevice, &properties);
                std::cout << "SturdyEngine is using: " << properties.deviceName << ", this device scores: " << scoreGPU(physicalDevice) << ", Total VRAM: " << getDeviceMemoryCapacityMB(getGPU()) << std::endl;
            }
        }

        void handleMultipleGPUs(std::vector<VkPhysicalDevice> devices) {
            struct deviceScoreStorage {
                VkPhysicalDevice device;
                double score;
                deviceScoreStorage(VkPhysicalDevice device, double score) {
                    this->device = device;
                    this->score = score;
                }
            };
            std::vector<deviceScoreStorage> scores;
            for (auto device : devices) {
                scores.push_back(deviceScoreStorage(device, scoreGPU(device)));
            }
            //now to sort the scores

            deviceScoreStorage winner = scores[0];
            for (auto s : scores) {
                if (s.score > winner.score) {
                    winner = s;
                }
            }

            physicalDevice = winner.device;
        }

        double scoreGPU(VkPhysicalDevice device) {
            double score = 0;
            VkPhysicalDeviceLimits limits;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            limits = properties.limits;
            score += limits.maxFramebufferWidth / 1000.0;
            score += limits.maxFramebufferHeight / 1000.0;
            score += limits.maxViewports;
            score += limits.maxFramebufferLayers;
            score += getDeviceMemoryCapacityMB(device) / 10.0;
            return score;
        }



        void createLogicalDevice(std::vector<const char*> extensions = deviceExtensions) {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures{};

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else {
                createInfo.enabledLayerCount = 0;
            }
            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }
            else if (true) {
                for (size_t i = 0; i < createInfo.enabledExtensionCount; ++i) {
                    std::cout << "Extension Enabled: " << extensions[i] << "\n";
                }
            }
            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
            vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
        }

        void createSwapChain() {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }

            vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;
        }

        void createImageViews() {
            swapChainImageViews.resize(swapChainImages.size());

            for (size_t i = 0; i < swapChainImages.size(); i++) {
                VkImageViewCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.image = swapChainImages[i];
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format = swapChainImageFormat;
                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseMipLevel = 0;
                createInfo.subresourceRange.levelCount = 1;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create image views!");
                }
            }
        }

        void createRenderPass() {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw std::runtime_error("failed to create render pass!");
            }
        }

        void createGraphicsPipeline() {
            auto vertShaderCode = readFile("shaders/vertex.spv");
            auto fragShaderCode = readFile("shaders/frag.spv");

            VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            auto bindingDescription = Scene::Vertex::getBindingDescription();
            auto attributeDescriptions = Scene::Vertex::getAttributeDescriptions();

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)swapChainExtent.width;
            viewport.height = (float)swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swapChainExtent;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pushConstantRangeCount = 0;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
        }

        void createFramebuffers() {
            swapChainFramebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkImageView attachments[] = {
                    swapChainImageViews[i]
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }

        void createCommandPool() {
            QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create command pool!");
            }
        }

        void createCommandBuffers() {
            commandBuffers.resize(swapChainFramebuffers.size());

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

            if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }

            for (size_t i = 0; i < commandBuffers.size(); i++) {
                std::cout << "attempting indexed draw on cmd buffer id: " << i << "." << std::endl;
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass;
                renderPassInfo.framebuffer = swapChainFramebuffers[i];
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = swapChainExtent;

                VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clearColor;
                auto buffer = commandBuffers[i];
                vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
                VkBuffer& vertBuffer = scene.getVertexBuffer();
                VkBuffer vertexBuffers[] = { vertBuffer };
                VkDeviceSize offsets[] = { 0 };
                scene.generateLists();
                auto indices = scene.getIndicies();
                vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffers[i], scene.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                vkCmdEndRenderPass(buffer);

                if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
                    throw std::runtime_error("failed to record command buffer!");
                }
            }
        }

        void createSyncObjects() {
            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
            imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create synchronization objects for a frame!");
                }
            }
        }

        void drawFrame() {
            auto tS = std::chrono::high_resolution_clock::now();
            vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

            uint32_t imageIndex;
            VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                recreateSwapChain();
                return;
            }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                throw std::runtime_error("failed to acquire swap chain image!");
            }
            if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
            }
            imagesInFlight[imageIndex] = inFlightFences[currentFrame];

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
            VkPipelineStageFlags waitStages[] = { 0x2984473279 };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

            VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            vkResetFences(device, 1, &inFlightFences[currentFrame]);

            if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { swapChain };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;

            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(presentQueue, &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                recreateSwapChain();
            }
            else if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to present swap chain image!");
            }


            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            frame++;
            auto tE = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(tE - tS);
            std::chrono::duration<double> ltLogged = std::chrono::duration_cast<std::chrono::duration<double>>(tE - lastTimeLogged);
            double del = delta.count();
            double ltLog = ltLogged.count();
            frameRate = (1.0 / del);
            frameTime = del;
        }



        VkShaderModule createShaderModule(const std::vector<char>& code) {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module!");
            }

            return shaderModule;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
            if (capabilities.currentExtent.width != UINT32_MAX) {
                return capabilities.currentExtent;
            }
            else {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

                return actualExtent;
            }
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        bool isDeviceSuitable(VkPhysicalDevice device, std::vector<const char*> req = deviceExtensions) {
            QueueFamilyIndices indices = findQueueFamilies(device);

            bool extensionsSupported = checkDeviceExtensionSupport(device, req);

            bool swapChainAdequate = false;
            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            return indices.isComplete() && extensionsSupported && swapChainAdequate;
        }

        bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExt) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(requiredExt.begin(), requiredExt.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        std::vector<const char*> getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        bool checkValidationLayerSupport() {
            return checkLayerSupport(validationLayers);
        }
        bool checkLayerSupport(const std::vector<const char*> layers) {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            size_t layerS = layers.size();
            for (const char* layerName : layers) {

                for (const auto& layerProperties : availableLayers) {
                    std::cout << "layer: " << layerProperties.layerName << " version " << layerProperties.implementationVersion << "found" << std::endl;
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerS--;
                        break;
                    }
                }

            }
            return layerS == 0;
        }
        static std::vector<char> readFile(const std::string& filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                throw std::runtime_error("failed to open file: " + filename + "!");
            }

            size_t fileSize = (size_t)file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();

            return buffer;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
            if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
            }

            return VK_FALSE;
        }
    };
};