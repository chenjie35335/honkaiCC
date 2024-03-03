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