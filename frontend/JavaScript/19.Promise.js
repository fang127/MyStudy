// Promise是ES6引入的一种异步编程解决方案，
// Promise对象代表一个异步操作的最终完成（或失败）及其结果值。
// Promise提供了一种更优雅的方式来处理异步操作，避免了回调地狱的问题。

// Promise的状态有三种：
// 1. Pending（待定）：初始状态，既不是成功，也不是失败。
// 2. Fulfilled（已完成）：操作成功完成，Promise对象进入这个状态，并且有一个值。
// 3. Rejected（已拒绝）：操作失败，Promise对象进入这个状态，并且有一个原因。

// 创建一个Promise对象
const myPromise = new Promise((resolve, reject) => {
  // 模拟一个异步操作，例如一个网络请求
  setTimeout(() => {
    const success = true; // 模拟成功或失败的结果
    if (success) {
      resolve('操作成功！'); // 成功时调用resolve，传递结果值
    } else {
      reject('操作失败！'); // 失败时调用reject，传递错误原因
    }
  }, 1000);
});

// 使用Promise对象
myPromise
  .then(result => {
    console.log(result); // 输出：操作成功！
  })
  .catch(error => {
    console.error(error); // 如果发生错误，输出：操作失败！
  });

// Promise还支持链式调用，可以在then方法中返回一个新的Promise对象，
// 这样可以实现多个异步操作的顺序执行。
myPromise
  .then(result => {
    console.log(result); // 输出：操作成功！
    return '下一步操作'; // 返回一个新的值，传递给下一个then
  })
  .then(nextResult => {
    console.log(nextResult); // 输出：下一步操作
  })
  .catch(error => {
    console.error(error); // 如果发生错误，输出：操作失败！
  });

// Promise还提供了静态方法，例如Promise.all()和Promise.race()，
// 用于处理多个Promise对象的情况。
const promise1 = Promise.resolve('第一个Promise');
const promise2 = Promise.resolve('第二个Promise');

Promise.all([promise1, promise2])
  .then(results => {
    console.log(results); // 输出：['第一个Promise', '第二个Promise']
  })
  .catch(error => {
    console.error(error); // 如果有任何一个Promise失败，输出错误信息
  });

Promise.race([promise1, promise2])
  .then(result => {
    console.log(result); // 输出：第一个Promise（因为它先完成）
  })
  .catch(error => {
    console.error(error); // 如果有任何一个Promise失败，输出错误信息
  });