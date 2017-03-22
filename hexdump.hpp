// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include <locale>
#include <ostream>
#include <string>

namespace cu
{

/// Prints a variable or array in hex dump format into a std::basic_ostream output stream
///
/// @example Here's how it's used:
/// @code
///   std::string r;
///
///   for (int i = 0; i != 100; ++i)
///     r += rand() % 256;
///
///    hexDump(r.c_str(), r.size(), std::cout);
/// @endcode
///
/// Output:
///
/// 0000 : )#...l..R.I..... 29 23 BE 84 E1 6C D6 AE 52 90 49 F1 F1 BB E9 EB
/// 0010 : ...<..>.$^....G. B3 A6 DB 3C 87 0C 3E 99 24 5E 0D 1C 06 B7 47 DE
/// 0020 : ..M.C.....Z}.8%. B3 12 4D C8 43 BB 8B A6 1F 03 5A 7D 09 38 25 1F
/// 0030 : ].....E;.......2 5D D4 CB FC 96 F5 45 3B 13 0D 89 0A 1C DB AE 32
/// 0040 :  .P.@x6..I2..}I. 20 9A 50 EE 40 78 36 FD 12 49 32 F6 9E 7D 49 DC
/// 0050 : .O..D@f.k.0.2;." AD 4F 14 F2 44 40 66 D0 6B C4 30 B7 32 3B A1 22
/// 0060 : ."..             F6 22 91 9D
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

} // namespace cu
