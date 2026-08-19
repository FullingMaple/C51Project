# -*- coding: utf-8 -*-
"""更新两份开发指南的内存监控节（新构建数据 + 提高上限）（UTF-8/CRLF 安全）"""

def patch(path, code_line, code_note, xdata_note):
    txt = open(path, 'rb').read().decode('utf-8').replace('\r\n', '\n')
    old = '''每次构建看 `Program Size`：

```
Program Size: data=15.5  xdata=3808  const=6528  code=42760
```

- `code`：Flash 程序 + 常量。当前 42760（48.1KB/64KB），**'''
    assert txt.count(old) == 1, path + ' anchor'
    new = '''每次构建看 `Program Size`：

```
Program Size: data=15.5  xdata=3892  const=6939  code=39767
```

- `code`：Flash 程序 + 常量。当前 39767（+const 6939 = 45.6KB/64KB，剩 ~18.4KB），**'''
    txt = txt.replace(old, new, 1)
    # 完成预计行
    import re
    txt = re.sub(r'当前 42760（48\.1KB/64KB），\*\*测温功能完成后应在 \d+ 以内\*\*（预计 \+1\.7KB 左右）',
                 '当前 39767（45.6KB/64KB），**测温功能完成后应在 41500 以内**（预计 +1.7KB 左右）', txt)
    txt = re.sub(r'当前 42760（48\.1KB/64KB）。\*\*计算器完成后应在 \d+ 以内\*\*（预计 \+1\.7KB 左右；表达式/映射表都是小数据）',
                 '当前 39767（45.6KB/64KB）。**计算器完成后应在 41500 以内**（预计 +1.7KB 左右；表达式/映射表都是小数据）', txt)
    # xdata 行
    txt = re.sub(r'- `xdata`：RAM。当前 3808/8192，测温页只加几个全局变量（<50B）',
                 '- `xdata`：RAM。当前 3892/8192，测温页只加几个全局变量（<50B）', txt)
    txt = re.sub(r'- `xdata`：当前 3808/8192。计算器页约 \+30B（表达式缓冲+状态）',
                 '- `xdata`：当前 3892/8192。计算器页约 +30B（表达式缓冲+状态）', txt)
    # 超限行提高
    txt = re.sub(r'- 超限（code > 63000 或 xdata > 8100）→ 立即告诉主开发',
                 '- 超限（code > 64000 或 xdata > 8100）→ 立即告诉主开发（预算已大幅放宽，但仍需留余量）', txt)
    out = txt.replace('\n', '\r\n').encode('utf-8')
    open(path, 'wb').write(out)
    print(path, 'updated')

patch('功能开发指南-NTC测温.md', '', '', '')
patch('功能开发指南-计算器.md', '', '', '')
