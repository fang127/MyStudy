let x = 0;
let i;
for(i = 1; i <= 10000; i++) {
    x += i;
}

console.log(x);

let arr = [1, 2, 3, 4, 5];
for(i = 0; i < arr.length; i++) {
    console.log(arr[i]);
}

// for循环的3个条件都是可以省略的，如果没有退出循环的判断条件，就必须使用break语句退出循环，否则就是死循环
for(;;) {
    if(i > 10000) {
        break;
    }
    x += i;
    i++;
}

// for ... in循环可以用来遍历对象的属性，或者数组的索引
let obj = {
    name: 'Alice',
    age: 30,
    city: 'New York'
};

for(let key in obj) {
    console.log(key + ': ' + obj[key]);
}

// 由于Array也是对象，而它的每个元素的索引被视为对象的属性，因此，for ... in循环可以直接循环出Array的索引：
let arr2 = ['a', 'b', 'c'];
for(let index in arr2) {
    console.log(index + ': ' + arr2[index]);
}

// while循环和do ... while循环的区别在于，while循环先判断条件，再执行循环体；而do ... while循环先执行循环体，再判断条件，因此，do ... while循环至少会执行一次循环体。
let j = 0;
while(j < 5) {
    console.log(j);
    j++;
}

let k = 0;
do {
    console.log(k);
    k++;
} while(k < 5);