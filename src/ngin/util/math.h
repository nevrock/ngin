#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <cstdlib>
#include <fstream> // Include this header for file operations

class MathUtil
{
public:
    // Corrected method signature and fixed parameter names
    static float get_max(float i1, float i2)
    {
        return (i1 > i2) ? i1 : i2; // Use the correct variable names
    }
    static float get_min(float i1, float i2)
    {
        return (i1 < i2) ? i1 : i2; // Use the correct variable names
    }
    static float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

private:
    // Typically, static utility classes do not need constructors or destructors
    MathUtil() = delete; // Delete default constructor to prevent instantiation
};

#endif // MATH_UTIL_H
