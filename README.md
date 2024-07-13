# SLcc

This is a compiler for Huawei Bisheng Cup.

## SLcc使用

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

* -O1优化编译

```shell
    build/compiler -riscv -o *.S  *.c -O1
```

* ast显示

```shell
    //文本形式
    build/compiler -astT -o  *.ast *.c 
    //图像形式
    build/compiler -astG -o  *.dot *.c
```

* dot语言转化成图片

```shell
    dot -Tjpg *.dot -o *.jpg
```

* 功能测试

```shell
    make docker-func
    autotest -riscv -s functional/hidden-functional /root/compiler  || make autotest
```

## 本人计划
### 后端规划

### 7-3 
    完成store{}的拆分
### 7-4
    完成后端的适配（调试）
### 7-5
    协助wjh完成函数后端
### 7-6
    协助wjh完成函数后端
### 7-7
    协助wjh完成函数后端
### 7-8 ～ 7-13
    协助wjh完成数组后端
### 7-14 ～ 7-20
    协助wjh完成浮点后端
### 7-20 ～ 7-27
    csj wjh完成后端的初步优化（指令指派）
### 7-28 ～ 8-1
    csj wjh完成部分指令集并行处理
### 8-2 ～ 8-10
    完全调试

### 中端规划

### 7-8 ～ 7-13
    前半： 完成并调试浮点
    9~10 常数传播的数组浮点鲁棒性
    11～12 死代码消除的数组浮点鲁棒性
    zzq: 不变量外提

### 7-14 ～ 7-20
    前半： 完成并调试mem2reg
    后半： 调试条件常数传播
    zzq：  归纳变量

### 7-21 ～ 7-27
    前半： 优化浮点
    zzq： 尾递归消除

### 7-28 ～ 8-10
    中端集成测试，修改bug,最终目标：通过所有测例














### 参考文档

https://www.cnblogs.com/zhangleo/p/15963442.html

https://pku-minic.github.io/online-doc/#/

https://compiler.educg.net/#/











