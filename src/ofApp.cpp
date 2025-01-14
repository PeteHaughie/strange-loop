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
	ofSetVerticalSync(false);
	shader1.load("shadersES2/shader1");
	shader2.load("shadersES2/shader2");
	shader3.load("shadersES2/shader3");
	shader4.load("shadersES2/shader4");
	lumakey.load("shadersES2/lumakey");
	shader6.load("shadersES2/hueShift");

	// Initialize MIDI
	midiIn.openPort(0); // Use first MIDI port
	midiIn.addListener(this);
	midiIn.setVerbose(true);

	image1.load("images/noInput.png");

	// CONTROL VARIABLES
	yAxis1 = 0.0;
	xAxis1 = 0.0;
	xAxis2 = 0.5;
	yAxis2 = 0.5;
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

	ofHideCursor();
	ofBackground(0, 0, 0);
	ofSetFrameRate(30);
	texture1.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	buffer.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	buffer2.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	buffer3.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	buffer.begin();
	ofClear(255, 255, 255, 0);
	buffer.end();
	buffer2.begin();
	ofClear(255, 255, 255, 0);
	buffer2.end();
	buffer3.begin();
	ofClear(255, 255, 255, 0);
	buffer3.end();

	// The following has to be UNCOMMENTED for NTSC operation ->
	// system("sudo v4l2-ctl -d /dev/video0 --set-standard=0");
	cam.setDeviceID(0);
	cam.setDesiredFrameRate(30);
	cam.initGrabber(720, 480);
	// The following has to be UNCOMMENTED for PAL operation ->
	// system("sudo v4l2-ctl -d /dev/video0 --set-standard=6");
	// cam.setDeviceID(0);
	// cam.setDesiredFrameRate(25);
	// cam.initGrabber(720, 576);

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
}

//--------------------------------------------------------------

void ofApp::update()
{
	// powerOffCheckRoutine();

	if (camOn)
	{
		cam.update();
	}

	checkClicksRoutine();
	checkJoysticksRoutine();
	updateControlsRoutine();
	checkVideoPlayback();
	player.update();
	if (player.isFrameNew())
	{
		texture1 = player.getTexture();
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofLog(OF_LOG_NOTICE, "fps: " + ofToString(ofGetFrameRate()) + " xAxis1: " + ofToString(xAxis1) + " yAxis1:" + ofToString(yAxis1) + " xAxis2:" + ofToString(xAxis2) + " yAxis2:" + ofToString(yAxis2));
	if (!texture1.isAllocated())
	{
		return;
	}
	if (noInput == true)
	{
		image1.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	else
	{

		// draw our debug surfaces
		player.draw(20, 10, ofGetWidth() / 4, ofGetHeight() / 4);
		texture1.draw((ofGetWidth() / 2) - ((ofGetWidth() / 4) - (ofGetWidth() / 4)), 10, ofGetWidth() / 4, ofGetHeight() / 4);
		cam.draw(ofGetWidth() - (ofGetWidth() / 4) - 20, 10, ofGetWidth() / 4, ofGetHeight() / 4);

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

		// reset the fill colour
		ofSetColor(ofColor::white);

		buffer.begin();

		switch (selector)
		{
		case 1: // CONTRAST
			shader1.begin();
			shader1.setUniform1f("in1", knob);
			shader1.setUniform1f("dispX", dispX * 0.04);
			shader1.setUniform1f("dispY", dispY * 0.04);
			shader1.setUniform1f("in2", yAxis2);
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofRotateZDeg((int)rotate);
			ofTranslate(0, 0, zoom);

			texture1.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);

			ofPopMatrix();
			shader1.end();
			break;

		case 2: // CHANGE HUE
			shader2.begin();
			shader2.setUniform1f("in1", yAxis2);
			shader2.setUniform1f("in2", knob);
			shader2.setUniform1f("dispX", dispX * 0.04);
			shader2.setUniform1f("dispY", dispY * 0.04);

			ofPushMatrix();

			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofRotateZDeg((int)rotate);
			ofTranslate(0, 0, zoom);

			texture1.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);

			ofPopMatrix();
			shader2.end();
			break;

		case 3: // NEGATIVE
			shader3.begin();
			shader3.setUniform1f("dispX", dispX * 0.04);
			shader3.setUniform1f("dispY", dispY * 0.04);
			shader3.setUniform1f("in1", yAxis2);
			shader3.setUniform1f("in2", knob);

			ofPushMatrix();

			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofTranslate(0, 0, zoom);
			ofRotateZDeg((int)rotate);

			texture1.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);

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
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
			ofTranslate(0, 0, zoom);

			ofRotateZDeg((int)rotate);
			texture1.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);

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
				texture1.draw(-ofGetWidth() / 2, -ofGetHeight() / 2);
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
		texture1 = buffer3.getTexture();

		// FINAL HUE+SAT ADJUST
		shader6.begin();

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
		shader6.setUniform1f("hue", xAxis2);

		buffer.draw(0, 0);
		shader6.end();

		buffer2.draw(0, 0);
	}
}

//--------------------------------------------------------------
void ofApp::checkClicksRoutine()
{
	switch (nClicks1)
	{
	case 1:
		// Center the framebuffer position
		xAxis1 = 0;
		yAxis1 = 0;
		nClicks1 = 0;
		break;

	case 2:
		// Change source position, rotation and zoom, or go back
		// to changing parameters in the feedback
		if (!paintMode)
		{
			sourceParamMode = !sourceParamMode;
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
		}
		nClicks1 = 0;
		break;

	case 3:
		// Jump to next video
		if (!paintMode)
		{
			nextVideo();
		}
		else
		{
			nextImage();
		}
		nClicks1 = 0;
		break;

	default:
		nClicks1 = 0;
		break;
	}

	switch (nClicks2)
	{
	case 1:
		// Zero out the feedback parameters and clear the
		// frame buffer
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
		nClicks2 = 0;
		break;

	case 2:
		// Change feedback mode
		if (selector < 5)
		{
			selector++;
		}
		else
		{
			selector = 1;
		}
		nClicks2 = 0;
		break;
	case 3:
		// Go to paint mode (if images are present) and back
		if (images.size() > 0)
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
				yAxis1 = dispX;
			}
		}
		nClicks2 = 0;
		break;
	case 4:
		// switch from capture to stored videos mode if possible
		if (cam.isInitialized() && videos.size() > 0)
		{
			camOn = !camOn;
		}
		nClicks2 = 0;
		break;
	default:
		nClicks2 = 0;
		break;
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
				nClicks1++;
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
				nClicks2++;
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
	// 	keyTresh = midiCCValues[1];															// Example: CC1 for key threshold
	// 	knob = midiCCValues[2];																	// Example: CC2 for knob value
	// 	zoom = ofMap(midiCCValues[3], 0.0, 1.0, -50, +50);			// CC3 mapped to zoom
	// 	rotate = ofMap(midiCCValues[4], 0.0, 1.0, -10.0, 10.0); // CC4 mapped to rotate
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
