export function normalizeMarkdownForAigc(text) {
  if (!text) return "";

  text = stripFrontMatter(text);
  text = text.replace("<!-- more -->", "\n");
  text = text.replace(/^```[\s\S]*?^```[ \t]*\n?/gm, "\n");
  text = text.replace(/^~~~[\s\S]*?^~~~[ \t]*\n?/gm, "\n");
  text = text.replace(/^ {4,}.*(?:\n {4,}.*)*/gm, "\n");
  text = text.replace(/<script[\s\S]*?<\/script>/gm, " ");
  text = text.replace(/<style[\s\S]*?<\/style>/gm, " ");
  text = text.replace(/<!--[\s\S]*?-->/g, " ");
  text = text.replace(/!\[([^\]]*)\]\([^)]+\)/g, "$1");
  text = text.replace(/\[([^\]]+)\]\([^)]+\)/g, "$1");
  text = text.replace(/`([^`]+)`/g, "$1");
  text = text.replace(/[*_>#~\-]{1,}/g, " ");
  text = text.replace(/\{[^{}]*\}/g, " ");
  text = text.replace(/\n{2,}/g, "\n");
  text = text.replace(/[ \t]{2,}/g, " ");
  return text.trim();
}

function stripFrontMatter(text) {
  if (!text.startsWith("---\n")) return text;
  const marker = "\n---\n";
  const end = text.indexOf(marker, 4);
  return end === -1 ? text : text.slice(end + marker.length);
}
