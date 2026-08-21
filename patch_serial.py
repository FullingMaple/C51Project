# -*- coding: utf-8 -*-
"""串口功能补全（临时脚本，用完即删）
1. OLED_UI.c：串口页加入 IsStaticIdle 特判——否则数据到达不刷新（关键 bug）
2. MenuData.c：Ser_PutChar 识别 GBK 双字节中文 → 1 位 '?' 占位（PC 发中文不乱码）
"""
import sys

def rep(f, old, new, tag):
    """字节替换，兼容 LF/CRLF 行尾"""
    b = open(f, 'rb').read()
    for eol in (b'\n', b'\r\n'):
        o = old.replace(b'\n', eol)
        if b.count(o) == 1:
            open(f, 'wb').write(b.replace(o, new.replace(b'\n', eol)))
            print(f'OK  {tag} (LF={eol==b"\\n"})')
            return
    hits = [b.count(old.replace(b'\n', e)) for e in (b'\n', b'\r\n')]
    raise AssertionError(f'{tag}: anchor hits {hits}')

# ---------- 1. OLED_UI.c ----------
b = open('src/ui/OLED_UI/OLED_UI.c', 'rb').read()
anchor = b'extern MenuPage ClockMenuPage;'
assert b.count(anchor) == 1, f'clock extern hits={b.count(anchor)}'
add = anchor + b'\r\n' if b'\r\n' in b[b.find(anchor):b.find(anchor)+20] else anchor + b'\n'
# 保持原行尾：找到该行结尾
end = b.find(b'\n', b.find(anchor))
line = b[b.find(anchor):end]
eol = b'\r\n' if line.endswith(b'\r') else b'\n'
if line.endswith(b'\r'): line = line[:-1]
new_line = line + eol + 'extern MenuPage SerialMenuPage;   /* 串口页：数据到达需实时刷新 */'.encode('gbk') + eol
b = b[:b.find(anchor)] + new_line + b[end+1:]
open('src/ui/OLED_UI/OLED_UI.c', 'wb').write(b)
print('OK  extern SerialMenuPage added')

rep('src/ui/OLED_UI/OLED_UI.c',
    b'if(CurrentMenuPage == &ClockMenuPage) return false;',
    ('if(CurrentMenuPage == &ClockMenuPage) return false;\n'
     '\tif(CurrentMenuPage == &SerialMenuPage) return false;   /* 串口页：新字节到达需实时重绘 */').encode('gbk'),
    'IsStaticIdle serial')

# ---------- 2. MenuData.c ----------
rep('src/ui/OLED_UI/OLED_UI_MenuData.c',
    'static uint8_t Ser_LastKey = 0;  /* 按键去重 */'.encode('gbk'),
    'static uint8_t Ser_LastKey = 0;  /* 按键去重 */\n'
    'static uint8_t Ser_GbkPending = 0;/* GBK 次字节待丢弃标记 */'.encode('gbk'),
    'Ser_GbkPending var')

rep('src/ui/OLED_UI/OLED_UI_MenuData.c',
    b'    Ser_ScrollTop = 0;\n}',
    b'    Ser_ScrollTop = 0;\n    Ser_GbkPending = 0;\n}',
    'Ser_Reset clear')

rep('src/ui/OLED_UI/OLED_UI_MenuData.c',
    b'static void Ser_PutChar(uint8_t ch)\n{\n'
    b'\tif(ch == 0x0D){ return; }',
    ('static void Ser_PutChar(uint8_t ch)\n{\n'
     '    if(Ser_GbkPending){ Ser_GbkPending = 0; return; }   /* GBK 次字节丢弃 */\n'
     '    if(ch >= 0x81){ Ser_GbkPending = 1; ch = (uint8_t)\'?\'; }  /* GBK 首字节：1 位占位 */\n'
     '    if(ch == 0x0D){ return; }').encode('gbk'),
    b'Ser_PutChar GBK')

# ---------- 完整性 ----------
for f, key in [('src/ui/OLED_UI/OLED_UI.c', b'SerialMenuPage'),
               ('src/ui/OLED_UI/OLED_UI_MenuData.c', b'Ser_GbkPending')]:
    d = open(f, 'rb').read()
    assert key in d, f'integrity: {f}'
    d.decode('gbk')
    print(f'OK  integrity {f}')
print('ALL DONE')
