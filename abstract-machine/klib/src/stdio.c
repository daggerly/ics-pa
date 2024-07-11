#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list ap;
  size_t i = 0, 
  /* 是否在格式化字符串中*/
  in_fmt=0, 
  /* 要拷贝几个字符*/
  n_to_copy=0, 
  /* 要拷贝的字符串起始位置*/
  copy_start=0, 
  /* 总共拷贝了几个字符*/
  copyed = 0, fmt_len = strlen(fmt);

  const char * arg_s = NULL;
  /* 存储数字%d的数值*/
  int arg_d = 0,
  /*数字位数*/
  arg_d_n=1, 
  /*数字某一位的数字*/
  arg_d_digit=0;

  va_start(ap, fmt);
  while (i < fmt_len){
    if (in_fmt){  /* 在格式化字符串中*/
      if (fmt[i] == 'd'){
        arg_d = va_arg(ap, int);
        arg_d_n=1;
          if (arg_d < 0){
            putch('-');
            copyed += 1;
            // arg_d = -arg_d;
            arg_d_n = -1;
          }
          arg_d_digit = arg_d;
          if (arg_d > 0){
            while (arg_d_digit > 0){
              arg_d_n *= 10;
              arg_d_digit /= 10;
            }
          }else{
            while (arg_d_digit < 0){
              arg_d_n *= 10;
              arg_d_digit /= 10;
            }
          }
          
          arg_d_n /= 10;
          if (arg_d != 0){
            while (arg_d_n != 0){
              arg_d_digit = arg_d / arg_d_n;
              putch(arg_d_digit + 48);
              copyed += 1;
              arg_d = arg_d % arg_d_n;
              arg_d_n /= 10;
            }
          }else{
            // 0
            putch(arg_d + 48);
            copyed += 1;
          }
      }else if (fmt[i] == 's'){
        arg_s = va_arg(ap, char*);
        for(const char* arg_s_p=arg_s; *arg_s_p; arg_s_p++){
          putch(*arg_s_p);
          copyed += 1;
        }
        arg_s = NULL;
      }else{
        panic("Not implemented");
      }
      /* 处理完格式化字符了*/
      in_fmt = 0;
      copy_start = i + 1; /* 要拷贝的字符串起始位置指向下一个字符*/
    }else if (fmt[i] == '%'){ /* 遇到格式化字符了*/
      in_fmt = 1;
      /* 把之前没拷贝的字符拷贝过去*/
      if (n_to_copy > 0){
        for(size_t n_copyed=0; n_copyed < n_to_copy; n_copyed++){
          putch(*(fmt+copy_start+n_copyed));
        }
        copyed += n_to_copy;
        n_to_copy = 0;
        copy_start = i;
      }
    }else{  /* 普通字符串 */
      n_to_copy ++;
    }
    i ++;
  }
  va_end(ap);
  if (n_to_copy > 0){
    for(size_t n_copyed=0; n_copyed < n_to_copy; n_copyed++){
      putch(*(fmt+copy_start+n_copyed));
    }
    copyed += n_to_copy;
    n_to_copy = 0;
  }
  // strncpy(out+copyed, "\0", 1);
  putch('\0');
  return copyed;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    size_t i = 0, 
    /* 是否在格式化字符串中*/
    in_fmt=0, 
    /* 要拷贝几个字符*/
    n_to_copy=0, 
    /* 要拷贝的字符串起始位置*/
    copy_start=0, 
    /* 总共拷贝了几个字符*/
    copyed = 0, fmt_len = strlen(fmt);

    const char * arg_s = NULL;
    /* 存储数字%d的数值*/
    int arg_d = 0,
    /*数字位数*/
    arg_d_n=1, 
    /*数字某一位的数字*/
    arg_d_digit=0;

    va_start(ap, fmt);
    while (i < fmt_len){
      if (in_fmt){  /* 在格式化字符串中*/
        if (fmt[i] == 'd'){
          arg_d = va_arg(ap, int);
          arg_d_n=1;
          if (arg_d < 0){
            memset(out+copyed, '-', 1);
            copyed += 1;
            // arg_d = -arg_d;
            arg_d_n = -1;
          }
          arg_d_digit = arg_d;
          if (arg_d > 0){
            while (arg_d_digit > 0){
              arg_d_n *= 10;
              arg_d_digit /= 10;
            }
          }else{
            while (arg_d_digit < 0){
              arg_d_n *= 10;
              arg_d_digit /= 10;
            }
          }
          
          arg_d_n /= 10;
          if (arg_d != 0){
            while (arg_d_n != 0){
              arg_d_digit = arg_d / arg_d_n;
              memset(out+copyed, arg_d_digit + 48, 1);
              copyed += 1;
              arg_d = arg_d % arg_d_n;
              arg_d_n /= 10;
            }
          }else{
            // 0
            memset(out+copyed, arg_d + 48, 1);
            copyed += 1;
          }
        }else if (fmt[i] == 's'){
          arg_s = va_arg(ap, char*);
          strcpy(out + copyed, arg_s);
          copyed += strlen(arg_s);
          arg_s = NULL;
        }else{
          panic("Not implemented");
        }
        /* 处理完格式化字符了*/
        in_fmt = 0;
        copy_start = i + 1; /* 要拷贝的字符串起始位置指向下一个字符*/
      }else if (fmt[i] == '%'){ /* 遇到格式化字符了*/
        in_fmt = 1;
        /* 把之前没拷贝的字符拷贝过去*/
        if (n_to_copy > 0){
          strncpy(out+copyed, fmt+copy_start, n_to_copy);
          copyed += n_to_copy;
          n_to_copy = 0;
          copy_start = i;
        }
      }else{  /* 普通字符串 */
        n_to_copy ++;
      }
      i ++;
    }
    va_end(ap);
    if (n_to_copy > 0){
      strncpy(out+copyed, fmt+copy_start, n_to_copy);
      copyed += n_to_copy;
      n_to_copy = 0;
    }
    // strncpy(out+copyed, "\0", 1);
    memset(out+copyed, 0, 1);
    return copyed;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
