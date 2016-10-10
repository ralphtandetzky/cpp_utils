#pragma once

#include <locale>
#include <ostream>
#include <string>

template<class elem, class traits>
inline void hexDump( const void* data,
                     std::size_t length,
                     std::basic_ostream<elem, traits>& stream,
                     std::size_t width = 16 )
{
    const char* const start = static_cast<const char*>(data);
    const char* const end = start + length;
    const char* line = start;
    const auto locale = stream.getloc();
    while (line != end)
    {
        stream.width(4);
        stream.fill('0');
        stream << std::hex << std::uppercase;
        stream << line - start << " : ";
        std::size_t lineLength = std::min(width, static_cast<std::size_t>(end - line));
        for (const char* next = line; next != line + lineLength; ++next)
        {
            char ch = *next;
            stream << ( !std::isprint( ch, locale ) ? '.' : ch );
        }
        stream << std::string(width - lineLength, ' ');
        for (const char* next = line; next != line + lineLength; ++next)
        {
            stream << " ";
            stream.width(2);
            stream.fill('0');
            stream << std::hex
                   << std::uppercase
                   << static_cast<int>(static_cast<unsigned char>(*next));
        }
        stream << std::endl;
        line += lineLength;
    }
}

