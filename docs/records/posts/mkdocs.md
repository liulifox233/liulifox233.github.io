---
date:
  created: 2025-01-08 23:27:30
categories:
  - Tech
authors:
  - ruri
hero: assets/posts/空.webp
---

# 部署 MkDocs 至 Cloudflare Pages

受不了 Hexo 了, 于是把博客迁移到了 [Material for MkDocs](https://squidfunk.github.io/mkdocs-material/).

然而假如你直接使用 Cloudflare Pages [官方给出的 MkDocs 预设](https://developers.cloudflare.com/pages/framework-guides/deploy-an-mkdocs-site/)...
<!-- more -->
=== "Error"
```bash
Executing user command: mkdocs build
/bin/sh: 1: mkdocs: not found
Failed: Error while executing user command. Exited with error code: 127
Failed: build command exited with code: 1
Failed: error occurred while running build command
```

这说明 Cloudflare Pages 对 MkDocs 的支持并不到位, 但是没关系，这里给出两种解决方案

### 方案一: 使用 Github Action 提前构建 ( Recommended )

=== "action.yml"
```yaml
name: CI
on:
  push:
    branches: [ "main" ]
jobs:
  build:
    env:
      PYTHONPATH: .
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
      with:
        fetch-depth: 0

    - name: Set up Python 3.8
      uses: actions/setup-python@v3
      with:
        python-version: "3.8"

    - name: ⚙️ Install dependencies
      run: |
        python -m pip install --upgrade pip
        pip install mkdocs
        pip install $(mkdocs get-deps)

    - name: Setup GIT user
      uses: fregante/setup-git-user@v1

    - name: 🏗️ Publish Site
      run: mkdocs gh-deploy

```

这样每次 push 后 action 就会自动将构建好的版本上传到仓库的 gh-pages 分支

之后只需要在 Cloudflare Pages 中设置好目标仓库的分支就完成啦

### 方案二: 编辑 Cloudflare Pages 构建命令

**编辑构建命令**:

| Configuration option | Value|
| ----------- | ----------- |
| Production branch | `main` |
| Build command | `pip install mkdocs && pip install $(mkdocs get-deps) && PYTHONPATH=. mkdocs build` |
| Build directory | `site` |

**设置环境变量** (Environment variables (advanced): 设置 `PYTHON_VERSION` 为 `3.8`  > Add variable)

## 总结

两种方案均可行，实测 Github Action 方案部署速度更快