#include "sdlutf8truetypefont.hpp"

#include "guichan/exception.hpp"
#include "guichan/image.hpp"

#include <guichan/sdl/sdlgraphics.hpp>

#include "utf8.h"

namespace gcn
{
    SDLUTF8TrueTypeFont::SDLUTF8TrueTypeFont (const std::string& filename, int size)
        :SDLTrueTypeFont(filename, size)
    {

    }

    int SDLUTF8TrueTypeFont::getWidth(const std::string& text) const
    {
        int w;
        if (TTF_SizeUTF8(mFont, text.c_str(), &w, NULL)) {
            return 0;
        } else {
            return w;
        }
    }

    void SDLUTF8TrueTypeFont::drawString(Graphics* graphics, const std::string& text, const int x, const int y)
    {
        if (text == "")
        {
            return;
        }

        SDLGraphics *sdlGraphics = dynamic_cast<SDLGraphics *>(graphics);

        if (sdlGraphics == NULL)
        {
            throw GCN_EXCEPTION("SDLUTF8TrueTypeFont::drawString. Graphics object not an SDL graphics object!");
            return;
        }

        // This is needed for drawing the Glyph in the middle if we have spacing
        int yoffset = getRowSpacing() / 2;

        Color col = sdlGraphics->getColor();

        SDL_Color sdlCol;
        sdlCol.b = col.b;
        sdlCol.r = col.r;
        sdlCol.g = col.g;

        SDL_Surface *textSurface;
        if (mAntiAlias)
        {
            textSurface = TTF_RenderUTF8_Blended(mFont, text.c_str(), sdlCol);
        }
        else
        {
            textSurface = TTF_RenderUTF8_Solid(mFont, text.c_str(), sdlCol);
        }

        SDL_Rect dst, src;
        dst.x = x;
        dst.y = y + yoffset;
        src.w = textSurface->w;
        src.h = textSurface->h;
        src.x = 0;
        src.y = 0;

        sdlGraphics->drawSDLSurface(textSurface, src, dst);
        SDL_FreeSurface(textSurface);
    }

    int SDLUTF8TrueTypeFont::getStringIndexAt(const std::string & text, int x) const
    {
        std::string::const_iterator cur, last;
        int idx = 0;
        if (text.size() == 0) return 0;
        if (x <= 0) return 0;
        
        last = text.begin();
        cur = last;

        try {
            utf8::next(cur, text.end());
        } catch (std::exception* e) {

            throw GCN_EXCEPTION("Invalid UTF-8 text!");
        }

        std::string buff;
        while(cur != text.end()) {
            buff = std::string(text.begin(), cur);
            
            if (getWidth(buff) > x) {
                return buff.size();
            } else {
                try {
                    utf8::next(cur, text.end());
                    idx++;
                } catch (...) {
                    throw GCN_EXCEPTION("Invalid UTF-8 text!");
                }
            }
        }
        
        if (x > getWidth(text)) {
            return text.size();
        } else {    
            return buff.size();
        }
    }

}

