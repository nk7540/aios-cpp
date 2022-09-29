#pragma once

#include "frame_buffer.hpp"

typedef struct {
  char r,g,b;
} PixelColor;


class PixelWriter {
  public:
    PixelWriter(const FrameBuffer& frame_buffer) : frame_buffer_{frame_buffer} {
    }
    void WritePixel(int x, int y, PixelColor color);

  private:
    const FrameBuffer& frame_buffer_;
};
