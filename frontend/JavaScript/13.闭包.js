/**
 * 高阶函数除了可以接受函数作为参数外，还可以把函数作为结果值返回。
 */

// 下面是一个返回函数的例子：
function lazy_sum(arr) {
    return function() {
        return arr.reduce(function(x, y) {
            return x + y;
        });
    };
}

// f是一个函数，调用f()时才真正计算求和的结果：
var f = lazy_sum([1, 2, 3, 4, 5]);
console.log(f()); // 15

// 每次调用lazy_sum()都会返回一个新的函数，即使传入相同的参数：
console.log(lazy_sum([1, 2, 3, 4, 5]) === lazy_sum([1, 2, 3, 4, 5])); // false

// 闭包
// 返回的函数在其定义内部引用了变量arr，
// 因此，返回的函数和变量arr形成了一个闭包（Closure）。
// 闭包是一个函数和其相关的引用环境组合而成的实体。
// 句话说，闭包就是一个函数和与其相关的变量绑定在一起的对象。
// 通过闭包，函数可以访问外部的变量，即使外部函数已经返回了。
// 闭包的数值会在外界被改变，但闭包内部的函数仍然可以访问到这个变量的最新值。 
function count() {
    var arr = [];
    for (var i = 1; i <= 3; i++) {
        arr.push(function() {
            return i * i;
        });
    }
    return arr;
}

var results = count();
let [f1, f2, f3] = results;
console.log(f1()); // 16
console.log(f2()); // 16
console.log(f3()); // 16
// 上面代码中，函数count()返回了一个函数列表，
// 但是每个函数都引用了变量i，而变量i在循环结束时已经变成了4，
// 因此调用f1()、f2()和f3()时，都会返回16。

// 如果要让每个函数都绑定当前i的值，可以再创建一个函数，
// 让这个函数绑定i的当前值，并返回一个匿名函数：
function count01() {
    let arr = [];
    for (var i=1; i<=3; i++) {
        arr.push((function (n) {
            return function () {
                return n * n;
            }
        })(i)); // 创建一个匿名函数并立刻执行，把i传入这个匿名函数，匿名函数返回一个函数，这个函数就绑定了当前i的值
    }
    return arr;
}

let [f11, f22, f33] = count01();
console.log(f11()); // 1
console.log(f22());
console.log(f33()); // 9

// 另一个方法是把循环变量i用let定义在for循环体中，let作用域决定了在每次循环时都会绑定新的i
function count02() {
    let arr = [];
    for (let i=1; i<=3; i++) {
        arr.push(function () {
            return i * i;
        });
    }
    return arr;
}

let [f111, f222, f333] = count02();
console.log(f111()); // 1
console.log(f222());
console.log(f333()); // 9