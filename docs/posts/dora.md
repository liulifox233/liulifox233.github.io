---
date:
  created: 2025-01-10 03:30:39
categories:
  - Tech
authors:
  - ruri
draft: true
---

# dora-rs 简单食用指北

> 你是否曾为 C++ 项目的编译痛苦万分？  
> 你是否听到过来自 Makefile 的古神低语？  
> 遗憾的是，如果你玩 Robotic, 这些都是你过不去的坎。因为 ROS2 早已积重难返  
> 等等, 真的过不去吗？  
> 为什么不试试 [dora-rs](https://github.com/dora-rs/dora) 呢？

**Dataflow-Oriented Robotic Architecture** (dora-rs) is a framework that makes creation of robotic applications fast and simple.

dora-rs 是一个**数据流导向**机器人架构框架 ，它能让你方便快速开发高效的机器人应用。
<!-- more -->
## 设计预览

dora 框架由不同的组件构成：

![design diagram](../assets/posts/overview.svg)

!!!QUOTE
    - **Nodes:** Dora nodes are separate, isolated processes that communicate with other nodes through the dora library. Nodes can be either a custom, user-specified program. Nodes implement their own `main` function and thus have full control over their execution.

        Nodes use the dora _library_ to communicate with other nodes, which is available for multiple languages (Rust, C; maybe Python, WASM). Communication happens through a _communication layer_, which will be `zenoh` in our first version. We plan to add more options in the future. All communication layer implementations should be robust against disconnections, so operators should be able to keep running even if they lose the connection to the coordinator.

    - **Coordinator:** The coordinator is responsible for reading the dataflow from a YAML file, verifying it, and deploying the nodes to the specified or automatically determined machines. It monitors the operator's health and implements high level cluster management functionality. For example, we could implement automatic scaling for cloud nodes or operator replication and restarts. The coordinator can be controlled through a command line program (CLI).


一个典型的 dora 工作流程:

- Coordinator 启动：在主控机器上启动 dora coordinator，它将作为系统的中央控制单元。

- 守护进程启动：在每台参与的机器上启动 dora daemon，并通过 --coordinator-addr 参数指定协调器的地址，以建立连接。

- 数据流部署：Coordinator 根据数据流配置，指示各守护进程在其管理的机器上启动相应的节点。

- 任务执行：各节点按照定义的操作符执行任务，守护进程负责本地管理，Coordinator 进行全局监控和协调。

## dora-rs cli

### Install

你可以用以下命令在任何具有有 rust 环境的机器下按照 dora-rs cli

```bash
cargo install --locked dora-cli
```

### Usage

```bash
$ dora --help
dora-rs cli client

Usage: dora <COMMAND>

Commands:
  check        Check if the coordinator and the daemon is running
  graph        Generate a visualization of the given graph using mermaid.js. Use --open to open browser
  build        Run build commands provided in the given dataflow
  new          Generate a new project or node. Choose the language between Rust, Python, C or C++
  run          Run a dataflow locally
  up           Spawn coordinator and daemon in local mode (with default config)
  destroy      Destroy running coordinator and daemon. If some dataflows are still running, they will be stopped first
  start        Start the given dataflow path. Attach a name to the running dataflow by using --name
  stop         Stop the given dataflow UUID. If no id is provided, you will be able to choose between the running dataflows
  list         List running dataflows
  logs         Show logs of a given dataflow and node
  daemon       Run daemon
  runtime      Run runtime
  coordinator  Run coordinator
  help         Print this message or the help of the given subcommand(s)

Options:
  -h, --help     Print help
  -V, --version  Print version
```

解释几个重要的命令:

```bash
$ dora build <PATH> # 构建 Workflow 内定义的节点, 在 dora run 之前运行
$ dora run <PATH> # 快速在本地环境下启动 Daemon 和 Coordinator 并运行动态节点, 用于开发
$ dora up # 快速在本地环境下启动 Daemon 和 Coordinator
$ dora destroy # 关闭 Daemon 和 Coordinator
$ dora start <PATH> --name <NAME> # 启动 Workflow, 但不运行节点
$ dora logs [UUID_OR_NAME] <NAME> # [UUID_OR_NAME]是 Workflow 名称或 uuid, <NAME> 是节点名称
$ dora list # 列出所有活动的 Workflow
```

## 如何编写一个 Workflow ?

