// Date
let data = new Date()
console.log(data) // 当前时间
console.log(data.getFullYear()) // 当前年份
console.log(data.getMonth()) // 当前月份（0-11）
console.log(data.getDate()) // 当前日期
console.log(data.getHours()) // 当前小时
console.log(data.getMinutes()) // 当前分钟
console.log(data.getSeconds()) // 当前秒数
console.log(data.getTime()) // 当前时间戳（毫秒数）
// 当前时间是浏览器从本机操作系统获取的时间，所以不一定准确，因为用户可以把当前时间设定为任何值。

let data1 = new Date(2024, 5, 1, 22, 1, 12) // 2024年6月1日（注意：月份从0开始）
console.log(data1) // 2024-06-01T14:01:12.000Z

let d = new Date('2024-06-01T22:01:12')
// Date对象的toLocaleString方法会根据当前环境的语言和时区设置来格式化日期和时间字符串。
console.log(d.toLocaleString()) // 2024/6/1 22:01:12
// toUTCString方法会将Date对象转换为UTC时间字符串，格式为YYYY-MM-DDTHH:mm:ss.sssZ，其中Z表示UTC时区。
console.log(d.toUTCString()) // 2024-06-01T14:01:12.000Z

// RegExp
let reg = /abc/ // 正则表达式字面量
let reg1 = new RegExp('abc') // 正则表达式构造函数
console.log(reg.test('abcdef')) // true
console.log(reg.test('defabc')) // true
console.log(reg.test('def')) // false
console.log(reg.test('abc')) // true
console.log(reg.test('ab')) // false

// JSON
// 在JSON中，一共就这么几种数据类型：
// number：和JavaScript的number完全一致；
// boolean：就是JavaScript的true或false；
// string：就是JavaScript的string；
// null：就是JavaScript的null；
// array：就是JavaScript的Array表示方式——[]；
// object：就是JavaScript的{ ... }表示方式。
// JSON的字符串规定必须用双引号""，Object的键也必须用双引号""

// 在JavaScript中，我们可以直接使用JSON，因为JavaScript内置了JSON的解析。
// 把任何JavaScript对象变成JSON，就是把这个对象序列化成一个JSON格式的字符串，这样才能够通过网络传递给其他计算机。
let xiaoming = {
    name: '小明',
    age: 14,
    gender: true,
    height: 1.65,
    grade: null,
    'middle-school': '\"W3C\" Middle School',
    skills: ['JavaScript', 'Java', 'Python', 'Lisp']
};

let s = JSON.stringify(xiaoming);
console.log(s);

// 可以加上参数来控制JSON.stringify的行为：
// 第二个参数是一个函数，用来控制哪些属性会被序列化，或者直接传入一个数组，指定哪些属性会被序列化；
// 第三个参数是一个字符串或者数字，用来控制输出的格式，通常用来美化输出。
let s1 = JSON.stringify(xiaoming, null, '    ');
console.log(s1);

// 还可以传入一个函数，这样对象的每个键值对都会被函数先处理：
function converter(key, value) {
    if (typeof value === 'string') {
        return value.toUpperCase();
    }
    return value;
}

let s2 = JSON.stringify(xiaoming, converter, '    ');
console.log(s2);

// 如果我们还想要精确控制如何序列化，可以定义一个toJSON()的方法，直接返回JSON应该序列化的数据
let person = {
    name: '小明',
    age: 14,
    gender: true,
    height: 1.65,
    grade: null,
    'middle-school': '\"W3C\" Middle School',
    skills: ['JavaScript', 'Java', 'Python', 'Lisp'],
    toJSON: function () {
        return { // 只输出name和age，并且改变了key：
            'Name': this.name,
            'Age': this.age
        };
    }
};

JSON.stringify(person); // '{"Name":"小明","Age":14}'

// 反序列化
// JSON.parse方法可以把JSON字符串解析成JavaScript对象。
let obj = JSON.parse(s);
console.log(obj);
JSON.parse('[1,2,3]'); // [1, 2, 3]
JSON.parse('{"name":"小明","age":14}'); // { name: '小明', age: 14 }
// JSON.parse()还可以接收一个函数，用来转换解析出的属性：
function reviver(key, value) {
    if (key === 'name') {
        return value.toUpperCase();
    }
    return value;
}

let obj1 = JSON.parse(s, reviver);
console.log(obj1);