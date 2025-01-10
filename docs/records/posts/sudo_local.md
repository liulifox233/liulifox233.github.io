---
date:
  created: 2025-01-10 23:43:22
categories:
  - Tech
authors:
  - ruri
hero: assets/posts/第三の公園.webp
---

# MacOS 使用 Touch ID 验证 sudo 权限

一直很烦每次 sudo 都要输入密码, 闲来无事找了一下有没有用 Touch ID 验证 sudo 权限的方法, 在这里记录一下.
<!-- more -->
=== "/etc/pam.d/sudo"
```fish
# sudo: auth account password session
auth       include        sudo_local
auth       sufficient     pam_smartcard.so
auth       required       pam_opendirectory.so
account    required       pam_permit.so
password   required       pam_deny.so
session    required       pam_permit.so
```

可以看到 /etc/pam.d/sudo 文件导入了名为 sudo_local 的子模块, 但实际上 /etc/pam.d/sudo_local 这个文件是不存在的.

Apple 提供了一个 sudo_local.template 模版:

=== "/etc/pam.d/sudo_local.template"
```sudo_local.template
# sudo_local: local config file which survives system update and is included for sudo
# uncomment following line to enable Touch ID for sudo
#auth       sufficient     pam_tid.so
```

所以实际上要执行的命令很简单:

```fish
sudo cp /etc/pam.d/sudo_local.template /etc/pam.d/sudo_local
```

随后使用 vim 去掉 `auth       sufficient     pam_tid.so` 前面的注释就可以了, pam_tid.so 是 Touch ID 模块.