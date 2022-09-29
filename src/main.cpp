#include "frame_buffer.hpp"
#include "graphics.hpp"
#include "console.hpp"

void* operator new(unsigned long size, void* buf) {
  return buf;
}

void operator delete(void* obj) noexcept {
}

extern "C" void main(FrameBuffer& frame_buffer) {
  for (int i = 0; i < frame_buffer.vertical_resolution; i++) {
    for (int j = 0; j < frame_buffer.horizontal_resolution; j++) {
      frame_buffer.buffer_base[i * frame_buffer.pixels_per_scan_line + j] = 0;
    }
  }
  char pixel_writer_buf[sizeof(PixelWriter)];
  PixelWriter *pixel_writer = new(pixel_writer_buf) PixelWriter{frame_buffer};
  PutChar(pixel_writer, *"A");
  // PutString("Hello, AIOS!");
  while(1) __asm__("hlt");
}
