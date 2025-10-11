#ifndef __TOOL_FUNCTIONS_HPP__
#define __TOOL_FUNCTIONS_HPP__

#include "Config.hpp"

extern "C"
{
#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"
}

// 行进到上一行起始位置
size_t f_seek_pre_line_begin(FILE *file);

// 读取文件一行
size_t f_getline(FILE *file, char *buffer, const size_t buffer_len);

// 进度条打印
void progress_print(size_t completed, size_t total, bool reflush);

// 16进制字符串转有效无符号整数（8字节）
uint64_t hex_to_value(const char *str);

// 判定字符串是否全部为有效数字
bool is_full_num(const char *str);

// 十进制字符串转有效无符号整数（8字节）
uint64_t str_to_value(const char *str);

#endif