/**
 * 我们用obj.xxx访问一个对象的属性时，
 * JavaScript引擎先在当前对象上查找该属性，
 * 如果没有找到，就到其原型对象上找，
 * 如果还没有找到，就一直上溯到Object.prototype对象，
 * 最后，如果还没有找到，就只能返回undefined。
 */
// 例如，创建一个Array对象
//        null
//          ▲
//          │
// ┌─────────────────┐
// │Object.prototype │
// └─────────────────┘
//          ▲
//          │
// ┌─────────────────┐
// │ Array.prototype │
// └─────────────────┘
//          ▲
//          │
// ┌─────────────────┐
// │       arr       │
// └─────────────────┘
// 函数也是一个对象，它的原型链是：
//         null
//           ▲
//           │
// ┌───────────────────┐
// │ Object.prototype  │
// └───────────────────┘
//           ▲
//           │
// ┌───────────────────┐
// │Function.prototype │
// └───────────────────┘
//           ▲
//           │
// ┌───────────────────┐
// │        foo        │
// └───────────────────┘

// 创建对象
// 除了直接用{ ... }创建一个对象外，JavaScript还可以用一种构造函数的方法来创建对象。
function Person(name) {
    this.name = name;
}
// 在JavaScript中，可以用关键字new来调用这个函数，并返回一个对象
// 如果不写new，这就是一个普通函数，它返回undefined
// 用new Person()创建的对象还从原型上获得了一个constructor属性，它指向函数Person本身
var p1 = new Person('Alice');
var p2 = new Person('Bob');
console.log(p1.name); // Alice
console.log(p2.name); // Bob
console.log(p1.constructor === Person); // true
console.log(p2.constructor === Person); // true

Person.prototype.greet = function() {
    console.log('Hello, ' + this.name);
};
p1.greet(); // Hello, Alice
p2.greet(); // Hello, Bob

// 原型继承（传统方法）    比较复杂不要用
// JavaScript由于采用原型继承，我们无法直接扩展一个Class，因为根本不存在Class这种类型。
// 但是，我们可以通过修改构造函数的prototype属性来实现继承。
// 现在，我们要基于Student扩展出PrimaryStudent，可以先定义出PrimaryStudent：
function Student(props) {
    this.name = props.name || 'Unnamed';
}

Student.prototype.hello = function () {
    alert('Hello, ' + this.name + '!');
}

function PrimaryStudent(props) {
    // 调用Student构造函数，绑定this变量:
    Student.call(this, props);
    this.grade = props.grade || 1;
}

// 空函数F:
function F() {
}

// 把F的原型指向Student.prototype:
F.prototype = Student.prototype;

// 把PrimaryStudent的原型指向一个新的F对象，F对象的原型正好指向Student.prototype:
PrimaryStudent.prototype = new F();

// 把PrimaryStudent原型的构造函数修复为PrimaryStudent:
PrimaryStudent.prototype.constructor = PrimaryStudent;

// 继续在PrimaryStudent原型（就是new F()对象）上定义方法：
PrimaryStudent.prototype.getGrade = function () {
    return this.grade;
};

// 创建xiaoming:
let xiaoming = new PrimaryStudent({
    name: '小明',
    grade: 2
});
xiaoming.name; // '小明'
xiaoming.grade; // 2

// 验证原型:
xiaoming.__proto__ === PrimaryStudent.prototype; // true
xiaoming.__proto__.__proto__ === Student.prototype; // true

// 验证继承关系:
xiaoming instanceof PrimaryStudent; // true
xiaoming instanceof Student; // true

// 新的关键字class从ES6开始正式被引入到JavaScript中
class Student {
    constructor(name) {
        this.name = name;
    }

    hello() {
        alert('Hello, ' + this.name + '!');
    }
}
/**
 * 比较一下就可以发现，
 * class的定义包含了构造函数constructor和定义在原型对象上的函数hello()（注意没有function关键字），
 * 这样就避免了Student.prototype.hello = function () {...}这样分散的代码。
 */
let student = new Student('小明');
student.hello(); // Hello, 小明!

// 继承 直接通过extends来实现
class PrimaryStudent extends Student {
    constructor(name, grade) {
        super(name); // 调用父类的constructor(name)
        this.grade = grade;
    }

    getGrade() {
        return this.grade;
    }
}