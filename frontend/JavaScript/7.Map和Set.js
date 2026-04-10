/**
 * JavaScript的默认对象表示方式{}可以视为其他语言中的Map或Dictionary的数据结构，即一组键值对。
 * 但是JavaScript的对象有个小问题，就是键必须是字符串。但实际上Number或者其他数据类型作为键也是非常合理的。
 * 为了解决这个问题，最新的ES6规范引入了新的数据类型Map
 *
 */

// Map是一种新的数据结构，类似于对象，也是键值对的集合，但它的键可以是任意类型的值，而不仅仅是字符串。
let m = new Map([['one', 1], ['two', 2], ['three', 3]]);
console.log(m); // Map(3) { 'one' => 1, 'two' => 2, 'three' => 3 }
// 底层是哈希表实现的，所以Map的查找效率非常高，平均时间复杂度为O(1)。

// Map的常用方法
m.set('four', 4); // 添加键值对
console.log(m.get('two')); // 2 获取键对应的值
console.log(m.has('three')); // true 判断是否存在某个键
m.delete('one'); // 删除键值对
console.log(m.size); // 3 获取Map中键值对的数量
m.clear(); // 清空Map
console.log(m.size); // 0

// 由于一个Key只能对应一个Value，所以如果多次对同一个Key赋值，后面的值会覆盖前面的值。
m.set('a', 1);
m.set('a', 2);
console.log(m.get('a')); // 2

// Set也是ES6引入的一种新的数据结构，它类似于数组，但成员的值都是唯一的，没有重复的值。
let s = new Set([1, 2, 3, 4, 4]);
console.log(s); // Set(4) { 1, 2, 3, 4 }
// Set的底层也是哈希表实现的，所以Set的查找效率也非常高，平均时间复杂度为O(1)。

// Set的常用方法
s.add(5); // 添加元素
console.log(s.has(3)); // true 判断是否存在某个元素
s.delete(2); // 删除元素
console.log(s.size); // 4 获取Set中元素的数量
s.clear(); // 清空Set
console.log(s.size); // 0

// Set会自动去重，所以如果多次添加同一个值，Set只会保留一个。
s.add(1);
s.add(1);
console.log(s.size); // 1