---
title: AIGC Detection
aigc_sidebar: true
---

使用 n-gram 模型分析文本 AI 生成率。

实现原理和模型参考 [用"古典"机器学习检测 LLM 生成的网文 (AIGC 文本检测)](https://blog.lyc8503.net/post/llm-classifier)。

!!!WARNING
    此 AI 检测模型 {==非通用模型==}，检测结果纯 {==图一乐==}。

<div id="aigc-app">
  <div class="aigc-input-wrap" id="aigc-input-wrap">
    <textarea
      id="aigc-textarea"
      class="aigc-textarea"
      placeholder="在此粘贴需要检测的文本…"
    ></textarea>
    <div id="aigc-output" class="aigc-output" style="display:none"></div>
  </div>
</div>
