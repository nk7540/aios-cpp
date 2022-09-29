#include "graphics.hpp"

extern const char _binary_hankaku_bin_start;
extern const char _binary_hankaku_bin_end;
extern const char _binary_hankaku_bin_size;


// These functions are only used in this file
namespace {
  /*
   * Look up font and return the address
   * Each font are 16 entries of 1 byte (16x8), and is ordered by ascii code
   * If font offset exceeds the end of font files, return nullptr (unsupported)
  */
  const char* GetFont(char c) {
    unsigned int offset = 16 * static_cast<unsigned int>(c);
    if (offset >= reinterpret_cast<unsigned long long>(&_binary_hankaku_bin_size)) {
      return nullptr;
    }
    return (char*)((unsigned long long)&_binary_hankaku_bin_start + offset);
  }
}

void WriteAscii(PixelWriter *pixel_writer, int x, int y, char c) {
  const char *font = GetFont(c);
  for (int dy = y; dy < y + 16; dy++) {
    for (int dx = x; dx < x + 8; dx++) {
      if (font[dy] << dx & 0x80) {
        pixel_writer->WritePixel(dx, dy, PixelColor{(char)255, (char)255, (char)255});
      }
    }
  }
}
