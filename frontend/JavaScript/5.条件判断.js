let age = 20;
if (age >= 18) {
    console.log("你已经成年了，可以进入夜店了");
} else {
    console.log("你还未成年，不能进入夜店");
}

// 只有一条语句时，可以省略大括号
if (age >= 18)
    console.log("你已经成年了，可以进入夜店了");
else
    console.log("你还未成年，不能进入夜店");

// if...else if...else 结构
let score = 85;
if (score >= 90) {
    console.log("优秀");
} else if (score >= 80) {
    console.log("良好");
} else {
    console.log("需要努力");
}