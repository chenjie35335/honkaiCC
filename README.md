# SLcc
This is a compiler for Huawei Bisheng Cup.

# SLcc使用

* 打开docker
```shell
    make docker
```
* 清空编译结果
```shell
    make clean
```
* 自动测试
```shell
    make autotest
```
* 手动测试
```shell
    make TARGET=-riscv/-koopa test
```
* 编译koopa
```shell
    make koopa-test
```
* 编译riscv
```shell
    make riscv-test
```

## 本人计划
### 3月 
#### 3日-15日
    lxh: 完成lv9
    csj：重建后端，实现lv5的后端转换
#### 16日-31日
    lxh：完成浮点的前端部分
    csj：完成lv8的后端适配

### 4月 编译原理
    确定是否比赛后，完成如下任务：
#### 1日-15日
    lxh,csj： 完成lv9和浮点的后端

#### 16日-31日
    lxh,csj: 完成对于AST的优化
