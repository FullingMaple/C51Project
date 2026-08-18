# -*- coding: utf-8 -*-
"""
add_fonts.py —— 给 OLED_Fonts.c 的 12x12 字库补字模（GBK 编码，Windows）
用法：python add_fonts.py 仪 度 当 前
原理：SimSun 内嵌点阵 1:1 渲染 → 列式字模（先上半页再下半页）→ 插入表头
依赖：PIL（pip install pillow）
注意：只操作 OLED_Fonts.c 的 OLED_CF12x12 表；改前请 git 备份
"""
import sys, io, re, os
from PIL import Image, ImageDraw, ImageFont, ImageOps

FONTS_FILE = os.path.join(os.path.dirname(__file__), 'src',
                          'ui', 'Driver', 'Software_Driver', 'OLED_Fonts.c')
SIMSUM = r'C:\Windows\Fonts\simsun.ttc'
SIZE = 12                      # 12x12 汉字

def render_char(ch):
    """渲染单个汉字 → 12x12 二值位图（SimSun 1:1，不缩放）"""
    font = ImageFont.truetype(SIMSUM, SIZE)
    # 大画布渲染，避免 bbox 裁切
    img = Image.new('L', (SIZE * 4, SIZE * 4), 255)
    d = ImageDraw.Draw(img)
    d.text((SIZE, SIZE), ch, font=font, fill=0)
    # 取字形 bbox（反色后 getbbox 只检测文字像素），居中到 12x12 画布
    bbox = ImageOps.invert(img).getbbox()
    if not bbox:
        raise RuntimeError('渲染失败: ' + ch)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    if w > SIZE or h > SIZE:
        raise RuntimeError('字形超 12x12: ' + ch + ' (%dx%d)' % (w, h))
    ox = (SIZE - w) // 2
    oy = (SIZE - h) // 2
    glyph = img.crop(bbox)
    canvas = Image.new('L', (SIZE, SIZE), 255)
    canvas.paste(glyph, (ox, oy))
    return canvas

def to_column_data(img):
    """12x12 位图 → 24 字节列式字模（先 12 列上半，再 12 列下半）"""
    px = img.load()
    data = []
    for half in (0, 1):                       # half=0 上半(行0-7)，half=1 下半(行8-11)
        for x in range(SIZE):                 # 12 列
            byte = 0
            for r in range(8):                # 每列 8 行（下半只有 4 行有效，其余 0）
                y = half * 8 + r
                if y < SIZE and px[x, y] < 128:
                    byte |= 1 << r            # bit0 在顶
            data.append(byte)
    return data

def gbk_bytes(s):
    return s.encode('gbk')

def make_entry(ch):
    data = to_column_data(render_char(ch))
    hexstr = ','.join('0x%02X' % b for b in data)
    return '{{"%s"},\n{%s}},' % (ch, hexstr)

def main():
    chars = sys.argv[1:]
    if not chars:
        print('用法: python add_fonts.py 字1 字2 ...')
        return 1
    # 检查重复（只在 OLED_CF12x12 表体内搜索，避免 16x16 同名误判）
    raw = open(FONTS_FILE, 'rb').read()
    txt = raw.decode('gbk')
    m = re.search(r'OLED_CF12x12\[\] = \{(.*?)\};', txt, re.S)
    body = m.group(1) if m else txt
    chars = [ch for ch in chars if ('{{"%s"},' % ch) not in body]
    if not chars:
        print('全部已在 12x12 字库，跳过')
        return 0
    # 生成条目
    entries = []
    for ch in chars:
        entries.append(make_entry(ch))
        print('已生成: ' + ch)
    block = '\r\n'.join(entries) + '\r\n'
    # 插入 OLED_CF12x12 表头（替换式，保持 GBK/CRLF）
    anchor = 'OLED_CF12x12[] = {' + '\r\n'
    assert txt.count(anchor) == 1, '找不到 OLED_CF12x12 表头（文件编码/格式异常，请检查）'
    txt = txt.replace(anchor, anchor + block, 1)
    open(FONTS_FILE, 'wb').write(txt.encode('gbk'))
    print('完成：%d 个字已插入 OLED_CF12x12' % len(entries))
    return 0

if __name__ == '__main__':
    sys.exit(main())
