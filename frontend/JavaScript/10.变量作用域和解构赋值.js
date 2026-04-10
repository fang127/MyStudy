// 如果一个变量在函数体内部申明，则该变量的作用域为整个函数体
// 如果两个不同的函数各自申明了同一个变量，那么该变量只在各自的函数体内起作用
function foo() {
    var a = 1;
    console.log(a); // 1
    if (true) {
        var b = 2;
        console.log(b); // 2
    }
    console.log(b); // 2
}
foo();
// console.log(a); // Uncaught ReferenceError: a is not defined
// console.log(b); // Uncaught ReferenceError: b is not defined

function bar() {
    var a = 3;
    console.log(a); // 3
}
bar();
// console.log(a); // Uncaught ReferenceError: a is not defined

// 由于JavaScript的函数可以嵌套，此时，内部函数可以访问外部函数定义的变量，反过来则不行
function foo1() {
    var a = 1;
    function bar() {
        console.log(a); // 1
    }
    bar();
}
foo1();
// console.log(a); // Uncaught ReferenceError: a is not defined
// bar(); // Uncaught ReferenceError: bar is not defined

// 内部函数变量可以覆盖外部函数变量
function foo2() {
    var a = 1;
    function bar() {
        var a = 2;
        console.log(a); // 2
    }
    bar();
    console.log(a); // 1
}
foo2();

// 变量提升：函数体内的变量和函数申明会被提升到函数体的顶部
// JavaScript的函数定义有个特点，它会先扫描整个函数体的语句，把所有用var申明的变量“提升”到函数顶部
function foo3() {
    console.log(a); // undefined
    var a = 1;
    console.log(a); // 1
}
foo3();
// 上面代码中，变量a在申明之前就被访问了，结果是undefined，而不是ReferenceError。这是因为JavaScript引擎在执行函数体之前，会先扫描整个函数体，把所有用var申明的变量提升到函数顶部，但不会提升变量的赋值操作，所以变量a在提升后是undefined。

/**
 * 由于JavaScript的这一怪异的“特性”，
 * 我们在函数内部定义变量时，
 * 请严格遵守“在函数内部首先申明所有变量”这一规则。
 * 最常见的做法是用一个var申明函数内部用到的所有变量
 */
function foo4() {
    var
        x = 1, // x初始化为1
        y = x + 1, // y初始化为2
        z, i; // z和i为undefined
    console.log(x, y, z, i); // 1 2 undefined undefined
}

// 全局作用域
/**
 * 不在任何函数内定义的变量就具有全局作用域。
 * 实际上，JavaScript默认有一个全局对象window，
 * 全局作用域的变量实际上被绑定到window的一个属性
 * window只在浏览器环境中存在，在Node.js环境中，全局对象是global，而在Web Worker环境中，全局对象是self
 */

var course = 'Learn JavaScript';
console.log(course); // Learn JavaScript
// console.log(global.course); // Learn JavaScript
// console.log(window.course); // Learn JavaScript 
// // 新版本的JavaScript引入了let和const两种新的变量申明方式，这两种方式申明的变量不具有全局作用域，因此不会被绑定到window对象上
// let course2 = 'Learn JavaScript';
// console.log(course2); // Learn JavaScript
// console.log(window.course2); // undefined
// 你可能猜到了，由于函数定义有两种方式，以变量方式var foo = function () {}定义的函数实际上也是一个全局变量，因此，顶层函数的定义也被视为一个全局变量，并绑定到window对象
var foo5 = function () {
    console.log('foo');
};
foo5(); // foo
// window.foo5(); // foo

// // 我们每次直接调用的alert()函数其实也是window的一个变量
// alert('Hello'); // Hello
// window.alert('Hello'); // Hello

// 这说明JavaScript实际上只有一个全局作用域。任何变量（函数也视为变量），如果没有在当前函数作用域中找到，就会继续往上查找，最后如果在全局作用域中也没有找到，则报ReferenceError错误。


// 名字空间
/**
 * 全局变量会绑定到window上，
 * 不同的JavaScript文件如果使用了相同的全局变量，
 * 或者定义了相同名字的顶层函数，都会造成命名冲突
 * 减少冲突的一个方法是把自己的所有变量和函数全部绑定到一个全局变量中。
 */
let MYAPP = {};
MYAPP.name = 'My Application';
MYAPP.version = '1.0.0';
MYAPP.foo = function () {
    console.log('foo');
};
MYAPP.bar = function () {
    console.log('bar');
};
console.log(MYAPP.name); // My Application
console.log(MYAPP.version); // 1.0.0
MYAPP.foo(); // foo
MYAPP.bar(); // bar
// 上面代码中，我们把所有的变量和函数都绑定到MYAPP这个全局变量上，这样就避免了命名冲突的问题。

// 局部作用域
/**
 * 由于JavaScript的变量作用域实际上是函数内部，
 * 我们在for循环等语句块中是无法定义具有局部作用域的变量的
 */
for (var i = 0; i < 10; i++) {
    console.log(i); // 0 1 2 3 4 5 6 7 8 9
}
console.log(i); // 10
// 上面代码中，变量i在for循环内部被定义，但它的作用域是整个函数体（如果没有函数体，则是全局作用域），因此，在for循环外部也可以访问到变量i，并且它的值是10，而不是ReferenceError错误。

// 为了解决块级作用域，ES6引入了新的关键字let，用let替代var可以申明一个块级作用域的变量
for (let j = 0; j < 10; j++) {
    console.log(j); // 0 1 2 3 4 5 6 7 8 9
}
// console.log(j); // Uncaught ReferenceError: j is not defined
// 上面代码中，变量j是用let申明的，因此它的作用域是for循环内部的块级作用域，在for循环外部访问变量j会报ReferenceError错误。

// ES6标准引入了新的关键字const来定义常量，const与let都具有块级作用域
const PI = 3.1415926;
console.log(PI); // 3.1415926
// PI = 3.14; // Uncaught TypeError: Assignment to constant variable.
// 上面代码中，PI是一个常量，不能被重新赋值，如果尝试修改PI的值，会报TypeError错误。

// 解构赋值,可以同时对一组变量进行赋值。
let arr = [3, 4, 5, 6];
let [a, b, ...rest] = arr;
console.log(a); // 3
console.log(b); // 4
console.log(rest); // [5, 6]
// 上面代码中，使用解构赋值的语法，把数组arr中的前两个元素分别赋值给变量a和b，剩余的元素通过...rest语法赋值给变量rest，这样就可以同时对多个变量进行赋值，并且可以把剩余的元素收集到一个数组中。
let [x, [y, z]] = ['hello', ['JavaScript', 'ES6']];
console.log(x); // hello
console.log(y); // JavaScript
console.log(z);
// 可以忽略某些元素
let [, , newZ] = ['hello', 'JavaScript', 'ES6']; // 忽略前两个元素，只对z赋值第三个元素
console.log(newZ); // ES6

// 如果需要从一个对象中取出若干属性，也可以使用解构赋值，便于快速获取对象的指定属性
let person = {
    name: 'Alice',
    age: 30,
    city: 'New York'
};
let { name, age } = person;
console.log(`name = ${name}, age = ${age}`); // name = Alice, age = 30
// 上面代码中，使用解构赋值的语法，把对象person中的name和age属性分别赋值给变量name和age，这样就可以快速获取对象的指定属性，并且可以省略掉不需要的属性。

// 对一个对象进行解构赋值时，同样可以直接对嵌套的对象属性进行赋值，只要保证对应的层次是一致的
let person01 = {
    name: '小明',
    age: 20,
    gender: 'male',
    passport: 'G-12345678',
    school: 'No.4 middle school',
    address: {
        city: 'Beijing',
        street: 'No.1 Road',
        zipcode: '100001'
    }
};
let {name01, address: {city, zip}} = person;
console.log(name01); // undefined
console.log(city); // Beijing
console.log(zip); // undefined
// 上面代码中，使用解构赋值的语法，把对象person中的name属性赋值给变量name01，把address属性中的city和zipcode属性分别赋值给变量city和zip，但是由于person对象中没有name01和zip属性，所以它们的值是undefined，而不是ReferenceError错误。

/**
 * 使用解构赋值对对象属性进行赋值时，
 * 如果对应的属性不存在，变量将被赋值为undefined，
 * 这和引用一个不存在的属性获得undefined是一致的。
 * 如果要使用的变量名和属性名不一致，可以用下面的语法获取：
 */
let person02 = {
    name: '小明',
    age: 20,
    gender: 'male',
    passport: 'G-12345678',
    school: 'No.4 middle school'
};

// 把passport属性赋值给变量id:
let {name02, passport:id} = person;
console.log(id); // G-12345678
// 上面代码中，使用解构赋值的语法，把对象person中的passport属性赋值给变量id，这样就可以使用不同的变量名来获取对象的属性值。

// 也可以使用默认值的语法，如果对应的属性不存在，变量将被赋值为默认值
let person03 = {
    name: '小明',
    age: 20,
    gender: 'male',
    passport: 'G-12345678',
    school: 'No.4 middle school'
};

// 把不存在的属性country赋值为默认值'China':
let {name03, country = 'China'} = person;
console.log(country); // China
// 上面代码中，使用解构赋值的语法，把对象person中的country属性赋值给变量country，并且指定了默认值'China'，由于person对象中没有country属性，所以变量country的值是'China'，而不是undefined。


// 有些时候，如果变量已经被声明了，再次赋值的时候，正确的写法也会报语法错误：
let m = 1, n = 2;
// {m, n} = {m: n, n: m}; // Uncaught SyntaxError: Unexpected token '='
// 上面代码中，变量m和n已经被声明了，如果直接使用解构赋值的语法来交换它们的值，会报SyntaxError错误。正确的写法是把解构赋值的语句放在一个括号内，这样就不会被当作一个代码块来解析：
({m, n} = {m: n, n: m});
console.log(m); // 2
console.log(n); // 1
// 上面代码中，使用解构赋值的语法来交换变量m和n的值，把解构赋值的语句放在一个括号内，这样就不会被当作一个代码块来解析，最终变量m的值是2，变量n的值是1。