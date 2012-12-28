#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);

    ///!! Before doing anything with Berkelium this should be called first !!
    BerkeliumGlobals::initBerkelium();

    browser = new SimpleBerkelium(ofGetWindowWidth(), ofGetWindowHeight(), true);

    ///You need to preced the url with the protocol used otherwise you wont see anything...
    browser->loadURL("http://www.google.com");
}

//--------------------------------------------------------------
void testApp::update(){
    ///This is global since its not necessary to be called for each object seperately
    BerkeliumGlobals::updateBerkelium();
}

//--------------------------------------------------------------
void testApp::draw(){
    browser->draw();
}

void testApp::exit(){
    browser->exit();
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
    browser->handleText(key);
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    browser->mouseMoved(x,y);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    browser->mouseMoved(x,y);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    browser->mousePressed(x,y,button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    browser->mouseReleased(x, y, button);
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
