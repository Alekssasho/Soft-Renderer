#include "color.h"

namespace SoftEngine
{

const Color Color::Red(255, 0, 0, 255);
const Color Color::Green(0, 255, 0, 255);
const Color Color::Blue(0, 0, 255, 255);
const Color Color::White(255, 255, 255, 255);
const Color Color::Black(0, 0, 0, 255);

std::ostream& operator<<(std::ostream& out, const Color& color)
{
    out << "Color:\nred: " << color.r() << "\ngreen: " << color.g() << "\nblue: " << color.b() << "\nalpha: " << color.a() << std::endl;
    return out;
}

Color operator*(Color& color, float scalar)
{
    float red = static_cast<float>(color.r()) / 255.0f;
    float green = static_cast<float>(color.g()) / 255.0f;
    float blue = static_cast<float>(color.b()) / 255.0f;

    red *= scalar;
    green *= scalar;
    blue *= scalar;

    int ired = static_cast<int>(red * 255.0f);
    int igreen = static_cast<int>(green * 255.0f);
    int iblue = static_cast<int>(blue * 255.0f);

    return Color(ired, igreen, iblue, color.a());
}

}// end of namespace
