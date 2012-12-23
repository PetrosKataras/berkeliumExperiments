#include "SimpleBerkelium.h"

static const bool USE_TRANSPARENCY = true;

using namespace Berkelium;

SimpleBerkelium::SimpleBerkelium(int width, int height, bool useTransparency) : w(width), h(height), transparent(useTransparency){
    bk_texture_window = new GLTextureWindow(w, h, transparent);
    bk_texture_window->window()->focus();
}

SimpleBerkelium::~SimpleBerkelium(){

}

void SimpleBerkelium::setup(){
}

void SimpleBerkelium::update(){
    Berkelium::update();
}

void SimpleBerkelium::draw(){
    glClear( GL_COLOR_BUFFER_BIT );
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
        ofTranslate(0, 0);
        glColor3f(1.f, 1.f, 1.f);
        bk_texture_window->bind();
            ofEnableNormalizedTexCoords();
            glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f); glVertex3f(0.0f, 0.0f, 0.0f);
            glTexCoord2f(0.f, 1.f); glVertex3f(0.0f,  h, 0.0f);
            glTexCoord2f(1.f, 1.f); glVertex3f(w,  h, 0.0f);
            glTexCoord2f(1.f, 0.f); glVertex3f(w, 0.0f, 0.0f);
            glEnd();
            ofDisableNormalizedTexCoords();
        bk_texture_window->release();
    glPopMatrix();
}

void SimpleBerkelium::loadURL(std::string url){
    if (bk_texture_window == NULL)
        return;

    bk_texture_window->clear();

    bk_texture_window->getWindow()->navigateTo(url.data(), url.length());
}

void SimpleBerkelium::mouseDragged(int x, int y, int button){
    bk_texture_window->getWindow()->mouseMoved(x,y);
}


void SimpleBerkelium::mouseMoved(int x, int y){
    bk_texture_window->getWindow()->mouseMoved(x,y);
}

void SimpleBerkelium::mousePressed(int x, int y, int button){
    bk_texture_window->getWindow()->mouseButton(button, true);
}

void SimpleBerkelium::mouseReleased(int x, int y, int button){
    bk_texture_window->getWindow()->mouseButton(button, false);
}

void SimpleBerkelium::textEvent(int key){

        if (key ==']'){
            key = 0x6708;
        }

        if (key == '['){
            key = 0x201d1;
        }

        // Some keys that come through this are actually special keys, despite being
        // represented as ASCII characters.
        if (key == '\b' || key == '\r' || key == '\n' || key == ' ' || key == 127 ||
            key >= 'a' && key <= 'z' || key >= 'A' && key <= 'Z') {
            bool pressed = true;
            int wvmods = mapGLUTModsToBerkeliumMods(glutGetModifiers());
            int vk_code = key == 127 ? BK_KEYCODE_DELETE : tolower(key);
            int scancode = 0;

            bk_texture_window->getWindow()->keyEvent(pressed, wvmods, vk_code, scancode);
        }

        if (!isASCIISpecialToBerkelium(key)){
            wchar_t w_key[2];
            w_key[0] = key;
            w_key[1] = 0;
            bk_texture_window->getWindow()->textEvent(w_key, 1);
        }
}


void SimpleBerkelium::specialTextEvent(int key){

            bool pressed = true;
            int wvmods = mapGLUTModsToBerkeliumMods(glutGetModifiers());
            int vk_code = mapGLUTKeyToBerkeliumKey(key);
            int scancode = 0;

            bk_texture_window->getWindow()->keyEvent(pressed, wvmods, vk_code, scancode);
}

void SimpleBerkelium::handleText(int key){

    switch (key){

        case OF_KEY_LEFT:
        case OF_KEY_RIGHT:
        case OF_KEY_UP:
        case OF_KEY_DOWN:
        case OF_KEY_PAGE_UP:
        case OF_KEY_PAGE_DOWN:
        case OF_KEY_END:
            this->specialTextEvent(key);
            break;
        default:
            this->textEvent(key);
            break;
    }
}


void SimpleBerkelium::exit(){
    Berkelium::destroy();
}


