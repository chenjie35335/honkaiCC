# honkaiCC

这是第五届华为毕昇杯优胜奖作品，通过135个功能测例，现提供所有版本

## 分支说明

main分支和backnew分支是后端使用图着色的版本，比赛当中没有使用；比赛使用的版本是backold,其中backold3是最终版本。

## honkaiCC使用

* 编译命令

```
    build/compiler -S -o <目标文件名> <源文件名> //生成汇编
    build/compiler -koopa -o <目标文件名> <源文件名> //生成中间代码
```

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

## 参考文档

https://www.cnblogs.com/zhangleo/p/15963442.html

https://pku-minic.github.io/online-doc/#/

https://compiler.educg.net/#/

## 开发者联系方式：

如要交流，可发送邮箱至```1042653777@qq.com```。同时仓库可能不定期有新commit，可关注。