//#include "meminfo.h" //this is only to be enabled on windows or linux, do not even try to use it elsewhere, as there are no protocols to gain access to memory consumption on other platforms. comment this out again when compiling for mac or mobile(Notice: as of right now this header does not exist, do not use)
#include "SturdyEngine.h"
#include <string>
#include <cstring>
#include <cmath>
#include <stdbool.h>
#include "glm/gtc/random.hpp"
class Application : public SFT::SturdyEngine {
    public:
        vec2 screenDimensions = vec2(0.0);
        vec2 windowVelocity;
        vec2 windowPos;
        bool firstUpdate = true;
        SFT::MonitorDescriptor mon;
        std::vector<SFT::Scene::Vertex> vertices;
        void setup() {
            this->renderer = SFT::renderTypes::Rasterized;
            setCursorMode(this->getWindow(), SFT::Input::Mouse::NORMAL);
            std::cout << "Application Suggested Threads: " << getSuggestedMaxThreadCount() << std::endl;
            std::cout << "Using Processor: " << processor.getDescriptor().name << std::endl;
        }
        void update() {
            draw();
        }
        void clean() {
            std::cout << "Clean Called, Application closing" << "\n";
        }

        //handles all keys raw, action is used to tell what is happening and mods is for the shift, ctrl, alt, super, capslock keys to detect activation
        void onKey(int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE) {
                exit(0);
            }
        }


        //handles text input as expected of a good text box, except for backspace, enter, delete, tab. these will need the onKey listener unfortunately
        void onChar(char key) {
            std::cout << "Char: " << key << std::endl;
        }

        void onScroll(GLFWwindow* window, int xDel, int yDel) {
            std::cout << "Scroll read: vec2(X: " << xDel << ", Y: " << yDel << ")" << std::endl;
        }

        void onMouseMove(vec2 pos) {

        }

        void onWindowResize(GLFWwindow* window, int width, int height) {
            if (window == this->getWindow()) {
                this->screenDimensions = vec2(width, height);
            }
        }
        //calls back every time there is a change of state of a mouse button, this works for the extended mouse buttons too(Mouse button 4 which you commonly see bound as a secondary input sometimes in games is valid here)
        void onClick(SFT::Input::Mouse::Button button) {
            std::cout << "Click Event\nButton: " << button.button << "\nPosition: (X: " << button.position.x << ", Y: " << button.position.y << ")\nAction: " << button.lastAction << "\nHandled: " << button.wasHandled << std::endl;
            button.wasHandled = true;
        }

    private:
    
};


int main() {
    Application app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}