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
### 7.30-8.1 
    完成数组的调试

### 8.2-8.3
    完成浮点的移植

### 8.4
    完成基本块合并

### 8.5 
    将DCE, SCCP ，公共子表达式删除适配

### 8.6-8.8
    修完所有测例

### 8.9-8.10
    性能测试跑出分

### 8.5 张子奇：
    循环展开完成

### 8.7 张子奇：
    适配内联数组部分

### 8.8 张子奇：
    完成第二轮优化的连接

### 8.9 张子奇：
    完成第三轮优化的连接





### 参考文档

https://www.cnblogs.com/zhangleo/p/15963442.html

https://pku-minic.github.io/online-doc/#/

https://compiler.educg.net/#/











