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

