# HTML

## 1. HTML 是什么
- HTML（HyperText Markup Language）是网页的骨架，用标签来描述内容结构。
- HTML 本身不负责好看（样式）和交互（行为）。
- 常见配合：
	- HTML：结构
	- CSS：样式
	- JavaScript：交互

## 2. 基本页面结构
```html
<!DOCTYPE html>
<html lang="zh-CN">
	<head>
		<meta charset="UTF-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title>我的第一个网页</title>
	</head>
	<body>
		<h1>Hello HTML</h1>
		<p>这是一个段落。</p>
	</body>
</html>
```

## 3. 常见标签
- 标题：`h1` ~ `h6`
- 段落：`p`
- 超链接：`a`
- 图片：`img`
- 列表：`ul`、`ol`、`li`
- 区块：`div`（块级）、`span`（行内）
- 表单：`form`、`input`、`button`、`label`

## 4. 常见属性
- `id`：唯一标识（一个页面里尽量不要重复）
- `class`：分类标识（可重复，主要给 CSS/JS 使用）
- `href`：链接地址
- `src`：资源地址（如图片、脚本）
- `alt`：图片加载失败时的替代文本

## 5. 语义化标签（推荐）
- `header`、`nav`、`main`、`section`、`article`、`aside`、`footer`
- 好处：结构更清晰，SEO 更友好，可读性更高。

## 6. HTML 学习建议
- 标签先理解“用途”，再背写法。
- 尽量写完整结构，不要只写碎片代码。
- 多在浏览器开发者工具里看 DOM 结构。

---

# CSS

## 1. CSS 是什么
- CSS（Cascading Style Sheets）用于控制网页样式：颜色、大小、布局、动画等。
- 目标：让网页更美观、更易读、更适配不同设备。

## 2. CSS 基本语法
```css
选择器 {
	属性1: 值1;
	属性2: 值2;
	属性3: 值3;
}
```

- 选择器：指定要设置样式的 HTML 元素，可以是标签、类、ID 等。
- 选择器的声明中可以有多个属性，每个属性以分号结尾。
- 属性和值以键值对的形式出现，属性和值之间用冒号分隔。

示例：

一个简单的 CSS 规则，设置所有 `h1` 标签的颜色为蓝色，字体大小为 32 像素：
```css
h1 {
	color: #1d4ed8;
	font-size: 32px;
}
```

## 3. 三种引入方式
1. 行内样式（不推荐大量使用）
```html
<p style="color: red;">文本</p>
```
2. 内部样式
```html
<style>
	p { color: red; }
</style>
```
3. 外部样式（推荐）
```html
<link rel="stylesheet" href="style.css" />
```

> - 三种方式优先级：行内 > 内部 > 外部。
> - 优先级高的样式会覆盖优先级低的样式。

## 4. 常见选择器

示例见:[CSS 选择器](HTML\6.CSS选择器.html)

- 元素选择器：`p`
- 类选择器：`.card`
- ID 选择器：`#title`
- 通用选择器：`*`
- 子元素选择器：`.nav > a`
- 后代选择器：`.nav a`
- 并集选择器：`h1, h2, h3`
- 伪类：`a:hover`

## 5. 盒模型（重点）

每个元素都被看作一个矩形盒子，包含以下部分：

- 内容区：`content`
- 内边距：`padding`
- 边框：`border`
- 外边距：`margin`

常用设置：
```css
* {
	box-sizing: border-box;
}
```

## 6. 布局入门
- 标准流布局：元素按顺序排列，块级元素换行，行内元素不换行。 `display: block | inline | inline-block`

- 浮动布局：`float` 属性，用于创建浮动框，将其移动到一边，直到遇到边界或另一个浮动元素
  - 常用于文字环绕图片等场景，但不推荐用于复杂布局。
  - 特性：
	- 浮动元素脱离文档流，不占据空间。
	- 一行显示，顶部对齐。
	- 具备行内块元素特性，可以设置宽高。
  
- 定位布局：`position` 属性，常用于需要精确控制位置的元素
	- 相对定位（`relative`）：相对于自身位置偏移。
	- 绝对定位（`absolute`）：相对于最近的定位祖先元素偏移，不占文档流空间。
	- 固定定位（`fixed`）：相对于浏览器窗口偏移。不占据文档流空间，固定在屏幕上的位置，不随滚动而移动。
  
- 自适应布局：`display: flex` 和 `display: grid`
  - FlexBox 布局（常用）：一维布局
  - Grid 布局：二维布局

Flex 示例：
```css
.container {
	display: flex;
	justify-content: space-between;
	align-items: center;
}
```

## 7. 常用单位
- 绝对单位：`px`
- 相对单位：`%`、`em`、`rem`、`vw`、`vh`

## 8. 优先级（简单理解）
- `!important` > 行内样式 > `#id` > `.class` > 标签
- 同优先级下：后写的会覆盖前写的。

## 9. CSS 学习建议
- 先学盒模型和定位，再学 Flex/Grid。
- 写样式时先排版（布局），后美化（颜色、阴影）。
- 养成给公共样式加类名的习惯。

---

# JavaScript

## 1. JavaScript 是什么
- JavaScript 是让网页“动起来”的编程语言。
- 常见用途：
	- 响应点击、输入等用户操作
	- 操作页面内容（DOM）
	- 与服务器交互（获取/提交数据）

## 2. 基本语法示例
```js
let name = "Tom";
const age = 18;
let isStudent = true;

console.log(name, age, isStudent);
```

## 3. 常见数据类型
- 基本类型：`string`、`number`、`boolean`、`undefined`、`null`、`symbol`、`bigint`
- 引用类型：`object`（数组、函数等本质都与对象相关）

## 4. 条件与循环
```js
let score = 85;

if (score >= 60) {
	console.log("及格");
} else {
	console.log("不及格");
}

for (let i = 0; i < 3; i++) {
	console.log(i);
}
```

## 5. 函数
```js
function add(a, b) {
	return a + b;
}

const result = add(2, 3);
console.log(result);
```

## 6. DOM 操作
```html
<button id="btn">点我</button>
<p id="text">原始文本</p>
```

```js
const btn = document.querySelector("#btn");
const text = document.querySelector("#text");

btn.addEventListener("click", function () {
	text.textContent = "你点击了按钮";
});
```

## 7. 常见数组方法
- `push()`：末尾添加
- `pop()`：末尾删除
- `map()`：映射新数组
- `filter()`：筛选
- `find()`：查找第一个符合条件的元素

## 8. 异步入门
- JS 是单线程，但可通过异步处理耗时任务。
- 常见写法：`Promise`、`async/await`

```js
async function getData() {
	const res = await fetch("https://jsonplaceholder.typicode.com/posts/1");
	const data = await res.json();
	console.log(data);
}
```

## 9. JavaScript 学习建议
- 先打牢变量、条件、循环、函数。
- 再学 DOM 和事件。
- 最后进阶异步、模块化、工程化（如 Vite、Webpack）。

---

# 前端学习路线（新手版）
1. HTML：标签 + 语义化 + 表单
2. CSS：选择器 + 盒模型 + Flex + 响应式
3. JavaScript：语法基础 + DOM + 事件 + 异步
4. 综合练习：做静态页面 -> 做交互页面 -> 对接接口

# 常用开发习惯
- 文件命名统一使用小写和中划线，例如：`user-card.css`
- 每写完一个功能，先在浏览器中手动测试。
- 遇到 bug 时先看控制台报错，再逐步定位。
- 多做小项目，比只看视频进步更快。