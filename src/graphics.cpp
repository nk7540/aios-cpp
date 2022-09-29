#include "graphics.hpp"

void PixelWriter::WritePixel(int x, int y, PixelColor color) {
  unsigned int pixel_val = color.r << 24 | color.g << 16 | color.b << 8;
  frame_buffer_.buffer_base[y * frame_buffer_.pixels_per_scan_line + x] =  pixel_val;
}
