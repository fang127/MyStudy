// 函数
function abs01(x) {
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
}

// 函数也是一种对象
// 这样是一个匿名函数，赋值给变量abs
let abs02 = function (x) {
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
};

console.log(abs01(-5)); // 5
console.log(abs02(-10)); // 5

// 由于JavaScript允许传入任意个参数而不影响调用，因此传入的参数比定义的参数多也没有问题，虽然函数内部并不需要这些参数
console.log(abs01(-5, 1, 2)); // 5
console.log(abs02(-10, 1, 2)); // 5
// 传入的参数少了也没有问题，缺少的参数会被自动补齐为undefined
console.log(abs01()); // NaN
console.log(abs02()); // NaN
// 要解决这个问题，可以在函数内部检查参数的个数，或者使用ES6的默认参数值来避免undefined的情况
function abs03(x = 0) {
    if(typeof x !== 'number') {
        throw '参数必须是数字';
    }
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
}
console.log(abs03(-5)); // 5
// console.log(abs03()); // 0
// console.log(abs03('a')); // 抛出异常：参数必须是数字

// JavaScript还有一个免费赠送的关键字arguments，它只在函数内部起作用，并且永远指向当前函数的调用者传入的所有参数。arguments类似Array但它不是一个Array
function foo(x) {
    console.log('参数个数：' + arguments.length);
    console.log('第一个参数：' + arguments[0]);
    console.log('第二个参数：' + arguments[1]);

    // 或者这样
    for(let i = 0; i < arguments.length; i++) {
        console.log('参数' + i + '：' + arguments[i]);
    }
}
// 利用arguments，你可以获得调用者传入的所有参数。也就是说，即使函数不定义任何参数，还是可以拿到参数的值
foo(1, 2); // 参数个数：2 第一个参数：1 第二个参数：2
foo(1); // 参数个数：1 第一个参数：1 第二个参数：undefined
foo(); // 参数个数：0 第一个参数：undefined 第二个参数：undefined
// 实际上arguments最常用于判断传入参数的个数。

// 引入rest参数后，函数定义时可以使用...args来表示不定数量的参数，这些参数会被放在一个数组中
function foo02(...args) {
    console.log('参数个数：' + args.length);
    console.log('第一个参数：' + args[0]);
    console.log('第二个参数：' + args[1]);

    // 或者这样
    for(let i = 0; i < args.length; i++) {
        console.log('参数' + i + '：' + args[i]);
    }
}
foo02(1, 2); // 参数个数：2 第一个参数：1 第二个参数：2
foo02(1); // 参数个数：1 第一个参数：1 第二个参数：undefined
foo02(); // 参数个数：0 第一个参数：undefined 第二个参数：undefined