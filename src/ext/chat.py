"""
对话气泡 Markdown 扩展
"""

import re
from markdown.extensions import Extension
from markdown.preprocessors import Preprocessor


class ChatExtension(Extension):
    """对话气泡扩展"""

    def extendMarkdown(self, md):
        """注册扩展"""
        md.preprocessors.register(ChatPreprocessor(md), 'chat', 100)


class ChatPreprocessor(Preprocessor):
    """预处理器：将简单的对话语法转换为 HTML"""

    def run(self, lines):
        new_lines = []
        in_chat_block = False
        chat_lines = []
        
        for line in lines:
            # 检查对话块开始
            if line.strip() == '```chat':
                in_chat_block = True
                chat_lines = []
                continue
            
            # 检查对话块结束
            if line.strip() == '```' and in_chat_block:
                in_chat_block = False
                # 处理对话块
                processed = self.process_chat_block(chat_lines)
                new_lines.append(processed)
                continue
            
            # 收集对话行
            if in_chat_block:
                chat_lines.append(line)
                continue
            
            # 非对话行保持原样
            new_lines.append(line)
        
        return new_lines
    
    def process_chat_block(self, lines):
        """处理对话块"""
        html_lines = ['<div class="chat-container">']
        
        for line in lines:
            # 跳过空行
            if not line.strip():
                continue
                
            # 解析消息行：[位置] [图片URL] 消息内容
            # 例如：[left] [path/to/image.png] 你好
            match = re.match(r'\[(left|right)\]\s*\[([^\]]+)\]\s*(.*)', line.strip())
            if match:
                position = match.group(1)
                image_url = match.group(2)
                content = match.group(3)
                
                # 转义 HTML 特殊字符
                content = content.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
                
                # 构建头像 HTML（图片标签）
                avatar_html = f'<img src="{image_url}" class="chat-avatar" alt="Avatar">'
                
                # 构建 HTML
                html = f'''  <div class="chat-message chat-{position}">
                    {avatar_html}
                    <div class="chat-bubble">
                    {content}
                    </div>
                </div>'''
                html_lines.append(html)
        
        html_lines.append('</div>')
        return ''.join(html_lines)


def makeExtension(**kwargs):
    return ChatExtension(**kwargs)