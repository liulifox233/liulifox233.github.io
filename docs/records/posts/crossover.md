---
date:
  created: 2024-11-24 17:06:33
  updated: 2025-01-07 07:22:54
categories:
  - Tech
authors:
  - ruri
---

# CrossOver Crack Record


!!!TIP
    笔者使用的版本为 24.0.5

CrossOver 在启动时会检查本地许可证，所以我们只需逐个更新许可证即可完成 unlimited trial crack。
<!-- more -->
许可证文件:

- `~/Library/Application Support/CrossOver/Bottles/*/system.reg`

    - 删除这个 wine 的注册表文件下的[Software\\CodeWeavers\\CrossOver\\cxoffice]字段

- `~/Library/Application Support/CrossOver/Bottles/*/.update-timestamp`

    - 直接删除这个文件

- `~/Library/Preferences/com.codeweavers.CrossOver.plist`

    - 更新`FirstRunDate`和`SULastCheckTime`字段

可以设置定时脚本自动处理，此处不提供。