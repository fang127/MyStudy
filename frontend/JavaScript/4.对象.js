// 对象是一种无序的集合数据类型，它由若干键值对组成。
// 用一个{...}表示一个对象，键值对以xxx: xxx形式申明，用,隔开。注意，最后一个键值对不需要在末尾加,，如果加了，有的浏览器（如低版本的IE）将报错。
let obj = {
    name: 'Alice',
    age: 30,
    city: 'New York',
    'favorite color': 'blue' // 键名中有空格，需要用引号括起来
};
// 访问属性是通过.操作符完成的，但这要求属性名必须是一个有效的变量名。如果属性名包含特殊字符，就必须用''括起来
console.log(obj.name); // 输出: Alice
console.log(obj['age']); // 输出: 30
// favorite color这个属性名包含空格，不是一个有效的变量，就需要用''括起来。
console.log(obj['favorite color']); // 输出: blue
// 访问不存在的属性不报错，而是返回undefined
console.log(obj.gender); // 输出: undefined

// 对象是动态类型，你可以自由地给一个对象添加或删除属性
obj.gender = 'female'; // 添加一个新属性
console.log(obj.gender); // 输出: female
delete obj.age; // 删除一个属性
console.log(obj.age); // 输出: undefined

// in操作符可以用来判断一个对象是否具有某个属性
// 继承属性也会被in操作符检测到
console.log('name' in obj); // 输出: true
console.log('age' in obj); // 输出: false

// toString方法是所有对象都继承自Object.prototype的一个方法，它返回一个表示对象的字符串。默认情况下，toString方法返回一个类似于[object Object]的字符串，但你可以通过重写toString方法来返回更有意义的字符串。
console.log(obj.toString()); // 输出: [object Object]
obj.toString = function() {
    return `Name: ${this.name}, City: ${this.city}`;
};
console.log(obj.toString()); // 输出: Name: Alice, City: New York

// hasOwnProperty方法是Object.prototype上的一个方法，用来判断一个对象是否具有某个属性（不包括继承的属性）。如果对象具有该属性，返回true；否则返回false。
console.log(obj.hasOwnProperty('name')); // 输出: true
console.log(obj.hasOwnProperty('toString')); // 输出: true