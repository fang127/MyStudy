#include <iostream>
#include <vector>
// 直接插入排序
// 最稳定的排序算法，时间复杂度O(n^2)，空间复杂度O(1)，适合小规模数据排序
void dictSort(std::vector<int> &vec)
{
    for (int i = 1; i < vec.size(); ++i)
    {
        int key = vec[i];
        int j = i - 1;
        for (; j >= 0 && vec[j] > key; --j) vec[j + 1] = vec[j];
        vec[j + 1] = key;
    }
}

// 希尔排序
// 插入排序的改进版本，时间复杂度O(n^1.3)，空间复杂度O(1)，适合中等规模数据排序
// 希尔排序通过将数据分成多个子序列进行插入排序，逐渐减少子序列的间隔，最终达到整体有序的效果
// 不稳定的排序算法，因为在分组过程中可能会改变相同元素的相对位置
void shellSort(std::vector<int> &vec)
{
    for (int i = vec.size() / 2; i > 0; i /= 2)
    {
        for (int j = i; j < vec.size(); ++j)
        {
            int key = vec[j];
            int k = j - i;
            for (; k >= 0 && vec[k] > key; k -= i) vec[k + i] = vec[k];
            vec[k + i] = key;
        }
    }
}

// 冒泡排序
// 最简单的排序算法，时间复杂度O(n^2)，空间复杂度O(1)，适合小规模数据排序
// 不稳定的排序算法，因为在交换过程中可能会改变相同元素的相对位置
void bubbleSort(std::vector<int> &vec)
{
    for (int i = 0; i < vec.size() - 1; ++i)
    {
        for (int j = vec.size() - 1; j > i; --j)
            if (vec[j - 1] > vec[j]) std::swap(vec[j - 1], vec[j]);
    }
}

// 快速排序
// 最常用的排序算法，平均时间复杂度O(n log n)，空间复杂度O(log
// n)，适合大规模数据排序
// 不稳定的排序算法，因为在分区过程中可能会改变相同元素的相对位置
void quickSort(std::vector<int> &vec, int left, int right)
{
    if (left >= right) return;
    int idx = left + rand() % (right - left + 1);
    int pivot = vec[idx];
    std::swap(vec[idx], vec[left]);
    int i = left + 1, j = right;
    while (true)
    {
        while (i <= j && vec[i] < pivot) ++i;
        while (i <= j && vec[j] > pivot) --j;
        if (i >= j) break;
        std::swap(vec[i], vec[j]);
        ++i;
        --j;
    }

    std::swap(vec[left], vec[j]);
    quickSort(vec, left, j - 1);
    quickSort(vec, j + 1, right);
}

// 直接选择排序
void selectSort(std::vector<int> &vec)
{
    for (int i = 0; i < vec.size() - 1; ++i)
    {
        int minIdx = i;
        for (int j = i + 1; j < vec.size(); ++j)
            if (vec[j] < vec[minIdx]) minIdx = j;
        std::swap(vec[i], vec[minIdx]);
    }
}

// 堆排序
// 时间复杂度O(n log n)，空间复杂度O(1)，适合大规模数据排序
// 不稳定的排序算法，因为在堆调整过程中可能会改变相同元素的相对位置

// 通用堆调整函数：调整以parent为根的子树为最大堆
// @param vec: 待调整的数组
// @param parent: 要调整的父节点索引
// @param heapSize: 当前堆的有效大小（已排序的元素不参与调整）
void maxHeapify(std::vector<int> &vec, int parent, int heapSize)
{
    while (true)
    {
        int leftChild = 2 * parent + 1;  // 左子节点索引
        int rightChild = 2 * parent + 2; // 右子节点索引
        int maxChild = parent;           // 记录父/子节点中最大值的索引

        // 找左/右子节点中更大的那个
        if (leftChild < heapSize && vec[leftChild] > vec[maxChild])
            maxChild = leftChild;
        if (rightChild < heapSize && vec[rightChild] > vec[maxChild])
            maxChild = rightChild;

        // 如果父节点已是最大值，无需调整，退出循环
        if (maxChild == parent) break;

        // 交换父节点和最大子节点，继续向下调整
        std::swap(vec[parent], vec[maxChild]);
        parent = maxChild; // 父节点指针下移到交换后的子节点位置
    }
}

// 构建最大堆
void buildMaxHeap(std::vector<int> &vec)
{
    int n = vec.size();
    // 从最后一个非叶子节点开始，自下而上调整堆
    for (int i = n / 2 - 1; i >= 0; --i)
        maxHeapify(vec, i, n); // 堆有效大小为整个数组
}

// 堆排序主函数
void heapSort(std::vector<int> &vec)
{
    if (vec.empty() || vec.size() == 1) return; // 空数组或单元素数组无需排序

    buildMaxHeap(vec); // 第一步：构建最大堆

    // 第二步：逐个将堆顶（最大值）交换到数组末尾，然后调整剩余堆
    for (int i = vec.size() - 1; i > 0; --i)
    {
        std::swap(vec[0], vec[i]); // 堆顶最大值交换到当前未排序区间的末尾
        maxHeapify(vec, 0, i); // 调整剩余i个元素为最大堆（已排序的i位置不参与）
    }
}

// 归并排序
// 时间复杂度O(n log n)，空间复杂度O(n)，适合大规模数据排序
// 稳定的排序算法，因为在合并过程中不会改变相同元素的相对位置
void merge(std::vector<int> &vec, int left, int mid, int right)
{
    std::vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;
    while (i <= mid && j <= right)
        if (vec[i] <= vec[j])
            temp[k++] = vec[i++];
        else
            temp[k++] = vec[j++];

    while (i <= mid) temp[k++] = vec[i++];
    while (j <= right) temp[k++] = vec[j++];

    for (int m = 0; m < temp.size(); ++m) vec[left + m] = temp[m];
}

void mergeSort(std::vector<int> &vec, int left, int right)
{
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(vec, left, mid);
    mergeSort(vec, mid + 1, right);

    merge(vec, left, mid, right);
}

int main()
{
    std::vector<int> vec = {5, 2, 9, 1, 5, 6};
    heapSort(vec);
    for (int num : vec) std::cout << num << " ";
    std::cout << std::endl;

    return 0;
}