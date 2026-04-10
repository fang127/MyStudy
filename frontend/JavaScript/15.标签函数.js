function update(strings, ...values) {
    console.log(strings);
    console.log(values);
}

let name = '张三';
let age = 20;
update`我的名字是${name},年龄是${age}`;