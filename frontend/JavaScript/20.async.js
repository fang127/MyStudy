// async是基于generator的语法糖，async函数返回一个Promise对象，函数体内可以使用await关键字等待Promise对象的结果。

// async函数的基本用法
async function example() {
  return 'Hello, World!';
}

example().then(result => console.log(result)); // 输出: Hello, World!

// 使用await等待Promise对象的结果
async function fetchData() {
  const response = await fetch('https://api.example.com/data');
  const data = await response.json();
  return data;
}

fetchData().then(data => console.log(data)); // 输出: 从API获取的数据

// 错误处理
async function errorExample() {
  try {
    const response = await fetch('https://api.example.com/invalid');
    const data = await response.json();
    return data;
  } catch (error) {
    console.error('Error fetching data:', error);
  }
}

errorExample(); // 输出: Error fetching data: [错误信息]

// async函数的并行执行
async function parallelExample() {
  const promise1 = fetch('https://api.example.com/data1').then(res => res.json());
  const promise2 = fetch('https://api.example.com/data2').then(res => res.json());

  const [data1, data2] = await Promise.all([promise1, promise2]);
  return { data1, data2 };
}

parallelExample().then(result => console.log(result)); // 输出: { data1: ..., data2: ... }