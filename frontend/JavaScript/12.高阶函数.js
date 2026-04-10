/**
 * JavaScript的函数其实都指向某个变量。
 * 既然变量可以指向函数，函数的参数能接收变量，
 * 那么一个函数就可以接收另一个函数作为参数，这种函数就称之为高阶函数。
 */
function add(x, y, f) {
    return f(x) + f(y);
}

console.log(add(-5, 6, Math.abs)); // 11

// Array的map()函数就是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回一个新的Array。
var arr = [-1, -2, -3];
var absArr = arr.map(Math.abs);
console.log(absArr); // [1, 2, 3]

// reduce()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回一个值。
var arr = [1, 2, 3, 4];
var sum = arr.reduce(function(x, y) {
    return x + y;
});
console.log(sum); // 10

// filter()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回一个新的Array，包含所有满足条件的元素。
var arr = [1, 2, 3, 4];
var evenArr = arr.filter(function(x) {
    return x % 2 === 0;
});
console.log(evenArr); // [2, 4]

// filter()接收的回调函数，其实可以有多个参数。通常我们仅使用第一个参数，表示Array的某个元素。回调函数还可以接收另外两个参数，表示元素的位置和数组本身
var arr = ['A', 'B', 'C'];
var arr2 = arr.filter(function(x, index, self) {
    console.log(x, index, self);
    return true;
});
/*
A 0 [ 'A', 'B', 'C' ]
B 1 [ 'A', 'B', 'C' ]
C 2 [ 'A', 'B', 'C' ]
*/

// sort()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回一个新的Array，包含所有满足条件的元素。
var arr = [1, 3, 2];
arr.sort(function(x, y) {
    return x - y;
});
console.log(arr); // [1, 2, 3]

// every()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回一个布尔值，表示是否所有元素都满足条件。
var arr = [1, 2, 3];
var allPositive = arr.every(function(x) {
    return x > 0;
});
console.log(allPositive); // true

// find()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回第一个满足条件的元素。
var arr = [1, 2, 3];
var firstEven = arr.find(function(x) {
    return x % 2 === 0;
});
console.log(firstEven); // 2

// findIndex()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回第一个满足条件的元素的索引。
var arr = [1, 2, 3];
var firstEvenIndex = arr.findIndex(function(x) {
    return x % 2 === 0;
});
console.log(firstEvenIndex); // 1

// forEach()函数也是一个高阶函数，它接收一个函数作为参数，并对Array的每个元素执行这个函数，最后返回undefined。
var arr = [1, 2, 3];
arr.forEach(function(x) {
    console.log(x);
});
/*
1
2
3
*/