#include "ofMain.h"
#include "ofApp.h"

int main()
{
    ofGLESWindowSettings settings;
    settings.setSize(1280, 720);
    // settings.windowMode = OF_GAME_MODE;
    settings.setGLESVersion(2);
    settings.windowMode = OF_WINDOW;
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}
