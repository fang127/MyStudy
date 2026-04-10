#include <iostream>
#include <string.h>
void *myMemcpy(void *dst, const void *src, size_t n)
{
    if (!dst || !src || n <= 0) return nullptr;
    unsigned char *d = static_cast<unsigned char *>(dst);
    const unsigned char *s = static_cast<const unsigned char *>(src);
    for (int i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}

int main()
{
    // 测试场景1：普通拷贝（无重叠）
    char src1[] = "hello world";
    char dst1[20] = {0};
    myMemcpy(dst1, src1, strlen(src1) + 1);                  // 包含 '\0'
    std::cout << "测试1（普通拷贝）：" << dst1 << std::endl; // 输出 hello world

    // 测试场景2：内存重叠（my_memcpy 结果未定义）
    char str2[] = "1234567890";
    myMemcpy(str2 + 2, str2, 5); // 目标地址在源地址后方，重叠
    std::cout << "测试2（内存重叠，my_memcpy 结果）：" << str2
              << std::endl; // 可能输出 1212345890（结果不可靠）

    return 0;
}