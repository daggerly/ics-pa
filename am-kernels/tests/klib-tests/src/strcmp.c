#include <check.h>

#define N 32
#define DEFV 99
char data[N], dst[N];


void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    dst[i] = i + 1;
    data[i] = N - i;
  }
}

int main() {
  reset();
  dst[N-1] = '\0';
  data[N-1] = '\0';
  assert(strcmp(dst, data) < 0);
  assert(strcmp(dst+1, data+N-2) > 0);
  assert(strcmp(dst+N-1, data+N-1) == 0);
  assert(strcmp(dst+N-2, data+N-2) > 0);

  return 0;
}