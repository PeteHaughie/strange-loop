#include "ofMain.h"
#include "ofApp.h"

int main()
{
    ofGLESWindowSettings settings;
    settings.setSize(720, 480);
    // settings.windowMode = OF_GAME_MODE;
    settings.setGLESVersion(2);
    settings.windowMode = OF_WINDOW;
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}
