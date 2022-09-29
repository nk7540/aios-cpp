#pragma once

typedef enum {
  PixelRedGreenBlue,
  PixelBlueGreenRed
} PixelFormat;

typedef struct {
  unsigned int*             buffer_base;
  unsigned long long        buffer_size;
  unsigned int              horizontal_resolution;
  unsigned int              vertical_resolution;
  PixelFormat               pixel_format;
  unsigned int              pixels_per_scan_line;
} FrameBuffer;
