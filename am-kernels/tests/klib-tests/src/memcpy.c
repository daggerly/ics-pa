#include <check.h>

#define N 32
#define DEFV 99
uint8_t data[N], dst[N];


// 检查[l,r)区间中的值是否依次为val, val + 1, val + 2...
void check_seq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    assert(dst[i] == val + i - l);
  }
}

// 检查[l,r)区间中的值是否均为val
void check_eq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    assert(dst[i] == val);
  }
}

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    dst[i] = i + 1;
    data[i] = DEFV;
  }
}

int main() {
  int l, r;
  for (l = 0; l < N; l ++) {
    for (r = l + 1; r <= N; r ++) {
      reset();
      memcpy(dst + l, data + l, r - l);
      check_seq(0, l, 1);
      check_eq(l, r, DEFV);
      check_seq(r, N, r+1);
    }
  }
  return 0;
}