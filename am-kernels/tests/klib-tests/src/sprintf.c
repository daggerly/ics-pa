#include <check.h>
#include <limits.h>

#define N 16
#define DEFV 48
char dst[N];
int data[] = {
  0, 126322567, 2147483647, -2147483648, -2147483647,
  252645135, 126322567, -1, 0, 0, 
  1, 1, -1, -1};
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
  "  1", "001"," -1", "-01",
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
  0, 4294967295, 1
};

int u_offsets[] = {
  1, 0, 0
};


char* u_fmts[] = {
  "bb%u", "%ubb", "%09u"
};

char* u_ans[] = {
  "abb0", "4294967295bb", "000000001",
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

  for (int i = 0; i < sizeof(udata)/sizeof(uint32_t); i++) {
    reset();
    sprintf(dst + u_offsets[i], u_fmts[i], udata[i]);
    //需要打开DEVICE
    // printf("%s %s\n", dst, u_ans[i]);
    assert(strcmp(dst, u_ans[i]) == 0);
  }

  return 0;
}
