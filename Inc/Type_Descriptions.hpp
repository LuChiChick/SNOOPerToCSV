#ifndef __TYPE_DESCRIPTIONS_HPP__
#define __TYPE_DESCRIPTIONS_HPP__

#include "Config.hpp"

extern "C"
{
#include "stdint.h"
}

// 文件链表节点
typedef struct file_node_struct
{
    const char *file_name_str = nullptr;
    file_node_struct *p_next = nullptr;
} file_node;

// 数据列表
typedef struct value_node_struct
{
    char value_name_str[100] = {'\0'};
    const char *value_type_str = "uint64_t";
    uint64_t raw_value = 0x0000000000000000;
    value_node_struct *p_next;
} value_node;

// 数据类型
#endif
