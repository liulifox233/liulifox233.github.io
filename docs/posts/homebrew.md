---
date:
  created: 2024-12-22 07:22:49
  updated: 2025-01-08 18:58:09
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

!!!TIP
    可以运行以下命令来验证格式和内容是否填写正确
    
    ```zsh
    % brew style <your_formula>
    % brew audit --strict <your_formula>
    ```

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

### Metadata

此处定义软件包的元信息

!!!INFO
    `depends_on "python3"` 不能严格通过测试，必须修改为特定的某个版本如 python@3.10
    详见 [Python declarations for applications](https://docs.brew.sh/Python-for-Formula-Authors#python-declarations-for-applications)

!!!WARNING
    Homebrew-core 不接受无 license 的 formula

=== "byrdocs-cli.rb"
```Ruby
include Language::Python::Virtualenv # 使用 `python` 虚拟环境 详见 https://docs.brew.sh/Python-for-Formula-Authors#installing-applications
homepage "https://github.com/byrdocs/byrdocs-cli"
url "https://files.pythonhosted.org/packages/66/37/429a8f524b9e23ed21cbef32fe7d6f20e4a24fdec22b4b2df8de473d2a70/byrdocs_cli-0.5.3.tar.gz" # python 代码仓库，此处为PyPI
sha256 "7413f94f900db6a37ecd400376472c3622dad07fcbd2c2585446685dafa877d9" # 验证下载
depends_on "python@3.10"
license "MIT" # Homebrew-core 不接受无 license 的 formula
```

### Install

这里的代码会在 `brew install <formula>` 时运行

=== "byrdocs-cli.rb"
```Ruby
def install
    virtualenv_create(libexec, "python3") # 应修改为 python3.10
    virtualenv_install_with_resources # 安装上面定义的所有 resources 以及程序本体
end
```

### Test

这里的代码会在用户运行 `brew test <formula>` 命令时运行，用于测试软件包是否构建成功

=== "byrdocs-cli.rb"
```Ruby
test do
    expect = "usage: byrdocs [-h] [--token TOKEN] [--manually] [command] [file]"
    assert_match expect, pipe_output("#{bin}/byrdocs --help 2>&1")
end
```

### 完成品参考 [byrdocs-cli.rb](https://github.com/byrdocs/homebrew-byrdocs-cli/blob/main/byrdocs-cli.rb)
=== "byrdocs-cli.rb"
```Ruby
class ByrdocsCli < Formula
  include Language::Python::Virtualenv

  desc "Command-line tool for uploading files to byrdocs.org"
  homepage "https://github.com/byrdocs/byrdocs-cli"
  license "MIT"

  url "https://files.pythonhosted.org/packages/ca/3d/b75a97066d4690cb08009fa7c135342b965b80e96aa46332fa6a3f2dabf1/byrdocs_cli-0.5.5.tar.gz"
  sha256 "6a870124e1f8cddb4e66450436993a9adf221de39e967467de0242b80591fd28"

  depends_on "python@3.10"

  resource "argcomplete" do
    url "https://files.pythonhosted.org/packages/5f/39/27605e133e7f4bb0c8e48c9a6b87101515e3446003e0442761f6a02ac35e/argcomplete-3.5.1.tar.gz"
    sha256 "eb1ee355aa2557bd3d0145de7b06b2a45b0ce461e1e7813f5d066039ab4177b4"
  end

  resource "boto3" do
    url "https://files.pythonhosted.org/packages/f8/94/8f0eeb6201904c08ffd4f24ad8313162ad72f9c6bfe179cbb9b8d3bcdd49/boto3-1.35.62.tar.gz"
    sha256 "f80eefe7506aa01799b1027d03eddfd3c4a60548d6db5c32f139e1dec9f3f4f5"
  end

  resource "botocore" do
    url "https://files.pythonhosted.org/packages/5b/59/832c6d4dc8812998f2a9211ebaba8f5bd1bb418f020dbfa93dd8e18c917c/botocore-1.35.62.tar.gz"
    sha256 "9df762294d5c727d9ea1c48b98579729a0ba40fd317c3262a6b8d8e12fb67489"
  end

  resource "certifi" do
    url "https://files.pythonhosted.org/packages/0f/bd/1d41ee578ce09523c81a15426705dd20969f5abf006d1afe8aeff0dd776a/certifi-2024.12.14.tar.gz"
    sha256 "b650d30f370c2b724812bee08008be0c4163b163ddaec3f2546c1caf65f191db"
  end

  resource "charset-normalizer" do
    url "https://files.pythonhosted.org/packages/16/b0/572805e227f01586461c80e0fd25d65a2115599cc9dad142fee4b747c357/charset_normalizer-3.4.1.tar.gz"
    sha256 "44251f18cd68a75b56585dd00dae26183e102cd5e0f9f1466e6df5da2ed64ea3"
  end

  resource "idna" do
    url "https://files.pythonhosted.org/packages/f1/70/7703c29685631f5a7590aa73f1f1d3fa9a380e654b86af429e0934a32f7d/idna-3.10.tar.gz"
    sha256 "12f65c9b470abda6dc35cf8e63cc574b1c52b11df2c86030af0ac09b01b13ea9"
  end

  resource "inquirerpy" do
    url "https://files.pythonhosted.org/packages/64/73/7570847b9da026e07053da3bbe2ac7ea6cde6bb2cbd3c7a5a950fa0ae40b/InquirerPy-0.3.4.tar.gz"
    sha256 "89d2ada0111f337483cb41ae31073108b2ec1e618a49d7110b0d7ade89fc197e"
  end

  resource "isbnlib" do
    url "https://files.pythonhosted.org/packages/9e/6d/55b9ee89fdfb3aacb92b975a60357c7aa547db358817e16be3b6f8f5d781/isbnlib-3.10.14.tar.gz"
    sha256 "96f90864c77b01f55fa11e5bfca9fd909501d9842f3bc710d4eab85195d90539"
  end

  resource "jmespath" do
    url "https://files.pythonhosted.org/packages/00/2a/e867e8531cf3e36b41201936b7fa7ba7b5702dbef42922193f05c8976cd6/jmespath-1.0.1.tar.gz"
    sha256 "90261b206d6defd58fdd5e85f478bf633a2901798906be2ad389150c5c60edbe"
  end

  resource "pfzy" do
    url "https://files.pythonhosted.org/packages/d9/5a/32b50c077c86bfccc7bed4881c5a2b823518f5450a30e639db5d3711952e/pfzy-0.3.4.tar.gz"
    sha256 "717ea765dd10b63618e7298b2d98efd819e0b30cd5905c9707223dceeb94b3f1"
  end

  resource "pinyin" do
    url "https://files.pythonhosted.org/packages/32/95/d2969f1071b7bc0afff407d1d7b4b3f445e8e6b59df7921c9c09e35ee375/pinyin-0.4.0.tar.gz"
    sha256 "8842ae53cb7a81c8c3ec03d1cd7dba9aedb20d8d6962aebc620fad74da0868f5"
  end

  resource "prompt-toolkit" do
    url "https://files.pythonhosted.org/packages/2d/4f/feb5e137aff82f7c7f3248267b97451da3644f6cdc218edfe549fb354127/prompt_toolkit-3.0.48.tar.gz"
    sha256 "d6623ab0477a80df74e646bdbc93621143f5caf104206aa29294d53de1a03d90"
  end

  resource "python-dateutil" do
    url "https://files.pythonhosted.org/packages/66/c0/0c8b6ad9f17a802ee498c46e004a0eb49bc148f2fd230864601a86dcf6db/python-dateutil-2.9.0.post0.tar.gz"
    sha256 "37dd54208da7e1cd875388217d5e00ebd4179249f90fb72437e91a35459a0ad3"
  end

  resource "PyYAML" do
    url "https://files.pythonhosted.org/packages/54/ed/79a089b6be93607fa5cdaedf301d7dfb23af5f25c398d5ead2525b063e17/pyyaml-6.0.2.tar.gz"
    sha256 "d584d9ec91ad65861cc08d42e834324ef890a082e591037abe114850ff7bbc3e"
  end

  resource "requests" do
    url "https://files.pythonhosted.org/packages/63/70/2bf7780ad2d390a8d301ad0b550f1581eadbd9a20f896afe06353c2a2913/requests-2.32.3.tar.gz"
    sha256 "55365417734eb18255590a9ff9eb97e9e1da868d4ccd6402399eaf68af20a760"
  end

  resource "s3transfer" do
    url "https://files.pythonhosted.org/packages/c0/0a/1cdbabf9edd0ea7747efdf6c9ab4e7061b085aa7f9bfc36bb1601563b069/s3transfer-0.10.4.tar.gz"
    sha256 "29edc09801743c21eb5ecbc617a152df41d3c287f67b615f73e5f750583666a7"
  end

  resource "six" do
    url "https://files.pythonhosted.org/packages/94/e7/b2c673351809dca68a0e064b6af791aa332cf192da575fd474ed7d6f16a2/six-1.17.0.tar.gz"
    sha256 "ff70335d468e7eb6ec65b95b99d3a2836546063f63acc5171de367e834932a81"
  end

  resource "tqdm" do
    url "https://files.pythonhosted.org/packages/e8/4f/0153c21dc5779a49a0598c445b1978126b1344bab9ee71e53e44877e14e0/tqdm-4.67.0.tar.gz"
    sha256 "fe5a6f95e6fe0b9755e9469b77b9c3cf850048224ecaa8293d7d2d31f97d869a"
  end

  resource "urllib3" do
    url "https://files.pythonhosted.org/packages/aa/63/e53da845320b757bf29ef6a9062f5c669fe997973f966045cb019c3f4b66/urllib3-2.3.0.tar.gz"
    sha256 "f8c5449b3cf0861679ce7e0503c7b44b5ec981bec0d1d3795a07f1ba96f0204d"
  end

  resource "wcwidth" do
    url "https://files.pythonhosted.org/packages/6c/63/53559446a878410fc5a5974feb13d31d78d752eb18aeba59c7fef1af7598/wcwidth-0.2.13.tar.gz"
    sha256 "72ea0c06399eb286d978fdedb6923a9eb47e1c486ce63e9b4e64fc18303972b5"
  end

  def install
    virtualenv_create(libexec, "python3.10")
    virtualenv_install_with_resources
  end

  test do
    expect = "usage: byrdocs [-h] [--token TOKEN] [--manually] [command] [file]"
    assert_match expect, pipe_output("#{bin}/byrdocs --help 2>&1")
  end
end
```

## Step.3 向 homebrew-core 提交 PR

向 homebrew-core 提交 PR, 你可以参考:

- [guidelines for contributing](https://github.com/Homebrew/homebrew-core/blob/HEAD/CONTRIBUTING.md)

- [commit style guide](https://docs.brew.sh/Formula-Cookbook#commit)

- [pull requests](https://github.com/Homebrew/homebrew-core/pulls)

成功 merge 或 push 到个人仓库后，就可以使用 `brew tap` 命令来导入你的 Homebrew Formula 了，可喜可贺，可喜可贺。