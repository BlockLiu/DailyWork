#### 问题&变化

- 问题：时间窗口w以内的cardinality查询
- 变化：将时间窗口等分成127份

#### 数据结构

- L3：m个bucket
  - 8bit
  - 初始化为-1
  - 每相邻k个bucket构成一个line（比如k=16）
- L2：256个counter
  - 32bit
  - 初始化为0
- L1：1个counter
  - 32bit
  - 初始化为0

#### 算法（插入flowID e)

- 计算哈希pos=h(e)%m
- 假设当前时间为t，那么当前应记录的时间戳为 ts = floor{t / (w/127)} % 255
  - 时间戳初始化为0xFF，表示该bucket未被使用过
- 将L3中第pos个bucket置为ts
- 将L3中第pos个bucket同一line里面过期的bucket置为-1
  - 过期bucket：时间戳不在 (ts-127)~(ts-1)之间的bucket
    - e.g.，当前时间为301：那么 174~300为为过期，其它的为过期时间戳
    - e.g.，当前时间为67：那么 0~66 和 195~254为未过期，其它的为过期时间戳
- 在step 3和step 4改变bucket值的时候，相应的将L2中的counter值变化
- L2的counter值变化后，根据当前ts，统计L2中第ts个counter之前127个counter的和，更新为L1的值



#### NP平台实现，性能指标（当成CPU来看）

- 內存需求
  - L3共有m个bucket存放8-bit的时间信息：m bytes
    - m需要根据每分钟流的个数确定，因此将L3放在DRAM中
  - L2共有256个counter，每个counter存放32-bit的统计信息：256*32 = 1KBytes
    - 通常的processor的L1-cache有[32KB](https://www.makeuseof.com/tag/what-is-cpu-cache/)，因此可将L2放在L1-cache中
  - L1共有1个32-bit counter
    - 常量放在寄存器中
  - 因此，总的内存需求为m bytes
- 访存需求（插入）:
  - （pos为哈希映射位置）将pos所在group（k个buckets）从内存取到L1-cache：
    - 1次内存访问
    - L1-cache的cache line大小为64bytes，因此要求一个group不能超过64个buckets
  - 对k个buckets逐一（or并行）比较
    - 需要k次L1-cache访问
  - 比较完成后最坏需要对L2 counter进行k次修改
    - 需要k次L1-cache访问
  - 对L1 counter的修改在寄存器中进行（最多修改k次）
  - [各级cache/mem访问时间](https://stackoverflow.com/questions/4087280/approximate-cost-to-access-various-caches-and-main-memory)
    - L1-cache hit:		4 cycles (2.1 - 1.2 ns)
    - local DRAM: 	    ~60ns
    - 一次DRAM访问可以进行大概30次L1-cache访问。因此，将k设置为16可以有效的利用DRAM访存时间
  - 总而言之，一次插入只需要一次内存访问
- 指令数



#### barefoot交换机实现，性能指标

- 大概思路?
  - 实现最简单版本的bitmap
  - 数据结构：m个64-bit的bucket
  - 插入：（包e，带准确时间戳ts）
    - 计算h(e)%m，将对应bucket的时间戳置为ts
  - 查询：遍历整个array，将有效的bucket取出，按照经典bitmap计算

- 內存：使用一个table

  - P4 switch里64位寄存器只能使用一半作为时间戳？？（分高低位）因此使用32-bit时间戳
  - 总共 4m bytes

- 资源使用表格（2 stages）：

  |     Resource usage     | Total | percentage |
  | :--------------------: | :---: | :--------: |
  | Exact Match Input xbar |  19   |   1.24%    |
  |        Hash Bit        |  16   |   0.32%    |
  |     Hash Dist Unit     |   1   |   1.39%    |
  |          SRAM          |  33   |   3.44%    |
  |        Map RAM         |  33   |   5.73%    |
  |       Meter ALU        |   1   |   2.08%    |
  | Exact Match Search Bus |   2   |   1.04%    |
  | Exact Match Result Bus |   2   |   1.04%    |

- 访存（插入）：只需要修改一个bucket

  

