#include "Tool_Functions.hpp"
#include "Global_Variables.hpp"

extern "C"
{
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "ctype.h"
}

int main(int argc, char *argv[])
{
    printf("\n\n");
    printf("SNOOPerToCSV Ver2.0\n\n");
    printf("Auther: LuChiChick\n");
    printf("%s\n%s\n%s\n\n", "Open source links:",
           "         ├─Github:              https://github.com/LuChiChick/SNOOPerToCSV",
           "         └─Personal Git System: https://git.luchichick.cn/LuChiChick/SNOOPerToCSV");

    // 处理指令输入
    for (int count = 1; count < argc; count++)
    {
        // 处理时间戳
        if (!strcmp(argv[count], "-t"))
        {
            // 参数不足
            if ((count + 1) >= argc)
            {
                printf("Warning: missing timestamp parameter,set to default (0.00s).\n");
                continue;
            }

            // 非法输入
            if (!((argv[count + 1][0] == '-' && isdigit(argv[count + 1][1])) || (isdigit(argv[count + 1][0]))))
            {
                printf("Warning: invalid timestamp parameter \"%s\",set to default (0.00s).\n", argv[count + 1]);
                continue;
            }

            // 读取时间戳起始
            count++;
            sscanf(argv[count], "%lf", &time_begin);
            printf("Timestamp begin: %lf(s).\n", time_begin);
            continue;
        }

        // 处理格式串指令
        if (!strcmp(argv[count], "-f"))
        {
            // 参数不足
            if ((count + 2) >= argc)
            {
                printf("Warning: missing format parameters,skip.\n");
                continue;
            }

            // 非法输入参数1
            if (argv[count + 1][0] == '-')
            {
                printf("Warning: invalid format parameters \"%s %s\",skip.\n", argv[count + 1], argv[count + 2]);
                continue;
            }

            // 非法输入参数2
            if (argv[count + 2][0] == '-')
            {
                printf("Warning: invalid format parameters \"%s %s\",skip.\n", argv[count + 1], argv[count + 2]);
                count++;
                continue;
            }

            // 非法的类型输入
            if (!(!strcmp(argv[count + 2], "uint") ||
                  !strcmp(argv[count + 2], "ubyte") ||
                  !strcmp(argv[count + 2], "uint8_t") ||
                  !strcmp(argv[count + 2], "uint16_t") ||
                  !strcmp(argv[count + 2], "uint32_t") ||
                  !strcmp(argv[count + 2], "uint64_t") ||
                  !strcmp(argv[count + 2], "int") ||
                  !strcmp(argv[count + 2], "sbyte") ||
                  !strcmp(argv[count + 2], "int8_t") ||
                  !strcmp(argv[count + 2], "int16_t") ||
                  !strcmp(argv[count + 2], "int32_t") ||
                  !strcmp(argv[count + 2], "int64_t") ||
                  !strcmp(argv[count + 2], "float") ||
                  !strcmp(argv[count + 2], "double")))
            {
                printf("Warning: invalid format type \"%s\",skip.\n", argv[count + 2]);
                count += 2;
                continue;
            }

            // 添加到类型列表中
            {
                char *p_param = argv[count + 1];
                int param_len = strlen(argv[count + 1]);
                format_list_node *target_format_node = nullptr;
                while (p_param - argv[count + 1] < param_len)
                {
                    // 分配空间
                    if (format_list_head == nullptr)
                    {
                        format_list_head = (format_list_node *)malloc(sizeof(format_list_node));
                        format_list_head->p_next = nullptr;
                        format_list_head->target_numID = -1;
                        memset(format_list_head->target_value_name_str, '\0', sizeof(format_list_head->target_value_name_str));
                        format_list_head->value_type_str = DEFAULT_VALUE_TYPE;
                        target_format_node = format_list_head;
                    }
                    else
                    {
                        target_format_node->p_next = (format_list_node *)malloc(sizeof(format_list_node));
                        target_format_node = target_format_node->p_next;
                        target_format_node->p_next = nullptr;
                        target_format_node->target_numID = -1;
                        memset(target_format_node->target_value_name_str, '\0', sizeof(target_format_node->target_value_name_str));
                        target_format_node->value_type_str = DEFAULT_VALUE_TYPE;
                    }

                    char value_identification[VARIABLE_NAME_STR_LENGTH_MAX] = {'\0'};
                    size_t id_count = 0;
                    while (p_param - argv[count + 1] < param_len)
                    {
                        value_identification[id_count++] = *p_param;
                        if (p_param[1] == ',')
                        {
                            p_param += 2;
                            break;
                        }
                        p_param++;
                    }

                    // 判定名称orID顺序
                    if (is_full_num(value_identification))
                        target_format_node->target_numID = str_to_value(value_identification);
                    else
                        sprintf(target_format_node->target_value_name_str, value_identification);
                    target_format_node->value_type_str = argv[count + 2];
                }
            }

            count += 2;
            continue;
        }

        // 其余输入作为文件输入
        {
            // 判定文件是否可以打开
            FILE *tempfile = fopen(argv[count], "rb");
            if (tempfile != nullptr)
            {
                static file_node *target_node = nullptr;
                if (file_list_head == nullptr)
                {
                    file_list_head = (file_node *)malloc(sizeof(file_node));
                    file_list_head->file_name_str = nullptr;
                    file_list_head->p_next = nullptr;

                    target_node = file_list_head;
                }
                else
                {
                    target_node->p_next = (file_node *)malloc(sizeof(file_node));
                    target_node = target_node->p_next;
                    target_node->file_name_str = nullptr;
                    target_node->p_next = nullptr;
                }
                target_node->file_name_str = argv[count];

                printf("File: \"%s\" open succeed.\n", argv[count]);
            }
            else
                printf("File: \"%s\" open failed.\n", argv[count]);

            fclose(tempfile);
        }
    }

    printf("\n\n");

    // 遍历文件
    file_node *target_file_node = file_list_head;
    while (target_file_node != nullptr)
    {
        double timestamp = time_begin;
        // 打开相关文件
        FILE *input_file = nullptr;
        FILE *output_file = nullptr;
        input_file = fopen(target_file_node->file_name_str, "rb");
        {
            // 分配输出文件名空间
            size_t buffer_len = strlen(target_file_node->file_name_str) + 10;
            char *buffer = (char *)malloc(buffer_len);
            memset(buffer, '\0', buffer_len);

            // 处理输出名称
            {
                sprintf(buffer, "%s", target_file_node->file_name_str);
                char *p_file_extension = buffer + strlen(buffer);

                // 退格到文件扩展名后方
                while (p_file_extension != buffer && *p_file_extension != '.')
                    p_file_extension--;

                // 无扩展名文件
                if (p_file_extension == buffer)
                    sprintf(buffer + strlen(buffer), ".csv");
                else
                    sprintf(p_file_extension, ".csv");
            }

            // 打开输出文件并释放输出空间
            output_file = fopen(buffer, "wb");
            free(buffer);
        }

        if (input_file == nullptr || output_file == nullptr)
        {
            target_file_node = target_file_node->p_next;
            continue;
        }

        printf("Solving file: \"%s\"\n", target_file_node->file_name_str);
        printf("----------------------------------------------------------------\n");
        printf("%-40s %-16s %s\n", "Value Name", "Out Type", "ID");
        printf("----------------------------------------------------------------\n");

        char segment_buffer[SEGMENT_BUFF_LENGTH] = {'\0'};

        size_t total_line;

        // 跳过非数据行
        while (true)
        {
            f_getline(input_file, segment_buffer, sizeof(segment_buffer));
            if (segment_buffer[0] == '-')
            {
                sscanf(segment_buffer + 1, "%zu", &total_line);
                f_seek_pre_line_begin(input_file);
                break;
            }
        }

        // 处理首行数据
        {
            // 分配数值链表节点
            value_list_head = (value_node *)malloc(sizeof(value_node));
            value_list_head->p_next = nullptr;
            value_list_head->value_type_str = DEFAULT_VALUE_TYPE;
            value_list_head->raw_value = 0x0000000000000000;
            memset(value_list_head->value_name_str, '\0', sizeof(value_list_head->value_name_str));

            char value_name_str[VARIABLE_NAME_STR_LENGTH_MAX] = {'\0'};
            char value_str[100] = {'\0'};
            char time_str[100] = {'\0'};

            f_getline(input_file, segment_buffer, sizeof(segment_buffer));

            // -0000475879| 0|   D:FE49E582 snoop               0000 .l\Dip_SignalGet_MotCurrent+0x2   66.903us
            sscanf(segment_buffer, "%*s%*s%*s%*s%s%s%s", value_str, value_name_str, time_str);
            // 记录首个节点信息
            strcpy(value_list_head->value_name_str, value_name_str);
            value_list_head->raw_value = hex_to_value(value_str);
        }

        // 处理第一个数值循环记录
        {
            value_node *target_value_node = value_list_head;
            while (true)
            {
                char value_name_str[VARIABLE_NAME_STR_LENGTH_MAX] = {'\0'};
                char value_str[100] = {'\0'};
                char time_str[100] = {'\0'};

                // 获取行
                memset(segment_buffer, '\0', sizeof(segment_buffer));
                f_getline(input_file, segment_buffer, sizeof(segment_buffer));
                sscanf(segment_buffer, "%*s%*s%*s%*s%s%s%s", value_str, value_name_str, time_str);

                // 完成回环
                if (!strcmp(value_name_str, value_list_head->value_name_str))
                {
                    // 回转到文件的数据行起始位置
                    memset(segment_buffer, '\0', sizeof(segment_buffer));
                    fseek(input_file, 0, SEEK_SET);
                    // 跳过非数据行
                    while (true)
                    {
                        f_getline(input_file, segment_buffer, sizeof(segment_buffer));
                        if (segment_buffer[0] == '-')
                        {
                            sscanf(segment_buffer + 1, "%zu", &total_line);
                            f_seek_pre_line_begin(input_file);
                            break;
                        }
                    }
                    break;
                }

                // 未完成回环,分配数值链表节点
                target_value_node->p_next = (value_node *)malloc(sizeof(value_node));
                target_value_node = target_value_node->p_next;
                target_value_node->p_next = nullptr;
                target_value_node->value_type_str = DEFAULT_VALUE_TYPE;
                target_value_node->raw_value = 0x0000000000000000;
                memset(target_value_node->value_name_str, '\0', sizeof(target_value_node->value_name_str));

                // 记录节点信息
                strcpy(target_value_node->value_name_str, value_name_str);
                target_value_node->raw_value = hex_to_value(value_str);
            }
        }

        // 展示数据类型输出及类型匹配
        {
            value_node *target_value_node = value_list_head;
            int value_count = 0;

            while (target_value_node != nullptr)
            {
                // 查找是否由对应的类型列表
                {
                    format_list_node *target_format_node = format_list_head;
                    while (target_format_node != nullptr)
                    {
                        // 检查对应的名称（宽松匹配）
                        if ((target_format_node->target_value_name_str[0] != '\0') && (strstr(target_value_node->value_name_str, target_format_node->target_value_name_str) != nullptr))
                        {
                            target_value_node->value_type_str = target_format_node->value_type_str;
                            break;
                        }
                        else if (value_count == target_format_node->target_numID)
                        {
                            // 数字ID仅生效一次，随后记录名称以适配批量处理文件时指定格式
                            sprintf(target_format_node->target_value_name_str, target_value_node->value_name_str);
                            target_format_node->target_numID = -1;
                            target_value_node->value_type_str = target_format_node->value_type_str;
                            break;
                        }

                        target_format_node = target_format_node->p_next;
                    }

                    // 一些预先标识的变量
                    if (strstr(target_value_node->value_name_str, "wmrmeasured_speed") ||
                        strstr(target_value_node->value_name_str, "wmrcontrol_signal") ||
                        strstr(target_value_node->value_name_str, "tagetSpeed") ||
                        strstr(target_value_node->value_name_str, "WinUp_PID") ||
                        strstr(target_value_node->value_name_str, "WinDw_PID"))
                    {
                        target_value_node->value_type_str = "float";
                    }
                }

                printf("%-40s %-16s [%02d]\n", target_value_node->value_name_str, target_value_node->value_type_str, value_count++);
                target_value_node = target_value_node->p_next;
            }
        }

        // 输出csv抬头部分
        {
            value_node *target_value_node = value_list_head;
            fprintf(output_file, "time");
            while (target_value_node != nullptr)
            {
                fprintf(output_file, ",%s", target_value_node->value_name_str);
                target_value_node = target_value_node->p_next;
            }
            fprintf(output_file, "\n");
        }

        // 循环处理数据
        {
            printf("----------------------------------------------------------------\n");

            memset(segment_buffer, '\0', sizeof(segment_buffer));
            size_t line_count = -1;
            while (f_getline(input_file, segment_buffer, sizeof(segment_buffer)) != 0)
            {
                char value_name_str[VARIABLE_NAME_STR_LENGTH_MAX] = {'\0'};
                char value_str[100] = {'\0'};
                char time_str[100] = {'\0'};
                sscanf(segment_buffer, "%*s%*s%*s%*s%s%s%s", value_str, value_name_str, time_str);

                // 处理时间戳
                {
                    double time = 0.0;
                    sscanf(time_str, "%lf", &time);

                    // 处理时间戳
                    if (strstr(time_str, "us"))
                        timestamp += time / (1000 * 1000);
                    else if (strstr(time_str, "ms"))
                        timestamp += time / (1000);
                    else if (strstr(time_str, "s"))
                        timestamp += time;
                }

                // 此处不能太频繁打印，否则影响速度
                line_count++;
                if ((line_count % 10000) == 0)
                    progress_print(line_count, total_line, true);

                // 处理数值链表
                value_node *target_value_node = value_list_head;
                fprintf(output_file, "%lf", timestamp);
                while (target_value_node != nullptr)
                {
                    // 更新对应的值
                    if (!strcmp(value_name_str, target_value_node->value_name_str))
                        target_value_node->raw_value = hex_to_value(value_str);

                    // 按类型输出值
                    {
                        // int8_t sbyte
                        if (!strcmp(target_value_node->value_type_str, "sbyte") ||
                            !strcmp(target_value_node->value_type_str, "int8_t"))
                        {
                            int8_t *value = (int8_t *)&(target_value_node->raw_value);
                            fprintf(output_file, ",%d", *value);
                        }
                        // int16_t
                        else if (!strcmp(target_value_node->value_type_str, "int16_t"))
                        {
                            int16_t *value = (int16_t *)&(target_value_node->raw_value);
                            fprintf(output_file, ",%d", *value);
                        }
                        // int32_t int
                        else if (!strcmp(target_value_node->value_type_str, "int32_t"))
                        {
                            int32_t *value = (int32_t *)&(target_value_node->raw_value);
                            fprintf(output_file, ",%d", *value);
                        }
                        // int64_t
                        else if (!strcmp(target_value_node->value_type_str, "int64_t"))
                        {
                            int64_t *value = (int64_t *)&(target_value_node->raw_value);
                            fprintf(output_file, ",%lld", *value);
                        }
                        // float 类型
                        else if (!strcmp(target_value_node->value_type_str, "float"))
                        {
                            float *value = (float *)&(target_value_node->raw_value);
                            fprintf(output_file, ",%f", *value);
                        }
                        // double 类型
                        else if (!strcmp(target_value_node->value_type_str, "float"))
                        {
                            double *value = (double *)&(target_value_node->raw_value);
                            fprintf(output_file, ",%lf", *value);
                        }
                        // 其它类型全部按照uint输出
                        else
                            fprintf(output_file, ",%llu", target_value_node->raw_value);
                    }
                    target_value_node = target_value_node->p_next;
                }
                fprintf(output_file, "\n");

                // 清理缓冲区
                memset(segment_buffer, '\0', sizeof(segment_buffer));
            }

            progress_print(line_count, total_line, true);
            printf("\n\n\n");
        }

        // 清理数据列表
        if (value_list_head != nullptr)
        {
            value_node *target_value_node = value_list_head;
            while (target_value_node->p_next != nullptr)
            {
                value_node *to_be_free = target_value_node;
                target_value_node = target_value_node->p_next;
                free(to_be_free);
            }
            free(target_value_node);
        }

        target_file_node = target_file_node->p_next;
        fclose(input_file);
        fclose(output_file);
    }

    // 清理类型列表
    if (format_list_head != nullptr)
    {
        format_list_node *target_format_node = format_list_head;
        while (target_format_node->p_next != nullptr)
        {
            format_list_node *to_be_free = target_format_node;
            target_format_node = target_format_node->p_next;
            free(to_be_free);
        }
        free(target_format_node);
    }

    // 无输入文件时直接退出
    if (file_list_head == nullptr)
    {
        printf("No input files,exit.\n");
        return -1;
    }
    else
    {
        // 清理文件列表后退出
        file_node *target_file_node = file_list_head;
        while (target_file_node->p_next != nullptr)
        {
            file_node *to_be_free = target_file_node;
            target_file_node = target_file_node->p_next;
            free(to_be_free);
        }
        return 0;
    }
}