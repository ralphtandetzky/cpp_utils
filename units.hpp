#pragma once

namespace cu
{

constexpr double pi          = 3.14159265358979323846;
constexpr double goldenRatio = 1.61803398874989484820;

constexpr double      seconds = 1.;
constexpr double milliseconds = 1.e-3;
constexpr double microseconds = 1.e-6;
constexpr double  nanoseconds = 1.e-9;

constexpr double     Hertz = 1/seconds;
constexpr double kiloHertz = 1.e3*Hertz;
constexpr double megaHertz = 1.e6*Hertz;
constexpr double gigaHertz = 1.e9*Hertz;

constexpr double  megaVolts = 1.e+6;
constexpr double  kiloVolts = 1.e+3;
constexpr double      Volts = 1.;
constexpr double milliVolts = 1.e-3;
constexpr double microVolts = 1.e-6;

} // namespace cu
