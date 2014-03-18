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

}// end of namespace
