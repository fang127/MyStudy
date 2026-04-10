// Array可以包含任意数据类型，并通过索引来访问每个元素
let arr = [1, 'hello', true, null, undefined, { name: 'Alice' }, [1, 2, 3]];
console.log(arr.length); // 输出: 7
console.log(arr[0]); // 输出: 1
console.log(arr[1]); // 输出: 'hello'
console.log(arr[5].name); // 输出: 'Alice'
console.log(arr[6][1]); // 输出: 2

// 给Array的length属性赋值可以改变数组的长度
arr.length = 3;
console.log(arr); // 输出: [1, 'hello', true]

// Array可以通过索引把对应的元素修改为新的值
// 如果通过索引赋值时，索引超过了范围，同样会引起Array大小的变化
arr[0] = 42;
console.log(arr[0]); // 输出: 42

// 常用的Array方法
let numbers = [1, 2, 3, 4, 5];

// push方法在数组末尾添加一个或多个元素，并返回新的长度
numbers.push(6);
console.log(numbers); // 输出: [1, 2, 3, 4, 5, 6]

// pop方法删除数组末尾的元素，并返回被删除的元素
let last = numbers.pop();
console.log(last); // 输出: 6
console.log(numbers); // 输出: [1, 2, 3, 4, 5]

// indexOf方法返回数组中第一个匹配元素的索引，如果没有找到则返回-1
console.log(numbers.indexOf(3)); // 输出: 2
console.log(numbers.indexOf(10)); // 输出: -1

// slice方法返回一个新的数组，包含从开始索引到结束索引（不包括结束索引）的元素
let sliced = numbers.slice(1, 4);
console.log(sliced); // 输出: [2, 3, 4]

// unshift方法在数组开头添加一个或多个元素，并返回新的长度
numbers.unshift(0);
console.log(numbers); // 输出: [0, 1, 2, 3, 4, 5]

// shift方法删除数组开头的元素，并返回被删除的元素
let first = numbers.shift();
console.log(first); // 输出: 0
console.log(numbers); // 输出: [1, 2, 3, 4, 5]

// sort方法对数组元素进行排序，默认按照字符串Unicode码点进行排序
numbers.sort();
console.log(numbers); // 输出: [1, 2, 3, 4, 5]

// reverse方法反转数组中元素的顺序
numbers.reverse();
console.log(numbers); // 输出: [5, 4, 3, 2, 1]

// splice方法可以在任意位置添加或删除元素
// 删除元素：splice(start, deleteCount)
numbers.splice(2, 1); // 从索引2开始删除1个元素
console.log(numbers); // 输出: [5, 4, 2, 1]

// 添加元素：splice(start, deleteCount, item1, item2, ...)
numbers.splice(2, 0, 3); // 从索引2开始删除0个元素，并添加3
console.log(numbers); // 输出: [5, 4, 3, 2, 1]

// concat方法用于合并两个或多个数组，并返回一个新的数组
let moreNumbers = [6, 7, 8];
let combined = numbers.concat(moreNumbers);
console.log(combined); // 输出: [5, 4, 3, 2, 1, 6, 7, 8]

// join方法将数组元素转换为字符串，并使用指定的分隔符连接
let joined = numbers.join('-');
console.log(joined); // 输出: '5-4-3-2-1'

// 多维数组
let matrix = [
  [1, 2, 3],
  [4, 5, 6],
  [7, 8, 9]
];
console.log(matrix[0][1]); // 输出: 2
console.log(matrix[1][2]); // 输出: 6