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
    char value_name_str[VARIABLE_NAME_STR_LENGTH_MAX] = {'\0'};
    const char *value_type_str = DEFAULT_VALUE_TYPE;
    uint64_t raw_value = 0x0000000000000000;
    value_node_struct *p_next;
} value_node;

// 格式定义列表
typedef struct format_list_node_struct
{
    char target_value_name_str[VARIABLE_NAME_STR_LENGTH_MAX] = {'\0'}; // 目标变量名
    int target_numID = -1;                                             // 目标数字ID
    const char *value_type_str = nullptr;                              // 目标类型
    format_list_node_struct *p_next;
} format_list_node;

// 数据类型
#endif
