import fs from "node:fs/promises";
import { initSync, Detector } from "../docs/assets/wasm/ai_text_detector_wasm.js";
import { normalizeMarkdownForAigc } from "../docs/javascripts/aigc-shared.mjs";

const decoder = new TextDecoder("utf-8");

async function main() {
  const input = decoder.decode(await readAllStdin());
  const jobs = JSON.parse(input);

  const wasmBytes = await fs.readFile(new URL("../docs/assets/wasm/ai_text_detector_wasm_bg.wasm", import.meta.url));
  const modelBytes = await fs.readFile(new URL("../docs/assets/models.bin", import.meta.url));

  initSync({ module: wasmBytes });

  const detector = new Detector();
  detector.loadModelsFromBytes(new Uint8Array(modelBytes));

  const result = {};
  for (const [key, rawText] of Object.entries(jobs)) {
    const text = normalizeMarkdownForAigc(rawText);
    if (!text || text.trim().length < 10) {
      result[key] = null;
      continue;
    }

    const report = detector.analyzeText(text);
    result[key] = {
      ai_rate: roundRate(report.weightedPercent),
      verdict: report.verdict ?? null,
      char_rate: roundRate(report.charPercent),
      sentence_rate: roundRate(report.sentencePercent),
    };
  }

  process.stdout.write(`${JSON.stringify(result)}\n`);
}

function roundRate(value) {
  if (typeof value !== "number" || Number.isNaN(value)) return null;
  return Math.round(value * 10) / 10;
}

async function readAllStdin() {
  const chunks = [];
  for await (const chunk of process.stdin) {
    chunks.push(chunk);
  }
  return Buffer.concat(chunks);
}

main().catch((error) => {
  process.stderr.write(`${error.stack || error}\n`);
  process.exitCode = 1;
});
