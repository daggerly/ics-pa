#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define  DEFAULT_PADDING ' '
#define  ZERO_PADDING '0'

typedef struct 
{
  void * addr;
  void (*handler)(void *, int, int);
} OutputClosure;

static int _printf(va_list ap, const char *fmt, OutputClosure *output){
    /* 是否在格式化字符串中*/
    uint8_t in_fmt = 0,
    /* 是不是padding字符 */
    is_padding = 1;
    size_t i = 0, 
    /* 要拷贝几个字符*/
    n_to_copy=0, 
    /* 要拷贝的字符串起始位置*/
    copy_start=0, 
    /* 总共拷贝了几个字符*/
    copyed = 0, 
    fmt_len = strlen(fmt);
    char padding = DEFAULT_PADDING;
    const char * arg_s = NULL;
    /* 存储数字%d的数值*/
    int arg_d = 0,
    /*模的基数*/
    mod = 1, 
    /*数字某一位的数字*/
    arg_d_digit = 0,
    /* 宽度 */
    width = 0;
    uint32_t arg_u = 0,
    arg_u_digit = 0,
    mod_u = 1;

    while (i < fmt_len){
      if (in_fmt){  /* 在格式化字符串中*/
        if (fmt[i] == 'd'){
          arg_d = va_arg(ap, int);
          mod=1;
          int diff_width = width;
          if (arg_d < 0){
            mod = -1;
            // 负数的负号本身占一位
            diff_width -= 1;
          }
          arg_d_digit = arg_d;
          if (arg_d > 0){
            while (arg_d_digit > 0){
              mod *= 10;
              arg_d_digit /= 10;
              diff_width -= 1;
            }
          }else if (arg_d == 0){
            diff_width -= 1;
          }else{
            while (arg_d_digit < 0){
              mod *= 10;
              arg_d_digit /= 10;
              diff_width -= 1;
            }
          }
          mod /= 10;
        
          /* 负数且空格填充时，空格在负号前 */
          if (arg_d < 0 && diff_width > 0 && padding == DEFAULT_PADDING){
            while (diff_width > 0)
            {
              output->handler(output->addr, copyed, padding);
              diff_width -= 1;
              copyed += 1;
            }
          }
          if (arg_d < 0){
            output->handler(output->addr, copyed, '-');
            copyed += 1;
          }
          while (diff_width > 0){
            output->handler(output->addr, copyed, padding);
            diff_width -= 1;
            copyed += 1;
          }
  
          if (arg_d != 0){
            while (mod != 0){
              arg_d_digit = arg_d / mod;
              output->handler(output->addr, copyed, arg_d_digit + 48);
              copyed += 1;
              arg_d = arg_d % mod;
              mod /= 10;
            }
          }else{
            // 0
            output->handler(output->addr, copyed, arg_d + 48);
            copyed += 1;
          }
        }else if (fmt[i] == 's'){
          arg_s = va_arg(ap, char*);
          for(const char* arg_s_p=arg_s; *arg_s_p; arg_s_p++){
            output->handler(output->addr, copyed, *arg_s_p);
            copyed += 1;
          }
          arg_s = NULL;
        }else if (fmt[i] == '0'){
          if (is_padding){
            padding = ZERO_PADDING;
            // 遇到padding了，再遇到0就是宽度数字了
            is_padding = 0;
          }else{
            // 不是padding，那就是宽度数字
            width *= 10;
            width += fmt[i] - 48;
          }
          i += 1;
          continue;
        }else if (fmt[i] > '0' && fmt[i] <= '9'){
          /* 宽度数字 */
          is_padding = 0;

          width *= 10;
          width += fmt[i] - 48;

          i += 1;
          continue;
        }else if (fmt[i] == 'u'){
          arg_u = va_arg(ap, uint32_t);
          mod_u=1;
          int diff_width = width;
          
          arg_u_digit = arg_u;
          if (arg_u > 0){
            arg_u_digit /= 10;
            diff_width -= 1;
            while (arg_u_digit > 0){
              mod_u *= 10;
              arg_u_digit /= 10;
              diff_width -= 1;
            }
          }else if (arg_u == 0){
            diff_width -= 1;
          }
          while (diff_width > 0){
            output->handler(output->addr, copyed, padding);
            diff_width -= 1;
            copyed += 1;
          }
  
          if (arg_u != 0){
            while (mod_u != 0){
              arg_u_digit = arg_u / mod_u;
              output->handler(output->addr, copyed, arg_u_digit + 48);
              copyed += 1;
              arg_u = arg_u % mod_u;
              mod_u /= 10;
            }
          }else{
            // 0
            output->handler(output->addr, copyed, '0');
            copyed += 1;
          }
        }else if (fmt[i] == 'x'){
          arg_u = va_arg(ap, uint32_t);
          mod_u=1;
          int diff_width = width, weishu = 1;
          
          arg_u_digit = arg_u;
          if (arg_u > 0){
            while (arg_u_digit > 0){
              weishu += 1;
              arg_u_digit = arg_u_digit >> 4;
              diff_width -= 1;
            }
            weishu -= 1;
          }else{
            diff_width -= 1;
          }
         
          for(;weishu > 1; weishu -= 1){
            mod_u = mod_u << 4;
          }

          for(; diff_width > 0; diff_width -= 1){
            output->handler(output->addr, copyed, padding);
            copyed += 1;
          }
  
          if (arg_u > 0){
            while (mod_u > 0){
              arg_u_digit = arg_u / mod_u;
              if(arg_u_digit < 10){
                // 0-9
                output->handler(output->addr, copyed, arg_u_digit + '0');
              }else{
                // a-f
                output->handler(output->addr, copyed, arg_u_digit -10 + 'a');
              }
              copyed += 1;
              arg_u = arg_u % mod_u;
              mod_u = mod_u >> 4;
            }
          }else{
            // 0
            output->handler(output->addr, copyed, '0');
            copyed += 1;
          }
        }else{
          panic("Not implemented");
        }
        /* 处理完格式化字符了*/
        in_fmt = 0;
        copy_start = i + 1; /* 要拷贝的字符串起始位置指向下一个字符*/
        padding = DEFAULT_PADDING;
        width = 0;
        is_padding = 1;
      }else if (fmt[i] == '%'){ /* 遇到格式化字符了*/
        in_fmt = 1;
        /* 把之前没拷贝的字符拷贝过去*/
        if (n_to_copy > 0){
          for(size_t n_copyed=0; n_copyed < n_to_copy; n_copyed++){
            output->handler(output->addr, copyed, *(fmt+copy_start+n_copyed));
            copyed += 1;
          }
          n_to_copy = 0;
          copy_start = i;
        }
      }else{  /* 普通字符串 */
        n_to_copy ++;
      }
      i ++;
    }
    
    if (n_to_copy > 0){
      for(size_t n_copyed=0; n_copyed < n_to_copy; n_copyed++){
        output->handler(output->addr, copyed, *(fmt+copy_start+n_copyed));
        copyed += 1;
      }
      n_to_copy = 0;
    }
    output->handler(output->addr, copyed, 0);
    return copyed;
}

static void printf_handler(void *addr, int offset, int c){
  putch(c);
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  OutputClosure output = {
      NULL, 
      .handler = printf_handler
  };
  int copyed = _printf(ap, fmt, &output);
  va_end(ap);
  return copyed;
}

static void sprintf_handler(void *addr, int offset, int c){
  memset(addr + offset, c, 1);
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    OutputClosure output = {
      out, 
      .handler = sprintf_handler
    };
    return _printf(ap, fmt, &output);
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int copyed = vsprintf(out, fmt, ap);
    va_end(ap);
    return copyed;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
