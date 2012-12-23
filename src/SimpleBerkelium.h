/* Based on the glut_demo and glut_input examples 
 * that are provided with the berkelium source code.
 */

#ifndef _SIMPLE_BERKELIUM_H_
#define _SIMPLE_BERKELIUM_H_


#include "berkelium/Berkelium.hpp"
#include "berkelium/Window.hpp"
#include "berkelium/WindowDelegate.hpp"
#include "berkelium/Context.hpp"
#include "SimpleBerkeliumUtils.h"

#include "ofMain.h"



using namespace Berkelium;

class SimpleBerkelium {

	public:

        SimpleBerkelium(int width, int height, bool useTransparency);
        ~SimpleBerkelium();

        void setup();
        void update();
        void draw();
        void exit();

        void loadURL(std::string url);

        ///mouse
        void mouseMoved(int x , int y);
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);

        ///text
        void handleText(int key);

	private:

        void textEvent(int key);
        void specialTextEvent(int key);

        GLTextureWindow* bk_texture_window;

        int w, h;
        bool transparent;

}; //SimpleBerkelium


#endif //_SIMPLE_BERKELIUM_H_
