# OpenMP

## 1. OpenMP 基础概念
- 什么是并行计算，线程与进程的区别
- OpenMP 的应用场景和优势
- OpenMP 的编程模型（fork-join 模型）
- 编译选项与运行环境设置

## 2. 并行区域与线程管理
- `#pragma omp parallel` 指令
- 线程数量控制 (`omp_set_num_threads`, `omp_get_num_threads`)
- 线程 ID 获取 (`omp_get_thread_num`)
- 并行执行和串行执行的区别

## 3. 循环并行化
- `#pragma omp for` / `#pragma omp parallel for`
- 循环分配策略：static, dynamic, guided
- 循环变量私有化（private, firstprivate, lastprivate）

## 4. 数据共享与变量属性
- 数据共享类型：shared、private、firstprivate、lastprivate
- 线程安全与竞态条件（race condition）
- `reduction` 操作，用于安全的并行累加/乘积等

## 5. 同步与调度
- 障碍 (`#pragma omp barrier`)
- 临界区 (`#pragma omp critical`)
- 原子操作 (`#pragma omp atomic`)
- 互斥与锁机制（lock/unlock）

## 6. 任务并行
- `#pragma omp task` 与任务依赖
- 任务生成和调度
- 适合不规则计算的并行策略

## 7. 高级特性
- 并行 sections (`#pragma omp sections`)
- 并行 reduction 的高级用法
- Nested parallelism（嵌套并行）
- SIMD 指令与向量化

## 8. 性能调优与调试
- 性能分析：负载均衡、线程开销
- 使用环境变量调整性能（OMP_NUM_THREADS 等）
- 常见调试技巧和工具

---------------------

