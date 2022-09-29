#include "console.hpp"
#include "graphics.hpp"
#include "font.hpp"

void PutChar(PixelWriter *pixel_writer, char c) {
  WriteAscii(pixel_writer, 0, 0, c);
};
