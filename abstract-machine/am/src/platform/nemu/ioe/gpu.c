#include <am.h>
#include <nemu.h>
#include <klib.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)


static int W = 0, H = 0;
// static uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}
static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }


void __am_gpu_init() {
  int i;
  uint32_t wh = inl(VGACTL_ADDR);
  W = wh >> 16;
  H = wh & 0xffff;
  for (i = 0; i < W * H; i ++) outl(FB_ADDR+i, 0);
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = W, .height = H,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;
  int len = (x + w >= W) ? W - x : w;
  for (int j = 0; j < h; j ++) {
    if (y + j < H) {
      uint32_t px = x + (j + y) * W;
      for (int i = 0; i < len; i ++) {
        outl(FB_ADDR + px*4, pixels[i+j*w]);
        px += 1;
      }
    }
  }


  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
