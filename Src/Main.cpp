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
    // 处理指令输入
    for (int count = 1; count < argc; count++)
    {
        // 处理时间戳
        if (!strcmp(argv[count], "-t"))
        {
            // 参数截至
            if ((count + 1) > argc)
                continue;

            // 读取时间戳起始
            count++;
            sscanf(argv[count], "%lf", &time_begin);
            continue;
        }

        // 处理格式串指令
        {
        }

        // 其余输入作为文件输入
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
        // printf("%s\n", argv[count]);
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
            char buffer[100] = {'\0'};
            sprintf(buffer, "%s%s", target_file_node->file_name_str, ".csv");
            output_file = fopen(buffer, "wb");
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

        char segment_buffer[1000] = {'\0'};

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

            char value_name_str[100] = {'\0'};
            char value_str[100] = {'\0'};
            char time_str[100] = {'\0'};

            f_getline(input_file, segment_buffer, sizeof(segment_buffer));

            // -0000475879| 0|   D:FE49E582 snoop               0000 .l\Dip_SignalGet_MotCurrent+0x2   66.903us
            sscanf(segment_buffer, "%*s%*s%*s%*s%s%s%s", value_str, value_name_str, time_str);
            // 记录首个节点信息
            strcpy(value_list_head->value_name_str, value_name_str);
            value_list_head->raw_value = hex_to_decimal(value_str);
        }

        // 处理第一个数值循环记录
        {
            value_node *target_value_node = value_list_head;
            while (true)
            {
                char value_name_str[100] = {'\0'};
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
                target_value_node->raw_value = hex_to_decimal(value_str);
            }
        }

        // 展示数据类型输出
        {
            value_node *target_value_node = value_list_head;
            size_t value_count = 0;

            while (target_value_node != nullptr)
            {
                printf("%-40s %-16s [%d]\n", target_value_node->value_name_str, target_value_node->value_type_str, value_count++);
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
                char value_name_str[100] = {'\0'};
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

                line_count++;
                if ((line_count % 10000) == 0)
                    progress_print(line_count, total_line, true);
                // printf("timestamp: %lfs %s\n", timestamp, segment_buffer);

                // 处理数值链表
                value_node *target_value_node = value_list_head;
                fprintf(output_file, "%lf", timestamp);
                while (target_value_node != nullptr)
                {
                    // 更新对应的值
                    if (!strcmp(value_name_str, target_value_node->value_name_str))
                        target_value_node->raw_value = hex_to_decimal(value_str);

                    // 特殊处理
                    if (strstr(target_value_node->value_name_str, "wmrmeasured_speed"))
                    {
                        float *value = (float *)&(target_value_node->raw_value);
                        fprintf(output_file, ",%f", *value);
                    }
                    else
                        fprintf(output_file, ",%llu", target_value_node->raw_value);

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
        {
        }

        target_file_node = target_file_node->p_next;
        fclose(input_file);
        fclose(output_file);
    }

    return 0;
}