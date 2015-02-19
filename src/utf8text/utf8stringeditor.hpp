#ifndef GCN_UTF8STRINGEDITOR_HPP
#define GCN_UTF8STRINGEDITOR_HPP

#include <guichan.hpp>

namespace gcn
{

    /**
     * UTF-8 string editor.
     *
     * This is a helper class which allows to use UTF-8 strings in
     * your application.
     *
     * @author Przemyslaw Grzywacz
     */
    class GCN_EXTENSION_DECLSPEC UTF8StringEditor
    {
    public:
        /**
         * Returns byte offset of the next character.
         *
         * @param text UTF-8 text to navigate.
         * @param offset Byte offset of current character.
         * @return Byte offset of the next character.
         */
        int nextCharacter(const std::string& text, int offset);

        /**
         * Returns byte offset of the previous character.
         *
         * @param text UTF-8 text to navigate.
         * @param offset Byte offset of current character.
         * @return Byte offset of the previous character.
         */
        int prevCharacter(const std::string& text, int offset);

        /**
         * Erase character at specified byte offset.
         *
         * @param text UTF-8 text to modify.
         * @param offset Byte offset of the character to erase.
         * @return New byte offset (is equal to byteOffset).
         */
        int eraseCharacter(std::string& text, int offset);

        /**
         * Insert a character at specified byte offset.
         *
         * @param text UTF-8 text to modify.
         * @param offset Byte offset where character will be inserted.
         * @param ch Unicode character to insert.
         * @return New byte offset (after the new character).
         */
        int insertCharacter(std::string& text, int offset, int ch);

        /**
         * Counts characters up to byteOffset.
         *
         * @param text UTF-8 text to navigate.
         * @param offset Byte offset inside the text.
         * @return Number of characters.
         */
        int countCharacters(const std::string& text, int offset);

        /**
         * Gets byte offset for character index.
         *
         * This method automaticly clips charIndex to be inside
         * the string + EOF
         *
         * @param text UTF-8 text to navigate.
         * @param index Character index to move to.
         * @return Byte offset of character at charIndex.
         */
        int getOffset(const std::string& text, int index);
    };

};

#endif // !GCN_UTF8STRINGEDITOR_HPP
