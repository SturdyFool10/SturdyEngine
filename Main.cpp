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
        void setup() {
            this->renderer = SFT::renderTypes::Rasterized;
            setCursorMode(this->getWindow(), SFT::Input::Mouse::NORMAL);
            std::cout << "Application Suggested Threads: " << getSuggestedMaxThreadCount() << std::endl;
            std::cout << "Using Processor: " << processor.getDescriptor().brandname << std::endl;
            windowVelocity = glm::diskRand(5000.0);
            int x, y;
            glfwGetWindowPos(getWindow(), &x, &y);
            windowPos = vec2(x, y);
            int n = (10289, 2394324, 4342434, 434322343, 3424343);
            std::cout << "Num: " << n << std::endl;
            requestFocus();
        }
        void update() {
            draw();
            vec2 corner2(windowPos.x + getWindowedSize().x, windowPos.y + getWindowedSize().y);
            SFT::MonitorDescriptor mon = getMonitorByPoint(windowPos);
            vec2 monCorner2(mon.bounds.x + mon.bounds.z, mon.bounds.y + mon.bounds.w);
            if (windowPos.x < mon.bounds.x) {
                windowVelocity.x = abs(windowVelocity.x);
            }
            if (corner2.x > monCorner2.x) {
                windowVelocity.x = -abs(windowVelocity.x);
            }
            if (windowPos.y < mon.bounds.y) {
                windowVelocity.y = abs(windowVelocity.y);
            }
            if (corner2.y > monCorner2.y) {
                windowVelocity.y = -abs(windowVelocity.y);
            }
            vec2 move = vec2(windowVelocity.x * getFrameTime(), windowVelocity.y * getFrameTime());
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