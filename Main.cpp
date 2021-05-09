#include "SturdyEngine.h"
#include <string>
#include <cstring>
#include <cmath>
#include <stdio.h>

class Application : public SF10::SturdyEngine {
    public:
        void setup() {
            this->renderer = SF10::renderTypes::Rasterized;
        }
        void update() {
            draw();
        }
        void clean() {
            std::cout << "Clean Called, Application closing" << "\n";
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