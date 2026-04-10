// 在一个对象中绑定函数，称为这个对象的方法
var obj = {
    name: 'obj',
    birth: 1990,
    sayName: function() {
        console.log(this.name);
    },
    age: function() {
        // this指向obj对象
        var age = new Date().getFullYear() - this.birth;
        console.log(age);
    }
}
obj.sayName(); // obj
obj.age(); // 34

// 这么写不对
let fn = obj.age;
fn(); // NaN
// 因为fn函数的this指向window对象，window对象没有birth属性，所以计算结果是NaN

// 解决方法一：使用bind方法改变this指向
let fn1 = obj.age.bind(obj);
fn1(); // 34

// 解决方法二：使用call方法改变this指向
obj.age.call(obj); // 34

// 解决方法三：使用apply方法改变this指向
obj.age.apply(obj); // 34

// 解决方法四：使用箭头函数，箭头函数没有自己的this，this指向外层作用域
var obj2 = {
    name: 'obj2',
    birth: 1990,
    age: function() {
        var that = this; // 保存this指向
        // 如果不捕获this，setTimeout函数中的this指向window对象，window对象没有birth属性，所以计算结果是NaN
        setTimeout(function() {
            var age = new Date().getFullYear() - that.birth;
            console.log(age);
        }, 1000);
    }
}
obj2.age(); // 34

