#pragma once

#include "ofMain.h"
#include <ofxMidi.h>
// #include <ofxMidiConstants.h>

typedef std::vector<std::string> stringvec;

class ofApp : public ofBaseApp, public ofxMidiListener
{
public:
	~ofApp() noexcept override;
	void setup();
	void update();
	void draw();

	void getVideos(const std::string &path, stringvec &ext, stringvec &vector);
	void getImages(const std::string &path, stringvec &ext, stringvec &vector);

	void drawSource();
	void nextVideo();
	void nextImage();
	void powerOffCheckRoutine();
	void checkClicksRoutine();
	void checkJoysticksRoutine();
	void updateControlsRoutine();
	void checkVideoPlayback();
	void keyPressed(int key);
	void newMidiMessage(ofxMidiMessage& message);

	ofxMidiIn midiIn;
	std::map<int, float> midiCCValues;
	float pot1Value, pot2Value, pot3Value, pot4Value = 0.0;
	bool button1Value, button2Value = 0;
	int long currentTime;
	int timer = 100;
	int clickCounter1, clickCounter2 = 0;
	int nClicks1, nClicks2 = 0;

	ofFbo buffer, buffer2, buffer3, textureBuffer;
	ofShader shader1, shader2, shader3, shader4, lumakey, shader6, textureShader;

	ofVideoGrabber cam;
	ofVideoPlayer player;
	ofImage image1, paintImage;

	stringvec videos;
	stringvec images;
	stringvec extVideos;
	stringvec extImages;
	string path;
	bool debug;
	bool isReady;
	bool noInput;
	bool camOn;
	bool notLoading;
	bool paintMode;
	bool sourceParamMode;
	bool sourceRotateWaitRecall, sourceZoomWaitRecall, feedbackRotateWaitRecall, feedbackZoomWaitRecall;
	int selector;
	int num, imageNum;
	uint64_t loadCount;
	bool turnOffStarted;
	uint64_t beginTime;
	int zoom;
	int paintZoom, oldPaintZoom;
	int sourceZoom, oldSourceZoom;
	float rotate;
	int paintRotate, oldPaintRotate;
	int sourceRotate, oldSourceRotate;
	float xAxis1, yAxis1, xAxis2, yAxis2, knob, keyTresh, dispX, dispY, paintDispX, paintDispY, sourceDispX, sourceDispY;
};
