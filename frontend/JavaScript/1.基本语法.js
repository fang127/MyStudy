// var和let都是变量
var x; // 函数作用域
let y = 5; // 块级作用域
const z = 10; // 常量必须初始化
console.log(x, y, z); // undefined
let name = "Alice"; // 字符串
let age = 30; // 数字
let isStudent = true; // 布尔值
console.log(name, age, isStudent);
let emptyValue = null; // null表示空值
let notDefined; // undefined表示未定义
console.log(emptyValue, notDefined);
// 缩进不是JavaScript语法要求必须的，但缩进有助于我们理解代码的层次
if(2 > 1) {
    let blockVar = "I am inside the block";
    console.log(blockVar); // I am inside the block
    if(3 > 2) {
        let nestedBlockVar = "I am inside the nested block";
        console.log(nestedBlockVar); // I am inside the nested block
    }
}

// 在同一个页面的不同的JavaScript文件中，如果都不用var申明，恰好都使用了变量i，将造成变量i互相影响，产生难以调试的错误结果。
i = 10; // 没有声明变量，i会成为全局变量
console.log(i); // 10
// 可以使用严格模式来避免这种情况
"use strict";
j = 20; // 在严格模式下，未声明的变量会抛出错误
console.log(j); // ReferenceError: j is not defined

// 注释
/*
这是一个多行注释
可以用于解释代码的功能
或者暂时禁用某些代码
*/