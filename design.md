上次设计的数据结构的耦合性太强，这次重新设计

对于RawValue和Memory来说，基本上可以确定为11对应关系
但是对于register和RawValue来说，不一定
随着遍历的进行会进行存入和读取

由于某个时刻Value和Register是对应的，所以
初始时刻寄存器未满时，来一个分配一个，当寄存器已满时，根据使用的频率换出

使用的频率应该还是使用hash表，只是访问一次变化一次

综上，对于register部分需要做到
两个unordered_map

1个用来存<寄存器,原始值>，不断修改原始值内容
1个用来存<原始值，使用次数> ，不断修改次数

这个交给backend中的StackManager 和 RegisterManager就行

一个C语言的语法常识：
尽量不要使用 const *

这里使用了双层的const *
第一层表示这个指针不能修改原先的内存的值，只能读取
第二层表示这个指针函数调用之后不能修改
这时取&就不会报错了

对于这个const的问题，文档这里做出以下解释：
对于使用变量赋值常量这种事情，我的想法就是说要

0 a
4 sum
8 b
12 a
16 b

现在比较难办的事情是如何切换的问题，现在给出方案
1、 我们知道在FuncDef时建立一个RawFunction没毛病，但是何时建立
RawBasicBlock是个问题，如果对应到AST,如果放到multiBlock那里当前访问的BasicBlock,可能会有更好的效果，根据访问到的语句，调整语句块的;但是如何开始调整语句块呢？
（1）首先在if-then-else那个位置，br指令之后，设定当前所在的basicblock结束
（2）现在对于替换RawBasicBlock的算法有以下问题
如果是返回multiBlock再判断就会存在一个问题在于跳跃的问题
这个就涉及到我的结构的问题，RawSlice这个是大问题，无法直接访问上层的RawBasicBlock,导致这里我们没有办法直接标记RawBasicBlock结束，可能还需要表（虽然上述方法还是需要表）

对此定义如下：
1、 在multiBlockItem的循环那里判断是否需要更换RawBasicBlock
2、 需要一个变量标志当前RawBasicBlock是否结束
3、 需要一个变量表示当前活跃的RawBasicBlock！
有了当前的活跃变量之后我们发现：generateGraph无需使用IR传参了，当要调取的时候调用这个全局变量即可
