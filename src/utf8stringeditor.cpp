#include <iostream>
#include "utf8stringeditor.hpp"
#include "utf8.h"

namespace gcn
{

    int UTF8StringEditor::nextCharacter(const std::string & text, int offset)
    {
        std::string::const_iterator c, e;

        c = text.begin() + offset;
        e = text.end();

        utf8::next(c, e);
        return std::string(text.begin(), c).size();
    }

    int UTF8StringEditor::prevCharacter(const std::string & text, int offset)
    {
        std::string::const_iterator c, b;

        c = text.begin() + offset;
        b = text.begin();

        utf8::prior(c, b);
        return std::string(b, c).size();
    }

    int UTF8StringEditor::eraseCharacter(std::string & text, int offset)
    {
        std::string::iterator begin, cur;
        begin = text.begin() + offset;
        cur = begin;
        utf8::next(cur, text.end());

        text = std::string(text.begin(), begin) + std::string(cur, text.end());
        return offset; // this shouldn't change!
    }

    int UTF8StringEditor::insertCharacter(std::string & text, int offset, int ch)
    {
        std::string newString;
        utf8::append(ch, std::back_inserter(newString));
        text.insert(offset, newString);
        return offset + newString.size();
    }

    int UTF8StringEditor::countCharacters(const std::string & text, int offset)
    {
        return utf8::distance(text.begin(), text.begin() + offset);
    }

    int UTF8StringEditor::getOffset(const std::string & text, int index)
    {
        std::string::const_iterator cur, end;
        int bytes = 0, i;

        if (index < 0)
        {
            return 0;
        }

        cur = text.begin();
        end = text.end();

        for(i = 0; i <= index; i++)
        {
            try
            {
                utf8::next(cur, end);
            }
            catch (...)
            {
                return text.size();
            }
        }

        return std::string(text.begin(), cur).size() - 1;
    }
};






