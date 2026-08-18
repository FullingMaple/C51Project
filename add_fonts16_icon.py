# -*- coding: utf-8 -*-
"""
add_fonts16_icon.py —— 一次性脚本：
  1. 给 OLED_CF16x16 补 16x16 字模（日 历）
  2. 给图标区补 32x32 日历图标 Image_calendar（列主序 4 页 x 32 列）
  3. OLED_Fonts.h 补 extern 声明
GBK/CRLF 安全（rb/wb 同编码）。用法：python add_fonts16_icon.py
"""
import os, re
from PIL import Image, ImageDraw, ImageFont, ImageOps

BASE = os.path.join(os.path.dirname(__file__), 'src', 'ui', 'Driver', 'Software_Driver')
FONTS = os.path.join(BASE, 'OLED_Fonts.c')
FONTS_H = os.path.join(BASE, 'OLED_Fonts.h')
SIMSUM = r'C:\Windows\Fonts\simsun.ttc'

def render16(ch):
    font = ImageFont.truetype(SIMSUM, 16)
    img = Image.new('L', (64, 64), 255)
    d = ImageDraw.Draw(img)
    d.text((16, 16), ch, font=font, fill=0)
    bbox = ImageOps.invert(img).getbbox()
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    assert w <= 16 and h <= 16, (ch, w, h)
    canvas = Image.new('L', (16, 16), 255)
    canvas.paste(img.crop(bbox), ((16 - w) // 2, (16 - h) // 2))
    px = canvas.load()
    data = []
    for half in (0, 1):                      # 上 16 列 + 下 16 列（bit0 在顶）
        for x in range(16):
            byte = 0
            for r in range(8):
                y = half * 8 + r
                if y < 16 and px[x, y] < 128:
                    byte |= 1 << r
            data.append(byte)
    return data

def make_cell16(ch):
    data = render16(ch)
    return '{{"%s"},\n{%s}},' % (ch, ','.join('0x%02X' % b for b in data))

def make_icon():
    img = Image.new('L', (32, 32), 255)
    d = ImageDraw.Draw(img)
    d.rectangle([3, 7, 28, 29], outline=0, width=1)     # 外框
    d.rectangle([3, 3, 28, 8], fill=0)                  # 顶栏（挂历头部）
    d.rectangle([8, 1, 11, 4], fill=0)                  # 左侧挂环
    d.rectangle([20, 1, 23, 4], fill=0)                 # 右侧挂环
    d.rectangle([6, 13, 13, 22], outline=0)             # 日期数字"1"框格
    d.rectangle([14, 13, 25, 22], outline=0)            # 日期数字"8"框格
    d.rectangle([16, 13, 19, 22], fill=0)               # "8"上半
    d.rectangle([18, 15, 23, 18], fill=0)               # "8"中横
    d.rectangle([16, 19, 19, 22], fill=0)               # "8"下半
    px = img.load()
    data = []
    for page in range(4):                                # 4 页 x 32 列（bit0 在顶）
        for x in range(32):
            byte = 0
            for r in range(8):
                y = page * 8 + r
                if y < 32 and px[x, y] < 128:
                    byte |= 1 << r
            data.append(byte)
    return data

def main():
    raw = open(FONTS, 'rb').read()
    txt = raw.decode('gbk')

    # ---- 1. 16x16 补字（日 历）----
    anchor16 = 'OLED_CF16x16[] = {' + '\r\n'
    assert txt.count(anchor16) == 1
    block16 = []
    for ch in ('日', '历'):
        if ('{{"%s"},' % ch) in txt:
            print('16x16 跳过（已有）: ' + ch)
        else:
            block16.append(make_cell16(ch))
            print('16x16 已生成: ' + ch)
    if block16:
        txt = txt.replace(anchor16, anchor16 + '\r\n'.join(block16) + '\r\n', 1)

    # ---- 2. Image_calendar 图标 ----
    if 'Image_calendar[]' in txt:
        print('Image_calendar 已存在，跳过')
    else:
        icon = make_icon()
        entry = ('code uint8_t Image_calendar[] = {\r\n' +
                 ',\r\n'.join(','.join('0x%02X' % b for b in icon[i:i+32]) for i in range(0, 128, 32)) +
                 '\r\n};')
        # 插到 Image_serial 之前（图标区起始）
        anchor_icon = 'code uint8_t Image_serial[]'
        assert txt.count(anchor_icon) == 1
        txt = txt.replace(anchor_icon, entry + '\r\n' + anchor_icon, 1)
        print('Image_calendar 已生成（128 字节）')

    open(FONTS, 'wb').write(txt.encode('gbk'))

    # ---- 3. OLED_Fonts.h extern ----
    raw_h = open(FONTS_H, 'rb').read()
    txt_h = raw_h.decode('gbk')
    if 'Image_calendar' not in txt_h:
        txt_h = txt_h.replace('extern code uint8_t Image_serial[];',
                              'extern code uint8_t Image_calendar[];\r\nextern code uint8_t Image_serial[];', 1)
        open(FONTS_H, 'wb').write(txt_h.encode('gbk'))
        print('OLED_Fonts.h 已补 extern')
    print('完成')
    return 0

if __name__ == '__main__':
    exit(main())
