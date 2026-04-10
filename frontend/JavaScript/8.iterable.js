// 迭代器
// 具有Symbol.iterator属性的对象被称为可迭代对象，for...of循环会调用这个属性返回一个迭代器对象
// 迭代器对象具有next方法，每次调用返回一个对象，包含value和done属性
// value表示当前迭代的值，done表示是否迭代完成

// 例如，数组是可迭代对象，可以使用for...of循环遍历
const arr = [1, 2, 3];
for (const item of arr) {
  console.log(item); // 输出1, 2, 3
}

let m = new Map();
m.set('a', 1);
m.set('b', 2);
m.set('c', 3);

for (const [key, value] of m) {
  console.log(key, value); // 输出a 1, b 2, c 3
}

let s = new Set();
s.add(1);
s.add(2);
s.add(3);

for (const item of s) {
  console.log(item); // 输出1, 2, 3
}

// for .. in 和 for .. of 的区别
// for .. in 用于遍历对象的可枚举属性，返回属性名
// for .. of 用于遍历可迭代对象，返回迭代的值

// iterable内部右forEach方法实现了迭代器协议，它接收一个回调函数作为参数，回调函数会在每次迭代时被调用，接收当前迭代的值、索引和整个数组作为参数
arr.forEach((value, index, array) => {
  console.log(value, index, array); // 输出1 0 [1, 2, 3], 2 1 [1, 2, 3], 3 2 [1, 2, 3]
});