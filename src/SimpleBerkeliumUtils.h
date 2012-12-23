#ifndef _SIMPLE_BERKELIUM_UTILS_H_
#define _SIMPLE_BERKELIUM_UTILS_H_

#include "berkelium/Berkelium.hpp"
#include "berkelium/Window.hpp"
#include "berkelium/ScriptUtil.hpp"

#include "ofMain.h"

#include <GL/glut.h>

#define DEBUG_PAINT false

using namespace Berkelium;


/** Handles an onPaint call by mapping the results into an OpenGL texture. The
 *  first parameters are the same as Berkelium::WindowDelegate::onPaint.  The
 *  additional parameters and return value are:
 *  \param dest_texture - the OpenGL texture handle for the texture to render
 *                        the results into.
 *  \param dest_texture_width - width of destination texture
 *  \param dest_texture_height - height of destination texture
 *  \param ignore_partial - if true, ignore any partial updates.  This is useful
 *         if you have loaded a new page, but updates for the old page have not
 *         completed yet.
 *  \param scroll_buffer - a temporary workspace used for scroll data.  Must be
 *         at least dest_texture_width * dest_texture_height * 4 bytes large.
 *  \returns true if the texture was updated, false otherwiase
 */
inline bool mapOnPaintToTexture(
    Berkelium::Window *wini,
    const unsigned char* bitmap_in, const Berkelium::Rect& bitmap_rect,
    size_t num_copy_rects, const Berkelium::Rect *copy_rects,
    int dx, int dy,
    const Berkelium::Rect& scroll_rect,
    int dest_texture,
    int dest_texture_width,
    int dest_texture_height,
    bool ignore_partial,
    char* scroll_buffer) {

    glBindTexture(GL_TEXTURE_2D, dest_texture);

    const int kBytesPerPixel = 4;
    // If we've reloaded the page and need a full update, ignore updates
    // until a full one comes in.  This handles out of date updates due to
    // delays in event processing.
    if (ignore_partial) {
        if (bitmap_rect.left() != 0 ||
            bitmap_rect.top() != 0 ||
            bitmap_rect.right() != dest_texture_width ||
            bitmap_rect.bottom() != dest_texture_height) {
            return false;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, kBytesPerPixel, dest_texture_width, dest_texture_height, 0,
            GL_BGRA, GL_UNSIGNED_BYTE, bitmap_in);
        ignore_partial = false;
        return true;
    }


    // Now, we first handle scrolling. We need to do this first since it
    // requires shifting existing data, some of which will be overwritten by
    // the regular dirty rect update.
    if (dx != 0 || dy != 0) {
        // scroll_rect contains the Rect we need to move
        // First we figure out where the the data is moved to by translating it
        Berkelium::Rect scrolled_rect = scroll_rect.translate(-dx, -dy);
        // Next we figure out where they intersect, giving the scrolled
        // region
        Berkelium::Rect scrolled_shared_rect = scroll_rect.intersect(scrolled_rect);
        // Only do scrolling if they have non-zero intersection
        if (scrolled_shared_rect.width() > 0 && scrolled_shared_rect.height() > 0) {
            // And the scroll is performed by moving shared_rect by (dx,dy)
            Berkelium::Rect shared_rect = scrolled_shared_rect.translate(dx, dy);

            int wid = scrolled_shared_rect.width();
            int hig = scrolled_shared_rect.height();
            if (DEBUG_PAINT) {
              std::cout << "Scroll rect: w=" << wid << ", h=" << hig << ", ("
                        << scrolled_shared_rect.left() << "," << scrolled_shared_rect.top()
                        << ") by (" << dx << "," << dy << ")" << std::endl;
            }
            int inc = 1;
            char *outputBuffer = scroll_buffer;
            // source data is offset by 1 line to prevent memcpy aliasing
            // In this case, it can happen if dy==0 and dx!=0.
            char *inputBuffer = scroll_buffer+(dest_texture_width*1*kBytesPerPixel);
            int jj = 0;
            if (dy > 0) {
                // Here, we need to shift the buffer around so that we start in the
                // extra row at the end, and then copy in reverse so that we
                // don't clobber source data before copying it.
                outputBuffer = scroll_buffer+(
                    (scrolled_shared_rect.top()+hig+1)*dest_texture_width
                    - hig*wid)*kBytesPerPixel;
                inputBuffer = scroll_buffer;
                inc = -1;
                jj = hig-1;
            }

            // Copy the data out of the texture
            glGetTexImage(
                GL_TEXTURE_2D, 0,
                GL_BGRA, GL_UNSIGNED_BYTE,
                inputBuffer
            );

            // Annoyingly, OpenGL doesn't provide convenient primitives, so
            // we manually copy out the region to the beginning of the
            // buffer
            for(; jj < hig && jj >= 0; jj+=inc) {
                memcpy(
                    outputBuffer + (jj*wid) * kBytesPerPixel,
//scroll_buffer + (jj*wid * kBytesPerPixel),
                    inputBuffer + (
                        (scrolled_shared_rect.top()+jj)*dest_texture_width
 + scrolled_shared_rect.left()) * kBytesPerPixel,
                    wid*kBytesPerPixel
                );
            }

            // And finally, we push it back into the texture in the right
            // location
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                shared_rect.left(), shared_rect.top(),
                shared_rect.width(), shared_rect.height(),
                GL_BGRA, GL_UNSIGNED_BYTE, outputBuffer
            );
        }
    }

    if (DEBUG_PAINT) {
      std::cout << (void*)wini << " Bitmap rect: w="
                << bitmap_rect.width()<<", h="<<bitmap_rect.height()
                <<", ("<<bitmap_rect.top()<<","<<bitmap_rect.left()
                <<") tex size "<<dest_texture_width<<"x"<<dest_texture_height
                <<std::endl;
    }
    for (size_t i = 0; i < num_copy_rects; i++) {
        int wid = copy_rects[i].width();
        int hig = copy_rects[i].height();
        int top = copy_rects[i].top() - bitmap_rect.top();
        int left = copy_rects[i].left() - bitmap_rect.left();
        if (DEBUG_PAINT) {
            std::cout << (void*)wini << " Copy rect: w=" << wid << ", h=" << hig << ", ("
                      << top << "," << left << ")" << std::endl;
        }
        for(int jj = 0; jj < hig; jj++) {
            memcpy(
                scroll_buffer + jj*wid*kBytesPerPixel,
                bitmap_in + (left + (jj+top)*bitmap_rect.width())*kBytesPerPixel,
                wid*kBytesPerPixel
                );
        }

        // Finally, we perform the main update, just copying the rect that is
        // marked as dirty but not from scrolled data.
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        copy_rects[i].left(), copy_rects[i].top(),
                        wid, hig,
                        GL_BGRA, GL_UNSIGNED_BYTE, scroll_buffer
            );
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

/// A few of the most useful keycodes handled below.
enum VirtKeys {
BK_KEYCODE_PRIOR = 0x21,
BK_KEYCODE_NEXT = 0x22,
BK_KEYCODE_END = 0x23,
BK_KEYCODE_HOME = 0x24,
BK_KEYCODE_LEFT = 0x25,
BK_KEYCODE_UP = 0x26,
BK_KEYCODE_RIGHT = 0x27,
BK_KEYCODE_DOWN = 0x28,
BK_KEYCODE_INSERT = 0x2D,
BK_KEYCODE_DELETE = 0x2E

};

/** Given modifiers retrieved from GLUT (e.g. glutGetModifiers), convert to a
 *  form that can be passed to Berkelium.
 */
inline int mapGLUTModsToBerkeliumMods(int modifiers) {
    int wvmods = 0;

    if (modifiers & GLUT_ACTIVE_SHIFT)
        wvmods |= Berkelium::SHIFT_MOD;
    if (modifiers & GLUT_ACTIVE_CTRL)
        wvmods |= Berkelium::CONTROL_MOD;
    if (modifiers & GLUT_ACTIVE_ALT)
        wvmods |= Berkelium::ALT_MOD;

    // Note: GLUT doesn't expose Berkelium::META_MOD

    return wvmods;
}

/** Given an input key from GLUT, convert it to a form that can be passed to
 *  Berkelium.
 */
inline unsigned int mapGLUTKeyToBerkeliumKey(int glut_key) {
    switch(glut_key) {
#define MAP_VK(X, Y) case OF_KEY_##X : return BK_KEYCODE_##Y;
        MAP_VK(INSERT, INSERT);
        MAP_VK(HOME, HOME);
        MAP_VK(END, END);
        MAP_VK(PAGE_UP, PRIOR);
        MAP_VK(PAGE_DOWN, NEXT);
        MAP_VK(LEFT, LEFT);
        MAP_VK(RIGHT, RIGHT);
        MAP_VK(UP, UP);
        MAP_VK(DOWN, DOWN);
    default: return 0;
    }
}


/** Returns true if the ASCII value is considered a special input to Berkelium
 *  which cannot be handled directly via the textEvent method and must be
 *  handled using keyEvent instead.
 */
inline unsigned int isASCIISpecialToBerkelium(unsigned int glut_char) {
    unsigned char ASCII_BACKSPACE = 8;
    unsigned char ASCII_TAB       = 9;
    unsigned char ASCII_ESCAPE    = 27;
    unsigned char ASCII_DELETE    = 127;

    return (glut_char == ASCII_BACKSPACE ||
        glut_char == ASCII_TAB ||
        glut_char == ASCII_ESCAPE ||
        glut_char == ASCII_DELETE
    );
}


/** GLTextureWindow handles rendering a window into a GL texture.  Unlike the
 *  utility methods, this takes care of the entire process and cleanup.
 */
class GLTextureWindow : public Berkelium::WindowDelegate {
public:
    GLTextureWindow(unsigned int _w, unsigned int _h, bool _usetrans)
     : width(_w),
       height(_h),
       needs_full_refresh(true)
    {
        // Create texture to hold rendered view
        glGenTextures(1, &web_texture);
        glBindTexture(GL_TEXTURE_2D, web_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        scroll_buffer = new char[width*(height+1)*4];

        Berkelium::Context *context = Berkelium::Context::create();
        bk_window = Berkelium::Window::create(context);
        delete context;
        bk_window->setDelegate(this);
        bk_window->resize(width, height);
        bk_window->setTransparent(_usetrans);
    }

    ~GLTextureWindow() {
        delete scroll_buffer;
        delete bk_window;
    }

    Berkelium::Window* getWindow() {
        return bk_window;
    }

    void clear() {
        // Black out the page
        unsigned char black = 0;
        glBindTexture(GL_TEXTURE_2D, web_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 1, 1, 0,
            GL_LUMINANCE, GL_UNSIGNED_BYTE, &black);

        needs_full_refresh = true;
    }

    void bind() {
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, web_texture);
    }

    void release() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    virtual void onPaint(Berkelium::Window *wini,
        const unsigned char *bitmap_in, const Berkelium::Rect &bitmap_rect,
        size_t num_copy_rects, const Berkelium::Rect *copy_rects,
        int dx, int dy, const Berkelium::Rect &scroll_rect) {
        bool updated = mapOnPaintToTexture(
            wini, bitmap_in, bitmap_rect, num_copy_rects, copy_rects,
            dx, dy, scroll_rect,
            web_texture, width, height, needs_full_refresh, scroll_buffer
        );
        if (updated) {
            needs_full_refresh = false;
           }
    }

    virtual void onAddressBarChanged(Berkelium::Window *win, URLString newURL) {
        std::cout << (void*)win << "*** onAddressBarChanged " << newURL << std::endl;
    }
    virtual void onStartLoading(Berkelium::Window *win, URLString newURL) {
        std::cout << (void*)win << "*** onStartLoading " << newURL << std::endl;
    }
    virtual void onLoad(Berkelium::Window *win) {
        std::wcout << (void*)win << L"*** onLoad " << std::endl;
    }
    virtual void onCrashedWorker(Berkelium::Window *win) {
        std::wcout << (void*)win << L"*** onCrashedWorker " << std::endl;
    }
    virtual void onCrashedPlugin(Berkelium::Window *win, WideString pluginName) {
        std::wcout << L"*** onCrashedPlugin " << pluginName << std::endl;
    }
    virtual void onProvisionalLoadError(Berkelium::Window *win, URLString url,
                                        int errorCode, bool isMainFrame) {
        std::cout << "*** onProvisionalLoadError " << url << ": " << errorCode;
        if (isMainFrame) std::cout << " (main frame)";
        std::cout << std::endl;
    }
    virtual void onConsoleMessage(Berkelium::Window *win, WideString message,
                                  WideString sourceId, int line_no) {
        std::wcout << L"*** onConsoleMessage " << message << L" from "
                   << sourceId << L" line " << line_no << std::endl;
    }
    virtual void onScriptAlert(Berkelium::Window *win, WideString message,
                              WideString defaultValue, URLString url,
                              int flags, bool &success, WideString &value) {
        std::wcout << L"*** onScriptAlert " << message << std::endl;
    }
    virtual void onNavigationRequested(Berkelium::Window *win, URLString newURL,
                                       URLString referrer, bool isNewWindow,
                                       bool &cancelDefaultAction) {
        std::cout << (void*)win << "*** onNavigationRequested " << newURL << " by " << referrer
                  << (isNewWindow?"  (new window)" : " (same window)") << std::endl;
    }
    virtual void onLoadingStateChanged(Berkelium::Window *win, bool isLoading) {
        std::cout << (void*)win << "*** onLoadingStateChanged "
                  << (isLoading?"started":"stopped") << std::endl;
    }
    virtual void onTitleChanged(Berkelium::Window *win, WideString title) {
        std::wcout << L"*** onTitleChanged " << title << std::endl;
    }
    virtual void onTooltipChanged(Berkelium::Window *win, WideString text) {
        std::wcout << L"*** onTooltipChanged " << text << std::endl;
    }
    virtual void onCrashed(Berkelium::Window *win) {
        std::cout << "*** onCrashed " << std::endl;
    }
    virtual void onUnresponsive(Berkelium::Window *win) {
        std::cout << "*** onUnresponsive " << std::endl;
    }
    virtual void onResponsive(Berkelium::Window *win) {
        std::cout << "*** onResponsive " << std::endl;
    }
    virtual void onCreatedWindow(Berkelium::Window *win, Berkelium::Window *newWindow,
                                 const Rect &initialRect) {
        std::cout << (void*)win << "*** onCreatedWindow " << (void*)newWindow<<" "
                  << initialRect.mLeft << "," << initialRect.mTop << ": "
                  << initialRect.mWidth << "x" << initialRect.mHeight << std::endl;
        if (initialRect.mWidth < 1 || initialRect.mHeight < 1) {
            newWindow->resize(this->width, this->height);
        }
        newWindow->setDelegate(this);
    }
    virtual void onWidgetCreated(Berkelium::Window *win, Widget *newWidget, int zIndex) {
        std::cout << "*** onWidgetCreated " << newWidget << " index " << zIndex << std::endl;
    }
    virtual void onWidgetResize(Berkelium::Window *win, Widget *wid, int newWidth, int newHeight) {
        std::cout << "*** onWidgetResize " << wid << " "
                  << newWidth << "x" << newHeight << std::endl;
    }
    virtual void onWidgetMove(Berkelium::Window *win, Widget *wid, int newX, int newY) {
        std::cout << "*** onWidgetMove " << wid << " "
                  << newX << "," << newY << std::endl;
    }
    virtual void onShowContextMenu(Berkelium::Window *win,
                                   const ContextMenuEventArgs& args) {
        std::cout << "*** onShowContextMenu at " << args.mouseX << "," << args.mouseY;
        std::cout << "; type: ";
        switch (args.mediaType) {
          case ContextMenuEventArgs::MediaTypeImage:
            std::cout << "image";
            break;
          case ContextMenuEventArgs::MediaTypeVideo:
            std::cout << "video";
            break;
          case ContextMenuEventArgs::MediaTypeAudio:
            std::cout << "audio";
            break;
          default:
            std::cout << "other";
        }
        if (args.isEditable || args.editFlags) {
            std::cout << " (";
            if (args.isEditable)
                std::cout << "editable; ";
            if (args.editFlags & ContextMenuEventArgs::CanUndo)
                std::cout << "Undo, ";
            if (args.editFlags & ContextMenuEventArgs::CanRedo)
                std::cout << "Redo, ";
            if (args.editFlags & ContextMenuEventArgs::CanCut)
                std::cout << "Cut, ";
            if (args.editFlags & ContextMenuEventArgs::CanCopy)
                std::cout << "Copy, ";
            if (args.editFlags & ContextMenuEventArgs::CanPaste)
                std::cout << "Paste, ";
            if (args.editFlags & ContextMenuEventArgs::CanDelete)
                std::cout << "Delete, ";
            if (args.editFlags & ContextMenuEventArgs::CanSelectAll)
                std::cout << "Select All";
            std::cout << ")";
        }
        std::cout << std::endl;

        if (args.linkUrl.length())
            std::cout << "        Link URL: " << args.linkUrl << std::endl;
        if (args.srcUrl.length())
            std::cout << "        Source URL: " << args.srcUrl << std::endl;
        if (args.pageUrl.length())
            std::cout << "        Page URL: " << args.pageUrl << std::endl;
        if (args.frameUrl.length())
            std::cout << "        Frame URL: " << args.frameUrl << std::endl;
        if (args.selectedText.length())
            std::wcout << L"        Selected Text: " << args.selectedText << std::endl;
    }

    virtual void onJavascriptCallback(Berkelium::Window *win, void* replyMsg, URLString url, WideString funcName, Script::Variant *args, size_t numArgs) {
        std::cout << "*** onJavascriptCallback at URL " << url << ", "
                  << (replyMsg?"synchronous":"async") << std::endl;
        std::wcout << L"    Function name: " << funcName << std::endl;
        for (size_t i = 0; i < numArgs; i++) {
            WideString jsonStr = toJSON(args[i]);
            std::wcout << L"    Argument " << i << ": ";
            if (args[i].type() == Script::Variant::JSSTRING) {
                std::wcout << L"(string) " << args[i].toString() << std::endl;
            } else {
                std::wcout << jsonStr << std::endl;
            }
            Script::toJSON_free(jsonStr);
        }
        if (replyMsg) {
            win->synchronousScriptReturn(replyMsg, numArgs ? args[0] : Script::Variant());
        }
    }

    /** Display a file chooser dialog, if necessary. The value to be returned should go ______.
     * \param win  Window instance that fired this event.
     * \param mode  Type of file chooser expected. See FileChooserType.
     * \param title  Title for dialog. "Open" or "Save" should be used if empty.
     * \param defaultFile  Default file to select in dialog.
     */
    virtual void onRunFileChooser(Berkelium::Window *win, int mode, WideString title, FileString defaultFile) {
        std::wcout << L"*** onRunFileChooser type " << mode << L", title " << title << L":" << std::endl;
#ifdef _WIN32
        std::wcout <<
#else
        std::cout <<
#endif
            defaultFile << std::endl;

        win->filesSelected(NULL);
    }

    virtual void onExternalHost(
        Berkelium::Window *win,
        Berkelium::WideString message,
        Berkelium::URLString origin,
        Berkelium::URLString target)
    {
        std::cout << "*** onExternalHost at URL from "<<origin<<" to "<<target<<":"<<std::endl;
        std::wcout << message<<std::endl;
    }

    Berkelium::Window* window() const {
        return bk_window;
    }

private:
    // The Berkelium window, i.e. our web page
    Berkelium::Window* bk_window;
    // Width and height of our window.
    unsigned int width, height;
    // Storage for a texture
    unsigned int web_texture;
    // Bool indicating when we need to refresh the entire image
    bool needs_full_refresh;
    // Buffer used to store data for scrolling
    char* scroll_buffer;
};

#endif //_SIMPLE_BERKELIUM_UTILS_H_



