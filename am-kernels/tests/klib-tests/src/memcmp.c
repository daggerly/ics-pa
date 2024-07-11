#include <check.h>

#define N 32
#define DEFV 99
uint8_t data[N], dst[N];


void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    dst[i] = i + 1;
    data[i] = N - i;
  }
}

int main() {
  reset();
  assert(memcmp(dst, data, 1) < 0);
  assert(memcmp(dst, data+N-1, 1) == 0);
  assert(memcmp(dst+N-1, data+N-1, 1) > 0);

  return 0;
}