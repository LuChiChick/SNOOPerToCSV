#include "Tool_Functions.hpp"

extern "C"
{
#include "string.h"
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

    // 其它情况

    // 移动到上一行的结尾处（回到本行起始位置）
    while (ftell(file) != 0 && fgetc(file) != '\n')
    {
        fseek(file, -2, SEEK_CUR);
        count--;
    }

    // 移动到上上一行的结尾处
    do
    {
        fseek(file, -2, SEEK_CUR);
        count--;
    } while (ftell(file) != 0 && fgetc(file) != '\n');

    return count;
}

// 读取文件一行
size_t f_getline(FILE *file, char *buffer, const size_t buffer_len)
{
    if (buffer == nullptr || file == nullptr)
        return 0;

    memset(buffer, '\0', buffer_len);

    size_t count = 0;

    // 循环读取
    while (true)
    {
        // 超长判定
        if (count + 1 == buffer_len)
            return buffer_len;

        // 正常读取
        char ch = fgetc(file);

        // 仅文件结尾时
        if (ch == EOF && count == 0)
            return 0;

        // 成功换行
        if (ch == '\n')
        {
            buffer[count] = '\n';
            return count + 1;
        }

        // 没有换行但是遇到了文件结尾
        if (ch == EOF)
        {
            buffer[count] = '\n';
            return count;
        }

        // 其它情况下正常复制
        buffer[count] = ch;
        count++;
    }
}

// 进度条打印
void progress_print(size_t completed, size_t total, bool reflush)
{

    // 消除单行内容
    if (reflush)
        putchar('\r');
    else
        putchar('\n');

    double p_percent = 1.0 * completed / total;

    putchar('[');

    // 输出完成部分填充
    size_t completed_count = (PROGRESS_LENGTH * p_percent) - 1;
    if (p_percent - (int)(p_percent) > 0.5)
        completed_count++;

    for (int count = completed_count; count > 0; count--)
        putchar('=');
    putchar('>');

    // 填充未完成部分
    for (int count = PROGRESS_LENGTH - (completed_count + 1); count > 0; count--)
        putchar(' ');
    putchar(']');

    // 进度数显
    printf("          %d/%d  (%%%.2lf)", completed, total, p_percent * 100);
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