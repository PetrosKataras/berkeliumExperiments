#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);

    Berkelium::init( Berkelium::FileString::empty() );

    browser = new SimpleBerkelium(ofGetWindowWidth(), ofGetWindowHeight(), true);

    browser->loadURL("http://www.google.com/");
}

//--------------------------------------------------------------
void testApp::update(){
    browser->update();
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
