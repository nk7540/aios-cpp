#include "frame_buffer.hpp"

extern "C" void main(FrameBuffer *frame_buffer) {
  for (int i = 0; i < frame_buffer->vertical_resolution; i++) {
    for (int j = 0; j < frame_buffer->horizontal_resolution; j++) {
      frame_buffer->buffer_base[i * frame_buffer->pixels_per_scan_line + j] = 0;
    }
  }
  while(1) __asm__("hlt");
}
