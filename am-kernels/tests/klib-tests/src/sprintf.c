#include <check.h>
#include <limits.h>

#define N 16
#define DEFV 48
char dst[N];
int data[] = {
  0, 126322567, 2147483647, -2147483648, -2147483647,
  252645135, 126322567, -1, 0, 0, 
  1, 10, -1, -10};
int d_offsets[] = {
  1, 0, 1, 1, 0, 
  0, 0, 13, 0, 0,
  0, 0, 0, 0
};

char* d_fmts[] = {
  "bb%d", "%dbb", "%d", "-%d", "%d",
  "%d", "%4d", "%d", "%2d", "%02d",
  "%3d", "%03d", "%3d", "%03d",
};

char* d_ans[] = {
  "abb0", "126322567bb", "a2147483647", "a--2147483648", "-2147483647",
  "252645135", "126322567", "aaaaaaaaaaaaa-1", " 0", "00",
  "  1", "010"," -1", "-10",
};

char* s_data[] = {"", "%s", "b",  "b", };
int s_offsets[] = {
  0, 1, 0, 0
};

char* s_fmts[] = {
  "%s",
  "%s",
  "b%s",
  "%sb"
};

char* s_ans[] = {
  "",
  "a%s",
  "bb",
  "bb"
};


uint32_t udata[] = {
  0, 0xffffffff, 1, 0xffffffff
};

int u_offsets[] = {
  1, 0, 0, 0
};


char* u_fmts[] = {
  "bb%u", "%ubb", "%09u", "%02u"
};

char* u_ans[] = {
  "abb0", "4294967295bb", "000000001", "4294967295"
};


uint32_t xdata[] = {
  0, 0x8000, 1, 0xffffffff
};

int x_offsets[] = {
  1, 0, 0, 0
};


char* x_fmts[] = {
  "bb%02x", "%xbb", "%9x", "%2x"
};

char* x_ans[] = {
  "abb00", "8000bb", "        1", "ffffffff"
};

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    dst[i] = 'a';
  }
}

int main() {
  // test %d
  for (int i = 0; i < sizeof(data)/sizeof(int); i++) {
    reset();
    sprintf(dst + d_offsets[i], d_fmts[i], data[i]);
    //需要打开DEVICE
    // printf("%s %s\n", dst, d_ans[i]);
    assert(strcmp(dst, d_ans[i]) == 0);
  }

  // test %s
  for (int i = 0; i < sizeof(s_data)/sizeof(char *); i++) {
    reset();
    sprintf(dst + s_offsets[i], s_fmts[i], s_data[i]);
    // printf("%s %s\n", dst, d_ans[i]);
    assert(strcmp(dst, s_ans[i]) == 0);
  }

  // test %u
  for (int i = 0; i < sizeof(udata)/sizeof(uint32_t); i++) {
    reset();
    sprintf(dst + u_offsets[i], u_fmts[i], udata[i]);
    //需要打开DEVICE
    // printf("%s %s\n", dst, u_ans[i]);
    assert(strcmp(dst, u_ans[i]) == 0);
  }

  // test %x
  for (int i = 0; i < sizeof(xdata)/sizeof(uint32_t); i++) {
    reset();
    sprintf(dst + x_offsets[i], x_fmts[i], xdata[i]);
    //需要打开DEVICE
    // printf("%s %s\n", dst, x_ans[i]);
    assert(strcmp(dst, x_ans[i]) == 0);
  }


  return 0;
}
