# CAP定理及其证明

分布式系统最多同时满足以下三个特性中的两个：

1. **一致性 (Consistency, C)**  
   所有节点访问同一份最新数据副本。

2. **可用性 (Availability, A)**  
   每个请求都能在有限时间内获得响应（非错误）。

3. **分区容错性 (Partition Tolerance, P)**  
   网络分区发生时，系统仍能继续运行。

$$
\boxed{\text{P} \implies \neg (\text{C} \land \text{A})}
$$

或

$$
\boxed{\text{P}  \wedge \text{C}  \wedge \text{A}}
$$

## 证明

设系统由两个节点 $N_1$ 和 $N_2$，存储数据项 $x$，初始值 $x = v_0$  

且网络异步通信（消息延迟无上限）

$$
\begin{aligned}
t_0 &: \forall N(x = v_0)\\
t_1 &: N_1: \text{write}(x, v_1) \\
t_2 &: N_2: \text{read}(x) \\
[t_1, \infty] &: \text{网络发生分区，} N_1 \leftrightarrow N_2 \text{ 通信中断}
\end{aligned}
$$

### Case1: $\text{P} \wedge \text{C}$

则：
$$
\exists t \in [t_1, t_2], \, \text{Response}(\text{read}(x)) = v_0 \neq v_1
$$
因此 $\neg A$

### Case2: $\text{P} \wedge \text{A}$
$N_2$ 必须返回最新值 $v_1$  

然而：
$$
\forall t \geq t_2, \, \text{Response}(\text{read}(x)) \text{ is incomplete}
$$

因此 $\neg C$，证毕