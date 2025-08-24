# NEON SIMD （MacBook M2 / Apple Silicon）

## 前置条件
- 熟悉 C++（含新特性：模板、constexpr、std::array 等）
- 熟悉基本指针和数组操作
- 熟悉 clang / gcc 编译选项
- 基本性能分析工具使用：Xcode Instruments, perf, clang -O3

---

## 阶段 0：SIMD & NEON 基础
**目标**：理解 SIMD 概念及 NEON 的寄存器和数据类型

- 理论
  - SIMD 与数据级并行思想
  - NEON 寄存器：
    - Q0–Q15: 128-bit 向量寄存器
    - D0–D31: 64-bit 寄存器
  - 数据类型：
    - `int8x16_t`, `int16x8_t`, `int32x4_t`, `float32x4_t`
  - 基本操作：加减乘除、逻辑运算、移位、比较
- 实践
  - 编写简单向量加法、乘法程序
  - 对比标量 vs NEON 性能
- 参考
  - [ARM NEON Intrinsics](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics)
  - [ARM SIMD Introduction](https://blog.csdn.net/mzpmzk/article/details/114686930)

---

## 阶段 1：NEON Intrinsics 熟悉
**目标**：熟练使用常用 NEON Intrinsics 并向量化循环

- 数据加载/存储
  - `vld1q_f32`, `vst1q_f32`（对齐/非对齐）
- 算术操作
  - `vaddq_f32`, `vmulq_f32`, `vsubq_s32`
- 逻辑与比较
  - `vandq_u32`, `vorrq_u32`, `vcgtq_f32`
- 向量拆分/合并/转置
  - `vzip`, `vuzp`, `vtrn`
- 实践项目
  - NEON 实现矩阵加法、矩阵乘法、卷积
  - 使用 `clang -S` 查看汇编向量化效果

---

## 阶段 2：性能优化技巧
**目标**：掌握高性能 NEON 编程的优化手法

- 技巧
  - 避免分支，使用掩码操作
  - 数据对齐、缓存友好
  - 循环展开、软件预取
  - 尾部标量处理（tail handling）
- 实践
  - 优化已有矩阵乘法或图像滤波算法
  - 使用 Instruments / perf 测量性能提升

---

## 阶段 3：高级 NEON 技巧
**目标**：复杂算法向量化和平台相关特性

- 多维向量化
  - 矩阵、卷积、FFT
- NEON 与 C++ 模板结合
  - 写泛型向量化函数
- 与框架结合
  - Accelerate, Metal SIMD kernels
- 实践
  - 用 NEON 加速卷积神经网络前向推理或 FFT
  - 探索内存布局优化（AoS vs SoA）

---

## 阶段 4：项目实战
**目标**：综合应用 NEON SIMD 编程

- 图像处理项目
  - 滤波器、灰度转换、卷积
- 数值计算项目
  - 矩阵乘法、向量点积、FFT
- 深度学习加速实验
  - 卷积、全连接前向推理
- 分析与优化
  - 比较标量、NEON Intrinsics、Apple Accelerate 性能
  - 使用性能分析工具调优缓存和指令吞吐率

---

## 建议学习顺序
1. 阶段 0：先理解 SIMD 与 NEON 基础
2. 阶段 1：掌握 Intrinsics 并向量化简单循环
3. 阶段 2：优化性能
4. 阶段 3：探索高级向量化技巧
5. 阶段 4：项目实战，巩固技能


------

`v<mod><opname><shape><flags>_<type>`
`v vector，表示这是 NEON 向量操作`

`<mod> 修饰符，对运算行为进行调整`

`<opname> 运算类型，比如 add/sub/mul`

`<shape> 输出向量长度变化或高低位处理`

`<flags> 操作寄存器类型（D寄存器64-bit 或 Q寄存器128-bit）`

`<type> 数据类型和元素类型`


| 修饰符 | 含义               | 示例                                 |
| --- | ---------------- | ---------------------------------- |
| `q` | 饱和计算（saturating） | `vqadd_s8(a,b)` → 相加结果超过范围时饱和      |
| `h` | 折半计算（halving）    | `vhsub_s8(a,b)` → 结果右移一位 `(a-b)/2` |
| `d` | 加倍计算（doubling）   | `vqdmull_s16(a,b)` → 结果扩大两倍并做饱和    |
| `r` | 舍入计算（rounding）   | `vrhadd_s8(a,b)` → `(a+b+1)/2`     |
| `p` | pairwise 两两操作    | `vpadd_s8(a,b)` → 相邻元素两两相加         |

| shape   | 含义                            | 示例                                              |
| ------- | ----------------------------- | ----------------------------------------------- |
| `l`     | long，输出元素长度 = 输入长度 × 2        | `vaddl_u8(a,b)` → uint8x8 → uint16x8            |
| `w`     | wide，第一个向量与输出长度一致，第二个向量元素长度加倍 | `vsubw_u8(a,b)` → uint16x8 = uint16x8 - uint8x8 |
| `n`     | narrow，输出长度 = 输入长度 ÷ 2        | `vmovn_u64(a)` → uint64x2 → uint32x2            |
| `_high` | AArch64 专用，处理高64位             | `vsubl_high_s8(a,b)` → 高64bit参与运算               |
| `_n`    | 标量参与向量计算                      | `vshr_n_s8(a, 3)` → a 向量每个元素右移 3 位              |
| `_lane` | 指定某个通道参与计算                    | `vmul_lane_s16(a, v, 2)` → 取 v 第2通道和 a 向量相乘     |

| flags | 含义                        |
| ----- | ------------------------- |
| `q`   | quad word → 128-bit 寄存器操作 |
| 无     | D 寄存器 → 64-bit 寄存器操作      |

| type             | 含义                   |
| ---------------- | -------------------- |
| `u8/u16/u32/u64` | 无符号整数 8/16/32/64 bit |
| `s8/s16/s32/s64` | 有符号整数 8/16/32/64 bit |
| `f16/f32/f64`    | 半/单/双精度浮点数           |

```
vadd_s32(a, b)       // 普通逐元素加法
vqadd_s32(a, b)      // 饱和加法，溢出取最大值
vaddl_s32(a, b)      // 长指令加法，输出元素位宽扩大
vhadd_s32(a, b)      // 半加 (相加后右移1)
vrhadd_s32(a, b)     // 舍入半加 ((a+b+1)>>1)
vpadd_s8(a,b)        // 相邻 pairwise 相加
vaddw_s32(a,b)       // 宽加法，b 被拓宽再加
vaddhn_s32(a,b)      // 窄加法，输出元素位宽减半

vsubq_s32(a,b)       // 普通减法
vqsub_s32(a,b)       // 饱和减法
vsubl_s32(a,b)       // 长指令减法
vhsub_s32(a,b)       // 半减 (相减右移1)
vsubw_s32(a,b)       // 宽减法
vsubhn_s32(a,b)      // 窄减法

vmul_s32(a,b)        // 元素逐个相乘
vmull_s32(a,b)       // 长指令乘法
vmul_n_s32(a,b)      // 与标量相乘
vmul_lane_s32(a,b,c) // 与向量某个通道相乘
vqdmulh_s32(a,b)     // 饱和乘法

vmla_s32(a,b,c)        // a + b*c
vmlal_s32(a,b,c)       // 长乘加
vmls_s32(a,b,c)        // a - b*c
vmlsl_s32(a,b,c)       // 长乘减

vrecpe_f32(a)          // 近似倒数
vrecps_f32(a,b)        // Newton-Raphson 步
vrsqrts_f32(a,b)       // 近似平方根

vneg_s32(a)             // 每个元素取负

vrndn_f32(a)            // 四舍五入到最近偶数
vrnda_f32(a)            // 四舍五入，tie away from zero
vrndp_f32(a)            // 向 +∞
vrndm_f32(a)            // 向 -∞
vrnd_f32(a)             // 向 0

vceq_s32(a,b)   // ==
vcge_s32(a,b)   // >=
vcle_s32(a,b)   // <=
vcgt_s32(a,b)   // >
vclt_s32(a,b)   // <
vcage_f32(a,b)  // |a| >= |b|
vcalt_f32(a,b)  // |a| < |b|
vtst_s32(a,b)   // a & b != 0

vabs_s32(a)     // |a|
vabd_s32(a,b)   // |a-b|
vaba_s32(a,b,c) // a + |b-c|

vmax_s32(a,b)   // 元素间取大
vmin_s32(a,b)   // 元素间取小
vpmax_s32(a,b)  // 相邻 pairwise max
vpmin_s32(a,b)  // 相邻 pairwise min

vshlq_u16(a,b)    // 左移，可传负数表示右移
vrshlq_u16(a,b)   // 左移 + 四舍五入
vqshlq_u16(a,b)   // 饱和截断
vshlq_n_u16(a,b)  // 按常数左移
vshrq_n_u16(a,b)  // 按常数右移
vsraq_n_u16(a,b,c) // 右移累加
vsliq_n_u16(a,b,c) // 左移插入
vsriq_n_u16(a,b,c) // 右移插入

vmvn_s32(a)    // ~a
vand_s32(a,b)  // a & b
vorr_s32(a,b)  // a | b
veor_s32(a,b)  // a ^ b
vbic_s32(a,b)  // ~a & b
vorn_s32(a,b)  // a | (~b)

vcvtq_u32_f32(a)   // f32 → u32
vcvtq_f32_u32(a)   // u32 → f32
vcvt_f16_f32(a)    // f32 → f16
vcvt_f32_f16(a)    // f16 → f32
vmovl_s8(a)        // int8 → int16
vqmovn_s16(a)      // int16 → int8 (饱和)
vreinterpret_f32_u32(a) // 重新解释类型，不改变位

vext_s8(a,b,c)   // 组合 a,b 向量，取 c 个元素偏移
vtbl1_s8(a,b)    // 查表索引
vtbl2_s8(a,b)    // 两个向量查表
vtbx1_s8(a,b,c)  // 查表并替换
vbsl_s8(mask,a,b)// mask 选择 a 或 b 的位

vrev64_s8(a)  // 元素反转，64位粒度
vrev32_s8(a)  // 元素反转，32位粒度
vrev16_s8(a)  // 元素反转，16位粒度

vtrn_s8(a,b)  // 交叉转置
vzip_s8(a,b)  // 交叉排列
vuzp_s8(a,b)  // 反交叉排列
vcombine_u8(low,high)  // 两个 64bit → 一个 128bit
vget_low_u8(a)          // 获取低半部分
vget_high_u8(a)         // 获取高半部分
```
