(function () {
  "use strict";

  var RAW_PREFIX = "https://raw.githubusercontent.com/liulifox233/liulifox233.github.io/main/docs/";

  /* ── WASM state (persisted across instant-nav) ─────────────────── */
  var _wasmReady = false;
  var _detector = null;
  var _modelLoaded = false;
  var _normalizeMarkdownForAigc = null;

  async function initWasm() {
    if (_wasmReady) return true;
    try {
      var jsUrl   = new URL("/assets/wasm/ai_text_detector_wasm.js",  location.origin).href;
      var wasmUrl = new URL("/assets/wasm/ai_text_detector_wasm_bg.wasm", location.origin).href;
      var mod = await import(jsUrl);
      await mod.default(wasmUrl);
      _detector = new mod.Detector();
      _wasmReady = true;
      return true;
    } catch (e) {
      console.error("[AIGC] wasm:", e);
      return false;
    }
  }

  async function loadModelBytes(bytes) {
    if (!_wasmReady) throw new Error("WASM not ready");
    _detector.loadModelsFromBytes(bytes);
    _modelLoaded = _detector.isReady();
  }

  async function normalizeMarkdownForAigc(text) {
    if (_normalizeMarkdownForAigc) return _normalizeMarkdownForAigc(text);
    var mod = await import(new URL("/javascripts/aigc-shared.mjs", location.origin).href);
    _normalizeMarkdownForAigc = mod.normalizeMarkdownForAigc;
    return _normalizeMarkdownForAigc(text);
  }

  /* ── Labels ───────────────────────────────────────────────────── */

  var MODEL_LABELS = {
    gemini: "Gemini",
    qwen: "Qwen",
    pony: "Pony",
    kimi25: "Kimi 2.5",
    glm47: "GLM-4.7",
    doubao: "Doubao",
    deepseekv32: "DeepSeek V3.2",
  };

  var VERDICT_LABEL = {
    "human":       "Human",
    "maybe-human": "Maybe Human",
    "maybe-ai":    "Maybe AI",
  };

  /* ── Page init ────────────────────────────────────────────────── */

  function initPage() {
    /* Main content elements */
    var app = document.getElementById("aigc-app");
    if (!app) return;
    if (app.dataset.init) return;
    app.dataset.init = "1";

    var textarea   = document.getElementById("aigc-textarea");
    var output     = document.getElementById("aigc-output");
    var analyzeBtn = document.getElementById("aigc-analyze-btn");
    var editBtn    = document.getElementById("aigc-edit-btn");

    /* Sidebar elements (rendered by toc.html override) */
    var sbDot        = document.getElementById("aigc-sb-dot");
    var sbStatusText = document.getElementById("aigc-sb-status-text");
    var sbUpload     = document.getElementById("aigc-sb-upload");
    var fileInput    = document.getElementById("aigc-model-file");
    var sbScore      = document.getElementById("aigc-sb-score");
    var sbVerdict    = document.getElementById("aigc-sb-score-verdict");
    var sbNum        = document.getElementById("aigc-sb-score-num");
    var sbBar        = document.getElementById("aigc-sb-bar");
    var sbSub        = document.getElementById("aigc-sb-score-sub");
    var sbStats      = document.getElementById("aigc-sb-stats");
    var sbDl         = document.getElementById("aigc-sb-dl");
    var sbModels     = document.getElementById("aigc-sb-models");
    var sbBreakdown  = document.getElementById("aigc-sb-breakdown");

    var mode = "edit"; // "edit" | "view"
    var pendingAutostart = false;

    /* ── Status helpers ── */
    function setStatus(s, text) {
      if (sbDot) sbDot.dataset.s = s;
      if (sbStatusText) sbStatusText.textContent = text;
    }

    function setModelReady() {
      setStatus("ready", "模型已就绪");
      if (sbUpload) sbUpload.style.display = "none";
      refreshBtn();
    }

    function showUpload() {
      setStatus("warn", "未找到模型文件");
      if (sbUpload) sbUpload.style.display = "flex";
      refreshBtn();
    }

    /* ── Textarea auto-resize ── */
    function autoResize() {
      textarea.style.height = "auto";
      textarea.style.height = Math.max(520, textarea.scrollHeight) + "px";
    }

    textarea.addEventListener("input", function () {
      autoResize();
      refreshBtn();
    });

    function refreshBtn() {
      analyzeBtn.disabled =
        !_modelLoaded || mode === "view" || textarea.value.trim().length < 10;
    }

    /* ── Edit / view toggle ── */
    function switchToView(report) {
      renderOutput(report.chunks);
      textarea.style.display = "none";
      output.style.display = "block";
      mode = "view";
      editBtn.style.display = "inline-flex";
      analyzeBtn.style.display = "none";
    }

    function switchToEdit() {
      output.style.display = "none";
      textarea.style.display = "block";
      mode = "edit";
      editBtn.style.display = "none";
      analyzeBtn.style.display = "inline-flex";
      autoResize();
      refreshBtn();
      textarea.focus();
    }

    editBtn.addEventListener("click", switchToEdit);
    output.addEventListener("click", switchToEdit);

    /* ── File upload ── */
    if (fileInput) {
      fileInput.addEventListener("change", async function () {
        var file = fileInput.files && fileInput.files[0];
        if (!file) return;
        setStatus("loading", "正在加载模型…");
        try {
          var bytes = new Uint8Array(await file.arrayBuffer());
          await loadModelBytes(bytes);
          setModelReady();
        } catch (e) {
          setStatus("error", "加载失败：" + e.message);
        }
      });
    }

    /* ── Analyze ── */
    analyzeBtn.addEventListener("click", function () {
      var text = textarea.value.trim();
      if (!text || !_modelLoaded) return;
      analyzeBtn.textContent = "检测中…";
      analyzeBtn.disabled = true;
      setTimeout(function () {
        try {
          var report = _detector.analyzeText(text);
          renderSidebar(report);
          switchToView(report);
        } catch (e) {
          setStatus("error", "检测出错，请重试");
          console.error("[AIGC]", e);
        } finally {
          analyzeBtn.textContent = "检测";
          refreshBtn();
        }
      }, 16);
    });

    loadArticleFromQuery();

    /* ── Bootstrap ── */
    (async function bootstrap() {
      if (!_wasmReady) {
        setStatus("loading", "正在加载 WASM 模块…");
        var ok = await initWasm();
        if (!ok) { setStatus("error", "WASM 加载失败，请刷新"); return; }
      }
      if (_modelLoaded) {
        setModelReady();
        maybeAutoAnalyze();
        return;
      }
      setStatus("loading", "正在加载模型文件…");
      try {
        var res = await fetch(new URL("/assets/models.bin", location.origin).href);
        if (!res.ok) throw new Error("HTTP " + res.status);
        var bytes = new Uint8Array(await res.arrayBuffer());
        await loadModelBytes(bytes);
        setModelReady();
        maybeAutoAnalyze();
      } catch (_) {
        showUpload();
      }
    })();

    async function loadArticleFromQuery() {
      try {
        var rawUrl = readRawUrlFromQuery();
        if (!rawUrl) return;

        setStatus("loading", "正在加载源 Markdown…");
        var res = await fetch(rawUrl, { credentials: "omit" });
        if (!res.ok) throw new Error("HTTP " + res.status);

        var rawMarkdown = await res.text();
        var normalized = await normalizeMarkdownForAigc(rawMarkdown);
        if (!normalized || normalized.trim().length < 10) {
          throw new Error("正文过短或为空");
        }

        textarea.value = normalized;
        autoResize();
        pendingAutostart = true;
        refreshBtn();

        if (_modelLoaded) {
          setModelReady();
          maybeAutoAnalyze();
        } else {
          setStatus("loading", "源 Markdown 已加载，等待模型就绪…");
        }
      } catch (e) {
        setStatus("error", "源 Markdown 加载失败：" + e.message);
      }
    }

    function maybeAutoAnalyze() {
      if (!pendingAutostart || !_modelLoaded || mode !== "edit") return;
      pendingAutostart = false;
      analyzeBtn.click();
    }

    /* ── Render sidebar results ── */
    function renderSidebar(report) {
      var pct     = report.weightedPercent != null ? report.weightedPercent
                  : report.charPercent    != null ? report.charPercent : 0;
      var verdict = report.verdict || "human";

      if (sbScore) {
        sbScore.dataset.v = verdict;
        sbScore.style.display = "flex";
      }
      if (sbVerdict) sbVerdict.textContent = VERDICT_LABEL[verdict] || verdict;
      if (sbNum)     sbNum.textContent = pct.toFixed(1) + "%";
      if (sbBar)     sbBar.style.width = Math.min(pct, 100) + "%";
      if (sbSub)     sbSub.textContent =
        (report.aiChars || 0) + " / " + (report.totalChars || 0) + " 字符被标记";

      /* Stats */
      if (sbDl) {
        sbDl.innerHTML = "";
        [
          ["字符 AIGC 率", fmt(report.charPercent) + "%"],
          ["句子 AIGC 率", fmt(report.sentencePercent) + "%"],
          ["加权 AIGC 率", fmt(report.weightedPercent) + "%"],
          ["AIGC 句子数",  (report.aiChunks || 0) + " / " + (report.totalChunks || 0)],
        ].forEach(function (pair) {
          var dt = document.createElement("dt"); dt.textContent = pair[0];
          var dd = document.createElement("dd"); dd.textContent = pair[1];
          sbDl.appendChild(dt); sbDl.appendChild(dd);
        });
      }
      if (sbStats) sbStats.style.display = "flex";

      /* Model breakdown */
      if (sbBreakdown && report.breakdown && report.breakdown.length) {
        sbBreakdown.innerHTML = "";
        report.breakdown.forEach(function (item) {
          var p   = (item.percent || 0);
          var row = document.createElement("div");
          row.className = "aigc-bd-row";
          row.innerHTML =
            '<span class="aigc-bd-name">' + esc(MODEL_LABELS[item.name] || item.name) + '</span>' +
            '<span class="aigc-bd-pct">' + p.toFixed(1) + '%</span>' +
            '<div class="aigc-bd-track"><div class="aigc-bd-fill" style="width:' +
            Math.min(p, 100).toFixed(1) + '%"></div></div>';
          sbBreakdown.appendChild(row);
        });
        if (sbModels) sbModels.style.display = "flex";
      }
    }

    /* ── Render highlighted output ── */
    function renderOutput(chunks) {
      output.innerHTML = "";
      if (!chunks || !chunks.length) return;
      chunks.forEach(function (chunk) {
        var span = document.createElement("span");
        span.appendChild(document.createTextNode(chunk.text));
        if (chunk.isAi) {
          var sev = Math.min(chunk.severity || 2, 4);
          span.className = "aigc-chunk aigc-chunk--sev" + sev;
          if (chunk.models && chunk.models.length) {
            span.title = chunk.models
              .map(function (m) { return MODEL_LABELS[m] || m; })
              .join(", ");
          }
        }
        output.appendChild(span);
      });
    }
  }

  /* ── Helpers ──────────────────────────────────────────────────── */

  function fmt(n) { return n != null ? n.toFixed(1) : "0.0"; }

  function esc(s) {
    return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
  }

  function readRawUrlFromQuery() {
    var params = new URLSearchParams(location.search);
    var rawUrl = params.get("url");
    if (!rawUrl) return "";
    if (!isAllowedRawUrl(rawUrl)) {
      throw new Error("仅支持当前仓库的 GitHub Raw Markdown 链接");
    }
    return rawUrl;
  }

  function isAllowedRawUrl(rawUrl) {
    if (!rawUrl) return false;
    if (rawUrl.slice(0, RAW_PREFIX.length) !== RAW_PREFIX) return false;
    return /\.md(?:[#?].*)?$/i.test(rawUrl);
  }

  /* ── Material instant-nav ─────────────────────────────────────── */

  if (typeof document$ !== "undefined") {
    document$.subscribe(function () {
      initPage();
    });
  } else if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", function () {
      initPage();
    });
  } else {
    initPage();
  }
})();
