---
date:
  created: 2024-12-22 07:22:49
  updated: 2025-01-07 06:37:07
categories:
  - Tech
authors:
  - ruri
---

# Homebrew Formula 二三事

最近经常遇到 homebrew 的打包需求，干脆研究了一下 homebrew Formula。这里以 [byrdocs/byrdocs-cli](https://github.com/byrdocs/byrdocs-cli) 举例，记录一下打包上传的流程
<!-- more -->
## Step.1 `brew tap`

`brew tap` 命令相当于 `git clone` ，将一个包含了 Formula 的仓库 clone 到本地路径 `$(brew --prefix)/Library/Taps` 下

=== "zsh"
    ``` zsh
    brew tap byrdocs/homebrew-byrdocs-cli
    ```

这个命令将 [byrdocs/homebrew-byrdocs-cli](https://github.com/byrdocs/homebrew-byrdocs-cli) 克隆到 `$(brew --prefix)/Library/Taps/byrdocs/homebrew-byrdocs-cli`

待后续步骤完成后即可使用 `brew install byrdocs-cli` 安装 byrdocs-cli

## Step.2 编写 Formula

`byrdocs-cli` 是一个 python command-line tool， 且已经上传到 [PyPI](https://pypi.org/project/byrdocs-cli/) 了。因此我们可以使用 homebrew 官方提供的工具 [homebrew-pypi-poet](https://github.com/tdsmith/homebrew-pypi-poet) 即 poet 来快速生成 Formula

``` zsh
% python3.10 -m venv .venv
% source .venv/bin/activate.fish
% pip install homebrew-pypi-poet byrdocs-cli
% poet -f byrdocs-cli > byrdocs-cli.rb
```

这时候 `byrdocs-cli.rb` 里就是一份包含了必要内容的完整 `formula` 了

!!!TIP
    Homebrew 对 Formula 的命名有着严格的要求：
    
    - `class name` 必须遵守 `PascalCase` 命名法
    - `file name`必须遵守 `kebab-case` 命名法
    - `class name` 和 `file name` 必须对应

=== "byrdocs-cli.rb"
``` Ruby
class ByrdocsCli < Formula
  include Language::Python::Virtualenv

  desc "Shiny new formula"
  homepage "https://github.com/byrdocs/byrdocs-cli"
  url "https://files.pythonhosted.org/packages/66/37/429a8f524b9e23ed21cbef32fe7d6f20e4a24fdec22b4b2df8de473d2a70/byrdocs_cli-0.5.3.tar.gz"
  sha256 "7413f94f900db6a37ecd400376472c3622dad07fcbd2c2585446685dafa877d9"

  depends_on "python3"

  resource "argcomplete" do
    url "https://files.pythonhosted.org/packages/5f/39/27605e133e7f4bb0c8e48c9a6b87101515e3446003e0442761f6a02ac35e/argcomplete-3.5.1.tar.gz"
    sha256 "eb1ee355aa2557bd3d0145de7b06b2a45b0ce461e1e7813f5d066039ab4177b4"
  end

  ...

  resource "wcwidth" do
    url "https://files.pythonhosted.org/packages/6c/63/53559446a878410fc5a5974feb13d31d78d752eb18aeba59c7fef1af7598/wcwidth-0.2.13.tar.gz"
    sha256 "72ea0c06399eb286d978fdedb6923a9eb47e1c486ce63e9b4e64fc18303972b5"
  end

  def install
    virtualenv_create(libexec, "python3")
    virtualenv_install_with_resources
  end

  test do
    false
  end
end
```

逐行解读一下

=== "byrdocs-cli.rb"
```Ruby
class ByrdocsCli < Formula # 定义此文件为 Formula
	include Language::Python::Virtualenv # 使用 `python` 虚拟环境 详见 https://docs.brew.sh/Python-for-Formula-Authors#installing-applications
	homepage "https://github.com/byrdocs/byrdocs-cli"
	url "https://files.pythonhosted.org/packages/66/37/429a8f524b9e23ed21cbef32fe7d6f20e4a24fdec22b4b2df8de473d2a70/byrdocs_cli-0.5.3.tar.gz" # python 代码仓库，此处为PyPI
	sha256 "7413f94f900db6a37ecd400376472c3622dad07fcbd2c2585446685dafa877d9" # 验证下载
	depends_on "python3" # 应修改为 python3.10
	license "MIT" # Homebrew-core 不接受无 license 的 formula
	... # python 依赖

	def install
	    virtualenv_create(libexec, "python3") # 应修改为 python3.10
	    virtualenv_install_with_resources # 安装上面定义的所有 resources 以及程序本体
	end

	test do
		false # 测试用例
	end
end
```

!!!WARNING
    有坑注意！
    这里的 `python3` 并不能严格通过测试，必须修改为特定的某个版本如 `python3.10`
    详见 [Python declarations for applications](https://docs.brew.sh/Python-for-Formula-Authors#python-declarations-for-applications)

!!!TIP
    可以运行以下命令来验证格式和内容是否填写正确
    
    ```zsh
    % brew style <your_formula>
    % brew audit --strict <your_formula>
    ```

最后的完成品: [byrdocs-cli.rb](https://github.com/byrdocs/homebrew-byrdocs-cli/blob/main/byrdocs-cli.rb)

## Step.3 向 homebrew-core 提 pr 或 push 到个人仓库

向 homebrew-core 提 PR, 你可以参考:

- [guidelines for contributing](https://github.com/Homebrew/homebrew-core/blob/HEAD/CONTRIBUTING.md)
- [commit style guide](https://docs.brew.sh/Formula-Cookbook#commit)
- [pull requests](https://github.com/Homebrew/homebrew-core/pulls)

成功 merge 或 push 到个人仓库后，就可以使用 `brew tap` 命令来导入你的 Homebrew Formula 了，可喜可贺，可喜可贺。