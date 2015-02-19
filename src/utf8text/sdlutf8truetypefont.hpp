#ifndef GCN_SDLUTF8TRUETYPEFONT_HPP
#define GCN_SDLUTF8TRUETYPEFONT_HPP

#include <map>
#include <string>

#include <SDL/SDL_ttf.h>

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/image.hpp"
#include "guichan/platform.hpp"
#include "sdltruetypefont.hpp"

namespace gcn
{
    /**
     * SDL UTF-8 True Type Font implementation of Font. It uses the SDL_ttf
     * library to display True Type Fonts with SDL. This extension to
     * SDLTrueTypeFont accepts UTF-8 strings!
     *
     * NOTE: You must initialize the SDL_ttf library before using this
     *       class. Also, remember to call the SDL_ttf libraries quit
     *       function.
     *
     * @author Przemyslaw Grzywacz
     */
    class GCN_EXTENSION_DECLSPEC SDLUTF8TrueTypeFont: public SDLTrueTypeFont
    {
    public:
        /**
        * Constructor.
        *
        * @param filename the filename of the True Type Font.
        * @param size the size the font should be in.
        */
        SDLUTF8TrueTypeFont (const std::string& filename, int size);

        /**
         * See gcn::Font::getWidth
         */
        virtual int getWidth(const std::string& text) const;

        /**
         * See gcn::Font::drawString
         */
        virtual void drawString(Graphics* graphics, const std::string& text, int x, int y);

        /**
         * See gcn::Font::getStringIndexAt
         */
        virtual int getStringIndexAt(const std::string& text, int x) const;
    };
}

#endif // end GCN_SDLUTF8TRUETYPEFONT_HPP

