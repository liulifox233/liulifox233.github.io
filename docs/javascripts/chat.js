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
      
      // 初始隐藏消息，准备动画
      message.classList.add('chat-message');
    });
  });
  
  // 添加滚动监听
  setupChatAnimation();
}

// 设置聊天消息动画
function setupChatAnimation() {
  const observerOptions = {
    root: null,
    rootMargin: '0px',
    threshold: 0.3 // 当30%的消息可见时触发
  };
  
  const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
      if (entry.isIntersecting) {
        // 延迟显示消息，创建逐个出现的效果
        const messages = entry.target.querySelectorAll('.chat-message');
        messages.forEach((message, index) => {
          setTimeout(() => {
            message.classList.add('animate');
          }, index * 300); // 每个消息延迟300ms
        });
        
        // 停止观察这个容器
        observer.unobserve(entry.target);
      }
    });
  }, observerOptions);
  
  // 观察所有聊天容器
  document.querySelectorAll('.chat-container').forEach(container => {
    observer.observe(container);
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