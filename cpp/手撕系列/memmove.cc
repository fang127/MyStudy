#include <iostream>
#include <cstring>

void *myMemmove(void *dst, const void *src, size_t n)
{
    if (!dst || !src || n == 0) return nullptr;

    unsigned char *d = static_cast<unsigned char *>(dst);
    const unsigned char *s = static_cast<const unsigned char *>(src);
    if (d < s || d >= s + n)
        for (size_t i = 0; i < n; ++i) d[i] = s[i];
    else
        for (size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    return dst;
}

int main()
{
    // 测试场景1：普通拷贝（无重叠）
    char src1[] = "hello world";
    char dst1[20] = {0};
    myMemmove(dst1, src1, strlen(src1) + 1);
    std::cout << "测试1（普通拷贝）：" << dst1 << std::endl; // 输出 hello world

    // 测试场景2：内存重叠（dst 在 src 后方）
    char str2[] = "1234567890";
    myMemmove(str2 + 2, str2,
              5); // 目标地址：str2[2]，源地址：str2[0]，拷贝5个字节
    std::cout << "测试2（内存重叠，my_memmove 结果）：" << str2
              << std::endl; // 正确输出 1212345890

    // 测试场景3：内存重叠（dst 在 src 前方）
    char str3[] = "1234567890";
    myMemmove(str3, str3 + 2,
              5); // 目标地址：str3[0]，源地址：str3[2]，拷贝5个字节
    std::cout << "测试3（内存重叠，my_memmove 结果）：" << str3
              << std::endl; // 正确输出 3456767890

    return 0;
}