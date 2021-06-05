#include "SturdyEngine.h"
#include <string>
#include <cstring>
#include <cmath>
#include <stdbool.h>
#include "glm/gtc/random.hpp"
class Application : public SF10::SturdyEngine {
    public:
        vec2 screenDimensions = vec2(0.0);
        vec2 windowVelocity;
        vec2 windowPos;
        void setup() {
            this->renderer = SF10::renderTypes::Rasterized;
            setCursorMode(this->getWindow(), SF10::Input::Mouse::NORMAL);
            std::cout << "Application Suggested Threads: " << getSuggestedMaxThreadCount() << std::endl;
            std::cout << "Using Processor: " << processor.getDescriptor().brandname << std::endl;
            windowVelocity = vec2(120, 420);
            windowPos = vec2(0.0);
        }
        void update() {
            draw();
            vec2 corner2(windowPos.x + getWindowedSize().x, windowPos.y + getWindowedSize().y);
            SF10::MonitorDescriptor mon = getMonitorByPoint(windowPos);
            vec2 monCorner2(mon.bounds.x + mon.bounds.z, mon.bounds.y + mon.bounds.w);
            if (windowPos.x < mon.bounds.x) {
                windowVelocity.x = abs(windowVelocity.x);
            }
            if (windowPos.y < mon.bounds.y) {
                windowVelocity.y = abs(windowVelocity.y);
            }
            if (corner2.x > monCorner2.x) {
                windowVelocity.x = -abs(windowVelocity.x);
            }
            if (corner2.y > monCorner2.y) {
                windowVelocity.y = -abs(windowVelocity.y);
            }
            vec2 move = vec2(windowVelocity.x * getFrameTime(), windowVelocity.x * getFrameTime());
            vec2 newPos = windowPos + move;
            windowPos = newPos;
            glfwSetWindowPos(getWindow(), windowPos.x, windowPos.y);

        }
        void clean() {
            std::cout << "Clean Called, Application closing" << "\n";
        }

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

        //calls back every time there is a change of state of a mouse button
        void onClick(SF10::Input::Mouse::Button button) {
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