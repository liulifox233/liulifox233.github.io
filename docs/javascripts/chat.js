// 对话气泡功能
function initializeChat() {
  // 查找所有对话容器
  const chatContainers = document.querySelectorAll('.chat-container');
  
  chatContainers.forEach(container => {
    // 为每个对话容器添加必要的类
    container.classList.add('chat-wrapper');
    
    // 处理每个消息
    const messages = container.querySelectorAll('.chat-message');
    messages.forEach(message => {
      // 清除可能已存在的类
      message.classList.remove('chat-message-left', 'chat-message-right');
      
      // 根据消息的 alignment 类设置位置
      if (message.classList.contains('chat-left')) {
        message.classList.add('chat-message-left');
      } else if (message.classList.contains('chat-right')) {
        message.classList.add('chat-message-right');
      }
    });
  });
}

// 页面加载完成后初始化
document.addEventListener("DOMContentLoaded", function() {
  initializeChat();
});

// 兼容 Material for MkDocs 的即时导航功能
if (typeof document$ !== "undefined") {
  document$.subscribe(() => {
    initializeChat();
  });
}