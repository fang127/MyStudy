let fn = x => x * 2
console.log(fn(5)) // 10

/**
 * 箭头函数相当于匿名函数，并且简化了函数定义。
 * 箭头函数有两种格式，一种像上面的，只包含一个表达式，连{ ... }和return都省略掉了。
 * 还有一种可以包含多条语句，这时候就不能省略{ ... }和return
 */
let fn2 = (x, y) => {
  let sum = x + y
  return sum * 2
}
console.log(fn2(5, 10)) // 30

// 如果要返回一个对象，就要注意，如果是单表达式，这么写的话会报错：
// let fn3 = x => { name: '张三' } // SyntaxError: Unexpected token ':'
// 这是因为箭头函数会把{ ... }当成函数体的开始和结束，所以就会报错。
// 解决办法是把对象字面量包裹在圆括号中，这样就不会被当成函数体了：
let fn3 = x => ({ name: '张三' })
console.log(fn3()) // { name: '张三' }

// 箭头函数还有一个特点，就是它没有自己的this，this的值是由外部环境决定的。
let obj = {
  name: '李四',
  fn: function() {
    console.log(this.name) // 李四
    let fn2 = () => {
      console.log(this.name) // 李四
    }
    fn2()
  }
}
obj.fn()
// 上面的代码中，fn2是一个箭头函数，它没有自己的this，所以它的this就是外部环境的this，也就是obj对象，所以输出李四。