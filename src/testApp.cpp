#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofEnableAlphaBlending();
	ofSetupScreen();
	ofBackground(0, 0, 0);
	ofSetVerticalSync(true);
	
	int nChannels = 2;
	bufferSize = 512;
	sampleRate = 44100;
	
	superCollider.setup(nChannels, bufferSize, sampleRate);
	
	ofSoundStreamSetup(nChannels, 0, this, sampleRate, bufferSize, 4);
	
}

testApp::~testApp() {
}


//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){

}

void testApp::audioRequested 	(float * output, int bufferSize, int nChannels){
	//make some white noise to process
	for(int i=0; i < bufferSize * nChannels; i++) output[i] = ofRandomf();
	
	//if generating sound only in SC, then clear the buffer each time
//	for(int i=0; i < bufferSize * nChannels; i++) output[i] = 0;

	//important: make sure there are no NaNs in the input, as this can *&^% up ugens in unpredictable ways
	for(int i=0; i < bufferSize * nChannels; i++) if (isnan(output[i])) output[i] = 0;

	superCollider.process(output, bufferSize, nChannels);	
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}