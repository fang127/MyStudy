// JavaScript不区分整数和浮点数，统一用Number表示
// JavaScript的整数最大范围不是±263，而是±253，因此，超过253的整数就可能无法精确表示
let a = 10; // 整数
let b = 3.14; // 浮点数

// 字符串类型 单引号或双引号都可以表示字符串
// 字符串是不可变的，如果对字符串的某个索引赋值，不会有任何错误，但是，也没有任何效果
let str1 = "Hello, World!";
let str2 = 'JavaScript is fun!';
let str3 = `
    这是一个多行字符串
    可以包含换行和缩进
    也可以使用${str1}和${str2}这样的变量插值
`
console.log(str1);
console.log(str2);
console.log(str3);
// 字符串可以利用+号进行连接
let greeting = str1 + " " + str2; // "Hello, World! JavaScript is fun!"
// 模板字符串（使用反引号）可以嵌入变量和表达式
let name = "Alice";
let age = 30;
let introduction = `My name is ${name} and I am ${age} years old.`; // "My name is Alice and I am 30 years old."
let c = name[0]; // "A"，字符串可以像数组一样访问每个字符
let length = name.length; // 5，字符串的长度
let upperCaseName = name.toUpperCase(); // "ALICE"，字符串的方法可以返回一个新的字符串，原字符串不变
let lowerCaseName = name.toLowerCase(); // "alice"
let s = 'hello world';
s.indexOf('o'); // 4，返回第一个'o'的索引
s.lastIndexOf('o'); // 7，返回最后一个'o'的索引
s.includes('world'); // true，判断字符串是否包含某个子串
s.startsWith('hello'); // true，判断字符串是否以某个子串开头
s.endsWith('world'); // true，判断字符串是否以某个子串结尾
s.substring(0, 5); // "hello"，返回字符串的子串，从索引0开始，长度为5
s.split(' '); // ["hello", "world"]，将字符串分割成数组，分隔符是空格


// 布尔类型
let isTrue = true;
let isFalse = false;

// null和undefined
let emptyValue = null; // 表示空值
let undefinedValue; // 未定义的变量，默认值为undefined

// Number可以直接做四则运算
let sum = a + b; // 13.14
let difference = a - b; // 6.86
let product = a * b; // 31.4
let quotient = a / b; // 3.184713375796178
let modulo = a % 3; // 1

// 打印Number的最大安全整数
console.log(Number.MAX_SAFE_INTEGER); // 9007199254740991
console.log(Number.MIN_SAFE_INTEGER); // -9007199254740991
// BigInt可以表示任意大的整数
let bigIntValue = 9007199254740991n; // 使用n后缀表示BigInt
console.log(bigIntValue + 1n); // 9007199254740992n
console.log(bigIntValue + 2n); // 9007199254740993n
// 使用BigInt可以正常进行加减乘除等运算，结果仍然是一个BigInt，但不能把一个BigInt和一个Number放在一起运算


// JavaScript允许对任意数据类型做比较
console.log(10 == "10"); // true，类型转换后比较
console.log(10 === "10"); // false，严格比较，不进行类型转换
console.log(null == undefined); // true，null和undefined相等
console.log(null === undefined); // false，严格比较（不会进行类型转换，如果不一致，则false）不相等
console.log(NaN === NaN); // false，NaN不等于任何值，包括它自己
// 唯一能判断NaN的方法是通过isNaN()函数
if(isNaN(NaN)) {
    console.log("NaN is not a number");
} else {
    console.log("NaN is a number");
}

// JavaScript中的变量可以随时改变类型
let variable = 42; // 现在是一个数字
variable = "Now I'm a string"; // 现在是一个字符串
variable = true; // 现在是一个布尔值

// 数组
// JavaScript的数组可以包括任意数据类型
let array = [1, "two", true, null, undefined, { name: "Alice" }, [1, 2, 3]];
console.log(array[0]);
console.log(array[1]);
console.log(array[2]);
console.log(array[3]);
console.log(array[4]);
// 也可以通过Array构造函数创建数组
let array2 = new Array(1, "two", true);
console.log(array2[0]);
console.log(array2[1]);
console.log(array2[2]);

// 对象
// JavaScript的对象是一组由键-值组成的无序集合\
// 对象的键都是字符串类型，值可以是任意数据类型
var person = {
    name: "Alice",
    age: 30,
    isStudent: false,
    hobbies: ["reading", "traveling"],
    address: {
        city: "New York",
        country: "USA"
    }
};
console.log(person.name); // "Alice"
console.log(person.age); // 30
console.log(person.isStudent); // false
console.log(person.hobbies[0]); // "reading"
console.log(person.address.city); // "New York"
