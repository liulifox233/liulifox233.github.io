---
date:
  created: 2024-11-14 22:01:35
  updated: 2025-01-07 05:34:05
categories:
  - Tech
authors:
  - ruri
---

# Live2D Cubism Editor Crack Record

!!!NOTICE
    以防自己忘记，在这里记录一下
    
    笔者使用的是 MacOS 版本的 Live2D Cubism Editor 5.1
<!-- more -->
## 尝试

按照常规思路，找找这个软件的许可证放在哪，翻了一下发现在 `/Library/Application Support/Live2D/Cubism License` 这个目录下

试试删除，然而 `live2d_editorE.lic` 在软件重启后仍然会自动生成 

打开看一眼

=== "live2d_editorE.lic"
    ``` 
    # This license created by RLM Internet Activation
    ```

看来是联网激活的，随后在网上找到一个仓库 [RLM_1221_Mod](https://github.com/ShigemoriHakura/rlm1221_Mod) 包含有对 Live2D Cubism Editor 的 RLM Crack

尝试直接用仓库里的 `rlm1603.jar` 替换 `/Applications/Live2D Cubism 5.1/res/rlm1603.jar`，发现软件打不开

后来才看到有 SHA-256 检验，这里作者提供了两种方式解除检验

=== "Way 1"
    - As they added file SHA-256 check, we have to patch the main jar
    - Use jadx and other tools to locate the CEAppDef file (in 4.2.0.1 it's g.class)
    - modify the checksum (using 010 editor or other tool, original checksum is 6b80a0f06acb44524d65d72edf4a097062f41edab54a53063f926d553f9647fa)
    - Remove the .RSA .SF and MANIFEST.MF file in META-INF
    - Move back the main jar and the rlm1221.jar
    - Done~ just enjoy!

=== "Way 2 (WTF is this way lol)"
    - As they added the SHA-256 check, we can also change the .bat file
    - Modify the CLASS_PATH, rename the app\lib\rlm1221.jar to rlm1221_mod
    - Also modify your jar file to rlm1221_mod.jar and copy that into app\lib
    - Change all 4 .bat files and you are ready to go
    - LOL

## 解决方案

使用方法2:

1. 找到 `/Applications/Live2D Cubism 5.1/res/CubismEditor.cfg`

2. 把`app.classpath=$APPDIR/rlm1603.jar` 替换为 `app.classpath=$APPDIR/rlm1603_mod.jar`

3. 把仓库内的`rlm1603.jar` 改名为 `rlm1603_mod.jar`, 放入 `/Applications/Live2D Cubism 5.1/res` 内

大功告成, Enjoy it