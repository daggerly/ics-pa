#include <check.h>

#define N 32
#define DEFV 48
char dst[N];


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
  }
}

int main() {
  int l;
  for (l = 0; l < N-1; l ++) {
    reset();
    dst[l] = '\0';
    assert(strlen(dst) == l);
  }
  return 0;
}
