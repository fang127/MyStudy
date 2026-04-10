// generator和函数不同的是，generator由function*定义（注意多出的*号），并且，除了return语句，还可以用yield返回多次。
function* helloWorldGenerator() {
  yield 'hello';
  yield 'world';
  return 'ending';
}
var hw = helloWorldGenerator();
console.log(hw.next()); // { value: 'hello', done: false }
for (let word of hw) {
  console.log(word); // hello world
}
// 上面代码中，helloWorldGenerator是一个generator函数，
// 调用这个函数会返回一个遍历器对象hw。
// 第一次调用hw.next()，会返回{ value: 'hello', done: false }，
// 第二次调用hw.next()，会返回{ value: 'world', done: false }，
// 第三次调用hw.next()，会返回{ value: 'ending', done: true }。
// 可以看到，generator函数可以返回多个值（yield语句），
// 而且generator函数的返回值是一个遍历器对象，可以用for...of循环遍历。