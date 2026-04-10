// 新增数据函数
function addRowData() {
    let table = document.getElementById("table");

    // 获取表格的行数
    var len = table.rows.length;
    // 插入新行
    let newRow = table.insertRow(len);
    
    // 插入新单元格
    let cell1 = newRow.insertCell(0);
    let cell2 = newRow.insertCell(1);
    let cell3 = newRow.insertCell(2);
    // 最后一个是操作单元格
    let cell4 = newRow.insertCell(3);
    // cell4下两个按钮：修改和删除
    cell4.innerHTML = "<button onclick='modifyRowData(this)'>修改</button> <button onclick='deleteRowData(this)'>删除</button>";

    // 设置新单元格的内容
    cell1.innerHTML = "新数据1";
    cell2.innerHTML = "新数据2";
    cell3.innerHTML = "新数据3";
}

// 删除数据函数
function deleteRowData(button) {
    let row = button.parentNode.parentNode; // 获取按钮所在行
    row.parentNode.removeChild(row); // 删除该行
}


// 修改数据函数
function modifyRowData(button) {
    let row = button.parentNode.parentNode; // 获取按钮所在行
    let name = row.cells[0];
    let age = row.cells[1];
    let gender = row.cells[2];

    let inputName = prompt("请输入新的名字", name.innerHTML);
    let inputAge = prompt("请输入新的年龄", age.innerHTML);
    let inputGender = prompt("请输入新的性别", gender.innerHTML);

    if (inputName !== null) {
        row.cells[0].innerHTML = inputName; // 更新名字
    }
    if (inputAge !== null) {
        row.cells[1].innerHTML = inputAge; // 更新年龄
    }
    if (inputGender !== null) {
        row.cells[2].innerHTML = inputGender; // 更新性别
    }
}