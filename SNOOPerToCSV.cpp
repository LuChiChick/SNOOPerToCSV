extern "C"
{
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "ctype.h"
}

// 行进到上一行起始位置
size_t f_seek_pre_line_begin(FILE *file)
{
    int count = 0;

    // 本身就在文件起始位置
    if (ftell(file) == 0)
        return 0;
    // 本身在文件结尾
    if (fgetc(file) == EOF)
    {
        // 移动到之前
        do
        {
            fseek(file, -2, SEEK_CUR);
            count--;
        } while (ftell(file) != 0 && fgetc(file) != '\n');

        return count;
    }

    // 其它情况返

    // 移动到上一行的结尾处
    while (ftell(file) != 0 && fgetc(file) != '\n')
    {
        fseek(file, -2, SEEK_CUR);
        count--;
    }

    fseek(file, -2, SEEK_CUR);
    count--;

    // 移动到上上一行的结尾处
    while (ftell(file) != 0 && fgetc(file) != '\n')
    {
        fseek(file, -2, SEEK_CUR);
        count--;
    }

    return count;
}

// 读取一行
size_t f_getline(FILE *file, char *buffer, size_t buffer_len)
{
    memset(buffer, '\0', buffer_len);
    int count = 0;

    while (true)
    {
        if (count >= buffer_len)
        {
            fseek(file, -count, SEEK_CUR);
            memset(buffer, '\0', buffer_len);
            return 0;
        }

        char ch = fgetc(file);

        if (ch == EOF && count == 0)
            return 0;
        if (ch == '\n' || ch == EOF)
        {
            buffer[count] = ch;
            return count + 1;
        }

        buffer[count] = ch;
        count++;
    }
}

// 16进制字符串转十进制数值（8字节）
uint64_t hex_to_decimal(const char *str)
{
    uint64_t value = 0;

    int len = strlen(str);
    // 循环读取
    for (int count = 0; count < len; count++)
    {
        value *= 16;
        if (str[count] >= '0' && str[count] <= '9')
            value += str[count] - '0';
        else
            value += toupper(str[count]) - 'A' + 10;
    }
    return value;
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
    uint64_t raw_value = 0x0000000000000000;
    value_node_struct *p_next;
} value_node;

// 文件列表
file_node *file_list_head = nullptr;
value_node *value_list_head = nullptr;
double timestamp = 0.0;

int main(int argc, char *argv[])
{
    // int argc = 4;
    // char argv[4][100] = {
    //     "awdawdawd",
    //     "16v_input.txt",
    //     "-t",
    //     "-32.69",
    // };

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
            sscanf(argv[count], "%lf", &timestamp);
            continue;
        }

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
        printf("%s\n", argv[count]);
    }

    // 遍历文件
    file_node *target_file_node = file_list_head;
    while (target_file_node != nullptr)
    {
        // 打开相关文件
        FILE *input_file = nullptr;
        FILE *output_file = nullptr;
        input_file = fopen(target_file_node->file_name_str, "rb");
        {
            char buffer[100] = {'\0'};
            sprintf(buffer, "%s%s", target_file_node->file_name_str, ".csv");
            output_file = fopen(buffer, "wb");
        }

        char segment_buffer[1000] = {'\0'};

        // 跳过第一行
        f_getline(input_file, segment_buffer, sizeof(segment_buffer));
        memset(segment_buffer, '\0', sizeof(segment_buffer));

        // 处理首行数据
        {
            // 分配数值链表节点
            value_list_head = (value_node *)malloc(sizeof(value_node));
            value_list_head->p_next = nullptr;
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

        // 处理第一个数值循环
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
                    // 回转到文件的第二行
                    memset(segment_buffer, '\0', sizeof(segment_buffer));
                    fseek(input_file, 0, SEEK_SET);
                    f_getline(input_file, segment_buffer, sizeof(segment_buffer));
                    break;
                }

                // 未完成回环,记录新的类型
                // 分配数值链表节点
                target_value_node->p_next = (value_node *)malloc(sizeof(value_node));
                target_value_node = target_value_node->p_next;
                target_value_node->p_next = nullptr;
                target_value_node->raw_value = 0x0000000000000000;
                memset(target_value_node->value_name_str, '\0', sizeof(target_value_node->value_name_str));

                // 记录节点信息
                strcpy(target_value_node->value_name_str, value_name_str);
                target_value_node->raw_value = hex_to_decimal(value_str);
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
            memset(segment_buffer, '\0', sizeof(segment_buffer));
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
                    timestamp += time / (1000 * 1000);
                }

                printf("timestamp: %lf %s\n", timestamp, segment_buffer);

                // 处理数值链表
                value_node *target_value_node = value_list_head;
                fprintf(output_file, "%lf", timestamp);
                while (target_value_node != nullptr)
                {
                    // 更新对应的值
                    if (!strcmp(value_name_str, target_value_node->value_name_str))
                        target_value_node->raw_value = hex_to_decimal(value_str);

                    // 输出当前值
                    fprintf(output_file, ",%d", target_value_node->raw_value + 0);
                    target_value_node = target_value_node->p_next;
                }
                fprintf(output_file, "\n");

                // 清理缓冲区
                memset(segment_buffer, '\0', sizeof(segment_buffer));
            }
        }

        target_file_node = target_file_node->p_next;
        fclose(input_file);
        fclose(output_file);
    }

    return 0;
}