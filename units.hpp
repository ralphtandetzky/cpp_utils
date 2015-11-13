/** @file Define constants for some physical units.
 *
 * Use these constants throughout your code whenever you encounter physical
 * quantities. When doing conversions the word "in" can be translated into
 * arithmetic as "divided by". For example
 *   @code
 *     auto lengthInInch = length / cu::inch;
 *   @endcode
 *
 * Currently, all basic SI units have the value 1. However, a program should
 * not rely on this and produce the same results independent of the concrete
 * values.
 *
 * @author Ralph Tandetzky
 */

#pragma once

namespace cu
{

constexpr double pi          = 3.14159265358979323846;
constexpr double goldenRatio = 1.61803398874989484820;

constexpr double      second = 1.;
constexpr double millisecond = 1.e-3 * second;
constexpr double microsecond = 1.e-6 * second;
constexpr double  nanosecond = 1.e-9 * second;

constexpr double minute = 60 * second;
constexpr double hour   = 60 * minute;
constexpr double day    = 24 * hour;

constexpr double      meter = 1.;
constexpr double  kilometer = 1.e+3 * meter;
constexpr double  decimeter = 1.e-1 * meter;
constexpr double centimeter = 1.e-2 * meter;
constexpr double millimeter = 1.e-3 * meter;
constexpr double micrometer = 1.e-6 * meter;
constexpr double  nanometer = 1.e-9 * meter;
constexpr double       inch = 2.54 * centimeter;
constexpr double       foot = 12 * inch;
constexpr double       yard = 3 * foot;

constexpr double     Hertz = 1/second;
constexpr double kiloHertz = 1.e3*Hertz;
constexpr double megaHertz = 1.e6*Hertz;
constexpr double gigaHertz = 1.e9*Hertz;

constexpr double      Volt = 1.;
constexpr double  megaVolt = 1.e+6 * Volt;
constexpr double  kiloVolt = 1.e+3 * Volt;
constexpr double milliVolt = 1.e-3 * Volt;
constexpr double microVolt = 1.e-6 * Volt;

} // namespace cu
