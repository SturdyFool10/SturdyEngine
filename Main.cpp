#include "SturdyEngine.h"
#include <string>
#include <cstring>
#include <cmath>
class Application : public SF10::SturdyEngine {
    public:
        void setup() {
            this->renderer = SF10::renderTypes::Rasterized;
            setCursorMode(this->getWindow(), SF10::Input::Mouse::NORMAL);
            setFullscreen(false);
            setFullscreen(false);
            setFullscreen(true);
            setFullscreen(true);
            setHDR10Support(true);
        }
        void update() {
            draw();
        }
        void clean() {
            std::cout << "Clean Called, Application closing" << "\n";
        }

        virtual void onKey(int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE) {
                exit(0);
            }
        }


        //handles text input as expected of a good text box, except for backspace, enter, delete, tab. these will need the onKey listener unfortunately
        virtual void onChar(char key) {
            std::cout << "Char: " << key << std::endl;
        }

        virtual void onMouseMove(vec2 pos) {

        }

        //calls back every time there is a change of state of a mouse button
        virtual void onClick(SF10::Input::Mouse::Button button) {
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