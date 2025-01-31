// VERSION 1.2
#include "ofApp.h"
#include <math.h>
#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

namespace fs = std::filesystem;

ofApp::~ofApp() noexcept
{
	midiIn.closePort();
	midiIn.removeListener(this);
}

//--------------------------------------------------------------
void ofApp::setup()
{
	// default app settings
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofDisableArbTex();

	// shaders
	shader1.load("shadersES2/default.vert", "shadersES2/shader1.frag");
	shader2.load("shadersES2/default.vert", "shadersES2/shader2.frag");
	shader3.load("shadersES2/default.vert", "shadersES2/shader3.frag");
	shader4.load("shadersES2/default.vert", "shadersES2/shader4.frag");
	lumakey.load("shadersES2/default.vert", "shadersES2/lumakey.frag");
	shader6.load("shadersES2/default.vert", "shadersES2/hueShift.frag");
	textureShader.load("shadersES2/default.vert", "shadersES2/texture.frag");

	// initialize MIDI
	midiIn.openPort(0); // Use first MIDI port
	midiIn.addListener(this);
	midiIn.setVerbose(true);

	image1.load("images/noInput.png");

	// control variables
	xAxis1 = 0.0;
	yAxis1 = 0.0;
	xAxis2 = 0.0;
	yAxis2 = 0.0;
	nClicks1 = 0;
	nClicks2 = 0;
	selector = 1;
	num = 0;
	imageNum = 0;
	camOn = false;
	paintMode = false;
	sourceParamMode = false;
	paintMode = false;
	noInput = false;
	turnOffStarted = false;
	notLoading = false;
	feedbackRotateWaitRecall = false;
	feedbackZoomWaitRecall = false;
	sourceRotateWaitRecall = false;
	sourceZoomWaitRecall = false;
	loadCount = 0;

	path = "../../../data/videos";

	// ofHideCursor();
	ofBackground(0, 0, 0);
	buffer.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	buffer2.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	buffer3.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	textureBuffer.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	buffer.begin();
	ofClear(255, 255, 255, 0);
	buffer.end();

	buffer2.begin();
	ofClear(255, 255, 255, 0);
	buffer2.end();
	
	buffer3.begin();
	ofClear(255, 255, 255, 0);
	buffer3.end();
	
	textureBuffer.begin();
	ofClear(255, 255, 255, 0);
	textureBuffer.end();

	// The following has to be UNCOMMENTED for NTSC operation ->
	// system("sudo v4l2-ctl -d /dev/video0 --set-standard=0");
	cam.setDeviceID(0);
	cam.setDesiredFrameRate(30);
	cam.setup(720, 480);
	// The following has to be UNCOMMENTED for PAL operation ->
	// system("sudo v4l2-ctl -d /dev/video0 --set-standard=6");
	// cam.setDeviceID(0);
	// cam.setDesiredFrameRate(25);
	// cam.setup(720, 576);

	if (cam.isInitialized())
	{
		camOn = true;
	}
	
	extVideos.push_back(".mp4");
	extVideos.push_back(".avi");
	extVideos.push_back(".flv");
	extVideos.push_back(".mov");
	extVideos.push_back(".mkv");
	extVideos.push_back(".mpg");
	extImages.push_back(".bmp");
	extImages.push_back(".png");
	extImages.push_back(".gif");
	extImages.push_back(".jpg");

	// system("sudo mount /dev/sda1 /media/pi/USBKey -o uid=pi,gid=pi");
	getVideos(path, extVideos, videos);
	getImages(path, extImages, images);

	if (images.size() == 0 && videos.size() == 0 && !cam.isInitialized())
	{
		noInput = true;
	}
	else if (videos.size() > 0)
	{
		player.load(videos[num]);
		player.setLoopState(OF_LOOP_NORMAL);
		player.play();
	}

	if (images.size() > 0)
	{
		paintImage.load(images[0]);
		if (videos.size() == 0 && !cam.isInitialized())
		{
			paintMode = true;
		}
	}
	ofLog() << textureBuffer.getWidth() << " " << textureBuffer.getHeight();
}

//--------------------------------------------------------------

void ofApp::update()
{
	// powerOffCheckRoutine();

	if (camOn)
	{
		cam.update();
		if (cam.isFrameNew())
		{
			textureBuffer.begin();
			textureShader.begin();
			textureShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			textureShader.setUniformTexture("tex0", cam.getTexture(), 0);
			// cam.draw(0, 0, ofGetWidth(), ofGetHeight());
			ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
			textureShader.end();
			textureBuffer.end();
		}
	}
	else if (!paintMode)
	{
		player.update();
		if (player.isFrameNew())
		{
			textureBuffer.begin();
			textureShader.begin();
			textureShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			textureShader.setUniformTexture("tex0", player.getTexture(), 0);
			// player.draw(0, 0, ofGetWidth(), ofGetHeight());
			ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
			textureShader.end();
			textureBuffer.end();
		}
	}
	else
	{
		textureBuffer.begin();
		ofClear(255, 255, 255, 0);
		ofPushMatrix();
		ofTranslate(paintImage.getWidth() / 2, paintImage.getHeight() / 2);
		ofTranslate(0, 0, paintZoom);
		ofTranslate(-paintDispX, -paintDispY);
		ofRotateZDeg(paintRotate);
		paintImage.draw(-paintImage.getWidth() / 2, -paintImage.getHeight() / 2);
		ofPopMatrix();
		textureBuffer.end();
	}

	checkClicksRoutine();
	checkJoysticksRoutine();
	updateControlsRoutine();
	checkVideoPlayback();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// set the fill colour
	ofSetColor(ofColor::white);
	if (noInput == true)
	{
		image1.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	else
	{

		buffer.begin();
		switch (selector)
		{
		case 1: // CONTRAST
			shader1.begin();
			shader1.setUniform1f("in1", knob);
			shader1.setUniform1f("dispX", dispX * 0.04);
			shader1.setUniform1f("dispY", dispY * 0.04);
			shader1.setUniform1f("in2", yAxis2);
			shader1.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofRotateZDeg((int)rotate);
			ofTranslate(0, 0, zoom);
			textureBuffer.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);
			ofPopMatrix();
			shader1.end();
			break;

		case 2: // CHANGE HUE
			shader2.begin();
			shader2.setUniform1f("in1", yAxis2);
			shader2.setUniform1f("in2", knob);
			shader2.setUniform1f("dispX", dispX * 0.04);
			shader2.setUniform1f("dispY", dispY * 0.04);
			shader2.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofRotateZDeg((int)rotate);
			ofTranslate(0, 0, zoom);
			textureBuffer.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);
			ofPopMatrix();
			shader2.end();
			break;

		case 3: // NEGATIVE
			shader3.begin();
			shader3.setUniform1f("dispX", dispX * 0.04);
			shader3.setUniform1f("dispY", dispY * 0.04);
			shader3.setUniform1f("in1", yAxis2);
			shader3.setUniform1f("in2", knob);
			shader3.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofTranslate(0, 0, zoom);
			ofRotateZDeg((int)rotate);
			textureBuffer.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);
			ofPopMatrix();
			shader3.end();
			break;
		case 4: // PIXELATE
			shader4.begin();
			shader4.setUniform1f("in1", knob);
			shader4.setUniform1f("in2", yAxis2);
			shader4.setUniform1f("dispX", dispX * 0.04);
			shader4.setUniform1f("dispY", dispY * 0.04);
			shader4.setUniform1f("textureWidth", (float)ofGetWidth());
			shader4.setUniform1f("textureHeight", (float)ofGetHeight());
			shader4.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofTranslate(0, 0, zoom);
			ofRotateZDeg((int)rotate);
			textureBuffer.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);
			ofPopMatrix();
			shader4.end();
			break;
		case 5: // pure feedback with speed control

			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofRotateZDeg((int)rotate);
			ofTranslate(-dispX * 10, -dispY * 10, zoom);
			if (ofGetFrameNum() % (uint64_t)(1.0 + (knob * 3)) == 0)
			{
				textureBuffer.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);
			}
			ofPopMatrix();
			break;
		}

		buffer.end();

		buffer2.begin();
		ofClear(255, 255, 255, 0);

		// LUMA KEY
		lumakey.begin();
		lumakey.setUniform1f("th1", keyTresh);
		lumakey.setUniform1f("op1", 0.0);

		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
		ofTranslate(-sourceDispX * 600, -sourceDispY * 600, sourceZoom);
		ofRotateZDeg((int)sourceRotate);
		ofTranslate(-ofGetWidth() / 2, -ofGetHeight() / 2, 0);

		drawSource();

		ofPopMatrix();

		lumakey.end();
		buffer2.end();

		buffer3.begin();
		buffer.draw(0, 0);
		buffer2.draw(0, 0);
		buffer3.end();

		// Load here the feedback
		textureBuffer.begin();
		buffer3.draw(0, 0, ofGetWidth(), ofGetHeight());
		textureBuffer.end();
		// textureBuffer.draw(0, 0, ofGetWidth(), ofGetHeight());
		// texture1 = buffer3.getTexture();

		// FINAL HUE + SAT ADJUST
		shader6.begin();
		shader6.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
		if (selector == 5)
		{ // if in no effect mode you can adjust saturation also.
			shader6.setUniform1f("in2", yAxis2);
		}
		else if (selector == 2)
		{
			shader6.setUniform1f("in2", 0.65);
		}
		else
		{
			shader6.setUniform1f("in2", 1.3);
		}
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		shader6.setUniform1f("hue", xAxis2);

		buffer.draw(0, 0);
		shader6.end();

		buffer2.draw(0, 0);
	}
	if (debug)
	{
		// draw our debug surfaces
		ofSetColor(ofColor::green);
		ofDrawBitmapString("fps: " + ofToString((int)ofGetFrameRate()) + ", clicks1: " + ofToString(nClicks1) + ", clicks2: " + ofToString(nClicks2), 20, 20);
		ofSetColor(ofColor::white);
		player.draw(20, 50, ofGetWidth() / 4, ofGetHeight() / 4);
		cam.draw(ofGetWidth() - (ofGetWidth() / 4) - 20, 50, ofGetWidth() / 4, ofGetHeight() / 4);
		textureBuffer.draw((ofGetWidth() / 2) - ((ofGetWidth() / 4) / 2), (ofGetHeight() / 4) + 50, ofGetWidth() / 4, ofGetHeight() / 4);

		// buttons
		ofSetColor(ofMap(button1Value, 0, 1, 0, 255), ofMap(button1Value, 0, 1, 0, 255), ofMap(button1Value, 0, 1, 0, 255), 255);
		ofDrawCircle(20, ofGetHeight() - 40, 5);
		ofSetColor(ofMap(button2Value, 0, 1, 0, 255), ofMap(button2Value, 0, 1, 0, 255), ofMap(button2Value, 0, 1, 0, 255), 255);
		ofDrawCircle(35, ofGetHeight() - 40, 5);

		// potentiometers
		ofSetColor(ofMap(pot1Value, 0, 1, 0, 255), ofMap(pot1Value, 0, 1, 0, 255), ofMap(pot1Value, 0, 1, 0, 255), 255);
		ofDrawCircle(20, ofGetHeight() - 25, 5);
		ofSetColor(ofMap(pot2Value, 0, 1, 0, 255), ofMap(pot2Value, 0, 1, 0, 255), ofMap(pot2Value, 0, 1, 0, 255), 255);
		ofDrawCircle(35, ofGetHeight() - 25, 5);
		ofSetColor(ofMap(pot3Value, 0, 1, 0, 255), ofMap(pot3Value, 0, 1, 0, 255), ofMap(pot3Value, 0, 1, 0, 255), 255);
		ofDrawCircle(50, ofGetHeight() - 25, 5);
		ofSetColor(ofMap(pot4Value, 0, 1, 0, 255), ofMap(pot4Value, 0, 1, 0, 255), ofMap(pot4Value, 0, 1, 0, 255), 255);
		ofDrawCircle(65, ofGetHeight() - 25, 5);

		// joystick 1
		ofSetColor(ofMap(xAxis1, 0, 1, 0, 255), ofMap(xAxis1, 0, 1, 0, 255), ofMap(xAxis1, 0, 1, 0, 255), 255);
		ofDrawCircle(20, ofGetHeight() - 10, 5);
		ofSetColor(ofMap(yAxis1, 0, 1, 0, 255), ofMap(yAxis1, 0, 1, 0, 255), ofMap(yAxis1, 0, 1, 0, 255), 255);
		ofDrawCircle(35, ofGetHeight() - 10, 5);
		// joystick 2
		ofSetColor(ofMap(xAxis2, 0, 1, 0, 255), ofMap(xAxis2, 0, 1, 0, 255), ofMap(xAxis2, 0, 1, 0, 255), 255);
		ofDrawCircle(50, ofGetHeight() - 10, 5);
		ofSetColor(ofMap(yAxis2, 0, 1, 0, 255), ofMap(yAxis2, 0, 1, 0, 255), ofMap(yAxis2, 0, 1, 0, 255), 255);
		ofDrawCircle(65, ofGetHeight() - 10, 5);
		// ofLog(OF_LOG_NOTICE, "fps: " + ofToString(ofGetFrameRate()) + " xAxis1: " + ofToString(xAxis1) + " yAxis1:" + ofToString(yAxis1) + " xAxis2:" + ofToString(xAxis2) + " yAxis2:" + ofToString(yAxis2));
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'd')
	{
		debug = !debug;
	}
}

//--------------------------------------------------------------
void ofApp::checkClicksRoutine()
{
	uint64_t currentTime = ofGetElapsedTimeMillis();

	// Handle Button 1 clicks
	if (button1Value)
	{
		if (!clickCounter1)
		{ // Start counting clicks
			clickCounter1 = 1;
			nClicks1 = 0;
			ofLogNotice() << "Button 1 pressed, starting click counter.";
		}
		if (currentTime - beginTime > timer)
		{ // Debounce logic
			beginTime = currentTime;
			nClicks1++;
			ofLogNotice() << "Button 1 click count: " << nClicks1;
		}
	}
	else if (clickCounter1)
	{ // Button released
		if (currentTime - beginTime > timer)
		{					   // Time elapsed since last click
			clickCounter1 = 0; // Reset the counter
			ofLogNotice() << "Button 1 released, total clicks: " << nClicks1;

			switch (nClicks1)
			{
			case 1:
				xAxis1 = 0;
				yAxis1 = 0; // Reset position
				break;
			case 2:
				sourceParamMode = !sourceParamMode; // Toggle mode
				if (sourceParamMode)
				{
					sourceZoomWaitRecall = true;
					sourceRotateWaitRecall = true;
					xAxis1 = sourceDispX;
					yAxis1 = sourceDispY;
				}
				else
				{
					feedbackZoomWaitRecall = true;
					feedbackRotateWaitRecall = true;
					xAxis1 = dispX;
					yAxis1 = dispY;
				}
				break;
			case 3:
				(!paintMode) ? nextVideo() : nextImage(); // Next video/image
				break;
			default:
				break;
			}
			nClicks1 = 0; // Reset click count
		}
	}

	// Handle Button 2 clicks (similar logic to Button 1)
	if (button2Value)
	{
		if (!clickCounter2)
		{
			clickCounter2 = 1;
			nClicks2 = 0;
			ofLogNotice() << "Button 2 pressed, starting click counter.";
		}
		if (currentTime - beginTime > timer)
		{
			beginTime = currentTime;
			nClicks2++;
			ofLogNotice() << "Button 2 click count: " << nClicks2;
		}
	}
	else if (clickCounter2)
	{
		if (currentTime - beginTime > timer)
		{
			clickCounter2 = 0;
			ofLogNotice() << "Button 2 released, total clicks: " << nClicks2;

			switch (nClicks2)
			{
			case 1:
				xAxis2 = 0.0;
				yAxis2 = 0.5;
				buffer.begin();
				ofClear(255, 255, 255, 0);
				buffer.end();
				buffer2.begin();
				ofClear(255, 255, 255, 0);
				buffer2.end();
				buffer3.begin();
				ofClear(255, 255, 255, 0);
				buffer3.end();
				break;
			case 2:
				selector = (selector < 5) ? selector + 1 : 1; // Change feedback mode
				break;
			case 3:
				if (!images.empty())
				{
					paintMode = !paintMode;
					if (paintMode)
					{
						xAxis1 = 0;
						yAxis1 = 0;
					}
					else
					{
						feedbackRotateWaitRecall = true;
						feedbackZoomWaitRecall = true;
						xAxis1 = dispX;
						yAxis1 = dispY;
					}
				}
				break;
			case 4:
				if (cam.isInitialized() && !videos.empty())
					camOn = !camOn; // Toggle camera mode
				break;
			default:
				break;
			}
			nClicks2 = 0;
		}
	}
}

//--------------------------------------------------------------
void ofApp::checkJoysticksRoutine()
{
	if (!paintMode && !sourceParamMode)
	{
		dispX = xAxis1;
		dispY = yAxis1;
	}
	else if (paintMode)
	{
		paintDispX = xAxis1 * (ofGetWidth() + (paintImage.getWidth() / 5));
		paintDispY = yAxis1 * (ofGetHeight() + (paintImage.getHeight() / 5));
	}
	else if (sourceParamMode)
	{
		sourceDispX = xAxis1;
		sourceDispY = yAxis1;
	}
	if (pot1Value > 535 || pot1Value < 470)
	{
		// The last two values in the following method determine the sensibility of the joystick
		xAxis1 += ofMap((float)pot1Value, 0.0, 1023.0, 0.05, -0.05);
		// These are the bounds of the frame movement
		if (xAxis1 > 1.00)
			xAxis1 = 1.00;
		if (xAxis1 < -1.00)
			xAxis1 = -1.00;
	}
	if (pot2Value > 535 || pot2Value < 470)
	{
		yAxis1 += ofMap((float)pot2Value, 0.0, 1023.0, -0.05, 0.05);
		if (yAxis1 > 1.0)
			yAxis1 = 1.00;
		if (yAxis1 < -1.0)
			yAxis1 = -1.00;
	}

	if (pot3Value > 535 || pot3Value < 470)
	{
		xAxis2 += ofMap((float)pot3Value, 0.0, 1023.0, -0.015, +0.015);
		//*DON'T*adjust these to alter the bounds of the parameter
		// keep this 1.0 and 0.0 and alter the shader parameters
		if (xAxis2 > 1.0)
			xAxis2 = 0.0;
		if (xAxis2 < 0.0)
			xAxis2 = 1.0;
	}
	if (pot4Value > 535 || pot4Value < 470)
	{
		yAxis2 += ofMap((float)pot4Value, 0.0, 1023.0, -0.017, +0.017);
		if (yAxis2 > 1.0)
			yAxis2 = 1.0;
		if (yAxis2 < 0.0)
			yAxis2 = 0.0;
	}
}

//--------------------------------------------------------------
/*
void ofApp::updateControlsRoutine()
{
	keyTresh = ofMap((float)pot2Value, 0.0, 1023.0, 0, 1.0);
	if (keyTresh > -0.03 && keyTresh < 0.03)
	{
		keyTresh = 0.0;
	}

	knob = ofMap((float)pot4Value, 0.0, 1023.0, 0.0, 1.0);
	if (!paintMode && !sourceParamMode)
	{
		if (abs(zoom - ofMap(pot1Value, 0, 1023, -50, +50)) < 10)
		{
			feedbackZoomWaitRecall = false;
		}
		if (!feedbackZoomWaitRecall)
		{
			zoom = ofMap(pot1Value, 0, 1023, -50, +50);
		}
		if (abs(rotate - ofMap((float)pot3Value, 0, 1023, -10.0, 10.0)) < 2.5)
		{
			feedbackRotateWaitRecall = false;
		}
		if (!feedbackRotateWaitRecall)
		{
			rotate = ofMap((float)pot3Value, 0, 1023, -10.0, 10.0);
		}
	}
	if (sourceParamMode)
	{
		// Don't change the parameters value until the potentiometer position
		// almost matches the stored value
		if (abs(oldSourceRotate - ofMap((float)pot3Value, 0, 1023, -180, 180)) < 5)
		{
			sourceRotateWaitRecall = false;
		}
		if (!sourceRotateWaitRecall)
		{
			// the following code avoids jump in the parameters
			// removing input noise
			sourceRotate = ofMap((float)pot3Value, 0, 1023, -180, 180);
			if (abs(sourceRotate) < 1)
			{
				sourceRotate = 0;
			}
			if (abs(sourceRotate - oldSourceRotate) < 3)
			{
				sourceRotate = oldSourceRotate;
			}
			oldSourceRotate = sourceRotate;
		}
		if (abs(oldSourceZoom - ofMap(pot1Value, 0, 1023, -600, +600)) < 30)
		{
			sourceZoomWaitRecall = false;
		}
		if (!sourceZoomWaitRecall)
		{
			sourceZoom = ofMap(pot1Value, 0, 1023, -600, +600);
			if (abs(sourceZoom - oldSourceZoom) < 30)
			{
				sourceZoom = oldSourceZoom;
			}
			oldSourceZoom = sourceZoom;
		}
	}
	// If in paintmode map the controls to paintparams
	//  and then set dispX and dispY to 0 so that
	// the frame buffer is centered
	if (paintMode)
	{
		paintRotate = ofMap(pot3Value, 0, 1023, -180, 180);
		if (abs(paintRotate) < 1)
		{
			paintRotate = 0;
		}
		if (abs(paintRotate - oldPaintRotate) < 2)
		{
			paintRotate = oldPaintRotate;
		}
		oldPaintRotate = paintRotate;
		paintZoom = ofMap(pot1Value, 0, 1023, -600, 600);
		if (abs(paintZoom - oldPaintZoom) < 20)
		{
			paintZoom = oldPaintZoom;
		}
		oldPaintZoom = paintZoom;
		dispX = 0;
		dispY = 0;
	}
}
*/

//--------------------------------------------------------------
void ofApp::updateControlsRoutine()
{
	for (auto &kv : midiCCValues)
	{
		// clicks
		if (kv.first == 58)
		{
			if (kv.second == 1)
			{
				button1Value = 1;
				// nClicks1 = 1;
			}
			else
			{
				button1Value = 0;
			}
		}
		if (kv.first == 59)
		{
			if (kv.second == 1)
			{
				button2Value = 1;
				// nClicks2 = 1;
			}
			else
			{
				button2Value = 0;
			}
		}
		// joystick 1
		if (kv.first == 0)
		{
			xAxis1 = kv.second;
		}
		if (kv.first == 1)
		{
			yAxis1 = kv.second;
		}
		// joystick 2
		if (kv.first == 2)
		{
			xAxis2 = kv.second;
		}
		if (kv.first == 3)
		{
			yAxis2 = kv.second;
		}
		// potentiometers
		if (kv.first == 16)
		{
			pot1Value = kv.second;
		}
		if (kv.first == 17)
		{
			pot2Value = kv.second;
		}
		if (kv.first == 18)
		{
			pot3Value = kv.second;
		}
		if (kv.first == 19)
		{
			pot4Value = kv.second;
		}
	}
}

//--------------------------------------------------------------
void ofApp::nextVideo()
{
	if (videos.size() > 0)
	{
		if (num + 1 < videos.size())
		{
			num += 1;
		}
		else
		{
			num = 0;
		}
		player.load(videos[num]);
	}
}

//--------------------------------------------------------------
void ofApp::drawSource()
{
	if (camOn)
	{
		cam.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	else if (!paintMode)
	{
		player.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	else
	{
		ofPushMatrix();
		ofTranslate(paintImage.getWidth() / 2, paintImage.getHeight() / 2);
		ofTranslate(0, 0, paintZoom);
		ofTranslate(-paintDispX, -paintDispY);
		ofRotateZDeg(paintRotate);
		paintImage.draw(-paintImage.getWidth() / 2, -paintImage.getHeight() / 2);
		ofPopMatrix();
	}
}

//--------------------------------------------------------------
void ofApp::nextImage()
{
	if (images.size() > 0)
	{
		if (imageNum + 1 < images.size())
		{
			imageNum += 1;
		}
		else
		{
			imageNum = 0;
		}
		paintImage.load(images[imageNum]);
	}
}

//--------------------------------------------------------------
void ofApp::powerOffCheckRoutine()
{
	// If both button remain pressed for 3 seconds the device turns off
	/*
	if (button1.currentValue && button2.currentValue)
	{

		if (!turnOffStarted)
		{
			beginTime = ofGetSystemTimeMillis();
			turnOffStarted = true;
		}
		else
		{
			if (ofGetSystemTimeMillis() - beginTime > 2000)
			{
				// system("sudo killall -9 feedback; sudo poweroff");
			}
		}
	}
	else
	{
		turnOffStarted = false;
	}
	*/
}

//--------------------------------------------------------------
void ofApp::checkVideoPlayback()
{
	if (videos.size() > 0)
	{
		if (player.isFrameNew())
		{
			notLoading = false;
		}
		else if (!player.isFrameNew() && !notLoading && !camOn)
		{
			notLoading = true;
			loadCount = ofGetSystemTimeMillis();
		}
		else if (!player.isFrameNew() && notLoading && !camOn)
		{
			if (ofGetSystemTimeMillis() - loadCount >= 1000 && loadCount > 0)
			{
				ofLog(OF_LOG_NOTICE, "***VIDEO WASN'T LOADING, SKIPPED IT!***");
				notLoading = false;
				loadCount = 0;
				nextVideo();
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage &message)
{
	if (message.status == MIDI_CONTROL_CHANGE)
	{
		midiCCValues[message.control] = message.value / 127.0f; // Normalize 0–127 to 0.0–1.0
	}
}

//--------------------------------------------------------------
// std::string path("/media/pi/USBKey");
// These two methods look through the directories in the specifed folder and selects video/image
// files with the specified extensions.
void ofApp::getVideos(const std::string &path, stringvec &ext, stringvec &vector)
{
	for (auto &p : fs::recursive_directory_iterator(path))
	{
		for (int i = 0; i < ext.size(); i++)
		{
			if (boost::iequals(p.path().extension().string(), ext[i]))
			{
				vector.push_back(p.path().string());
				break;
			}
		}
		std::sort(vector.begin(), vector.end());
	}
}

//--------------------------------------------------------------
void ofApp::getImages(const std::string &path, stringvec &ext, stringvec &vector)
{
	for (auto &p : fs::recursive_directory_iterator(path))
	{
		for (int i = 0; i < ext.size(); i++)
		{
			if (boost::iequals(p.path().extension().string(), ext[i]))
			{
				vector.push_back(p.path().string());
				break;
			}
		}
		std::sort(vector.begin(), vector.end());
	}
}
