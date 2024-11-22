# Quick-and-dirty Markdown (.md) to SymbOS Help (.hlp) converter. Supports a
# subset of standard Markdown syntax (listed below). Note that links are not
# currently supported. Due to how SymbOS Help is designed, bold will generally
# look better than italics, which will typically be converted to uppercase.
#
#   # Main section start
#   ## Main section start
#   ### Subsection start
#   #### Subheading within section
#   *italic*
#   **bold**
#   ***bold italic***
#   `code`
#   ```codeblock
#   | Table | Table |
#
# Other tag types (e.g., lists) will look OK just because raw Markdown is
# designed to be readable.

import sys
import os
import re
from unidecode import unidecode

char_width = [0, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0, 5, 5, 0, 6, 5, 5, 5, 5, 5, 6, 5, 6, 6, 6, 6, 5, 5, 6, 1, 7, 7, 3, 2, 4, 6, 4, 5, 5, 2, 3, 3, 6, 6, 3, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 3, 4, 5, 4, 5, \
              5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 5, 5, 5, 6, 6, 5, 5, 5, 5, 5, 6, 5, 6, 6, 6, 6, 5, 3, 5, 3, 4, 6, 3, 5, 5, 5, 5, 5, 3, 5, 5, 2, 3, 5, 3, 6, 5, 5, 5, 5, 4, 4, 3, 5, 6, 6, 5, 5, 5, 4, 2, 4, 6, 3, \
              6, 8, 5, 6, 6, 3, 4, 4, 7, 7, 4, 7, 3, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 3, 4, 5, 7, 5, 6, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6, 6, 6, 7, 7, 6, 6, 7, 6, 6, 7, 6, 7, 7, 7, 7, 6, 4, 6, 4, 5, 8, 4, 6, \
              6, 6, 6, 6, 4, 6, 6, 3, 4, 6, 4, 8, 6, 6, 6, 6, 5, 6, 4, 6, 7, 8, 6, 6, 7, 5, 3, 5, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6, 6, 6, 7, 7, 6, 6, 7, 6, 6, 7, 6, 7, 7, 7, 7, 6]

if len(sys.argv) != 2 or sys.argv[1].lower()[-3:] != ".md":
    print("usage: python md2hlp.py [filename.md]")
    sys.exit()

def word16(value):
    result = bytearray()
    result.append(value & 255)
    result.append(value >> 8)
    return result
    
def bold(bstring):
    return bytearray([(45 if x == 45 else (61 if x == 61 else (95 if x == 95 else (133 if x == 96 else (x + 94 if x >= 33 and x <= 125 else x))))) for x in bstring])
    
def italic(bstring):
    return bytearray([(x + 165 if x >= 65 and x <= 90 else (x + 133 if x >= 97 and x <= 122 else (x + 172 if x >= 48 and x <= 57 else x))) for x in bstring])
    
def underline(bstring):
    return bytearray([(190 if x == 32 else (27 if x == 74 else (28 if x == 77 else (x - 64 if x >= 65 and x <= 90 else x)))) for x in bstring])

def format_text(bstring):
    global char_width
    font_terminators = [10, 13, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 58, 59, 60, 61, 62, 63, 64, 91, 92, 93, 94, 95, 123, 124, 125, 126]
    sstring = bstring.decode('ascii')
    sstring = re.sub(r'\[(.*?)\]\(.*?\)', r'\1', sstring) # convert links to just the label
    bstring = bytes(sstring, 'ascii')
    outstring = bytearray()
    font = [0]
    index = -1
    tab_id = 0
    tab_widths = [0]
    tab_pos = 0
    pixel = 0
    code_mode = 0
    while index < len(bstring) - 1:
        index += 1
        byte = bstring[index]
        if byte == 96: # `
            if font[-1] == 4:
                font = font[:-1]
                code_mode = 0
            else:
                font.append(4)
                code_mode = 1
            continue
        if byte == 9:
            outstring.append(32)
            outstring.append(32)
            outstring.append(32)
            outstring.append(32)
            continue
        if byte == 42 and code_mode == 0: # *
            if (bstring[index-1] == 13 or bstring[index-1] == 10 or bstring[index-1] == 32) and bstring[index+1] != 32 and bstring[index+1] != 42:
                font.append(2)
                continue
            if (bstring[index-1] == 13 or bstring[index-1] == 10 or bstring[index-1] == 32) and bstring[index+1] == 42 and bstring[index+2] != 32 and bstring[index+2] != 42:
                font.append(1)
                index += 1
                continue
            if (bstring[index-1] == 13 or bstring[index-1] == 10 or bstring[index-1] == 32) and bstring[index+1] == 42 and bstring[index+2] == 42 and bstring[index+3] != 32 and bstring[index+3] != 42:
                font.append(1)
                index += 2
                continue
            if font[-1] == 2 and bstring[index-1] != 32 and bstring[index-1] != 13 and bstring[index-1] != 10 and bstring[index+1] in font_terminators:
                font = font[:-1]
                continue
            if font[-1] == 1 and bstring[index-1] != 32 and bstring[index-1] != 13 and bstring[index-1] != 10 and bstring[index+1] == 42 and bstring[index+2] in font_terminators:
                font = font[:-1]
                index += 1
                continue
            if font[-1] == 1 and bstring[index-1] != 32 and bstring[index-1] != 13 and bstring[index-1] != 10 and bstring[index+1] == 42 and bstring[index+2] == 42 and bstring[index+3] in font_terminators:
                font = font[:-1]
                index += 2
                continue
        if byte == 124: # |
            if bstring[index-1] == 13 or bstring[index-1] == 10:
                if tab_id <= 1:
                    tab_widths = [0]
                tab_id = 1
            elif tab_id > 0:
                if tab_id >= len(tab_widths):
                    outstring.append(32)
                    outstring.append(32)
                    outstring.append(32)
                    pixel += 9
                    tab_widths.append(pixel)
                else:
                    while pixel + 8 <= tab_widths[tab_id]:
                        outstring.append(189)
                        pixel += 8
                    while pixel + 3 <= tab_widths[tab_id]:
                        outstring.append(32)
                        pixel += 3
                    while pixel + 1 <= tab_widths[tab_id]:
                        outstring.append(29)
                        pixel += 1
                tab_id += 1
            continue
        if (byte == 32 or byte == 9) and tab_id > 0 and ((tab_id < len(tab_widths) and (outstring[-1] == 32 or outstring[-1] == 9)) or bstring[index-1] == 124):
            continue
        if byte == 13 or byte == 10:
            pixel = 0
        if font[-1] == 1 or font[-1] == 4:
            byte = bold([byte])[0]
        elif font[-1] == 2:
            byte = italic([byte])[0]
        elif font[-1] == 3:
            byte = underline([byte])[0]
        pixel += char_width[byte]
        outstring.append(byte)
    return outstring
    
with open(sys.argv[1], mode="r", encoding="utf-8") as file:
    lines = file.readlines()
    
lines = [unidecode(line).replace("\n", "") for line in lines]

chdata = {}
chapter = ""

id = 0
split_count = 1
for line in lines:
    if line[0:2] == "# ":
        id = id + 1
        chapter = str(id) + " " + line[2:]
        split_count = 1
    elif line[0:3] == "## ":
        id = id + 1
        chapter = str(id) + " " + line[3:]
        split_count = 1
    elif line[0:4] == "### ":
        id = id + 1
        chapter = str(id) + " " + "  " + line[4:]
        split_count = 1
    else:
        if chapter not in chdata:
            chdata[chapter] = b""
        if line[0:5] == "#### ":
            line = "**" + line[5:].trim() + "**\r\n"
        if line[0:3] == "```":
            while len(chdata[chapter]) > 0 and (chdata[chapter][-1] == 13 or chdata[chapter][-1] == 10):
                chdata[chapter] = chdata[chapter][:-1]
            chdata[chapter] += b"\r\n`____________\r\n\r\n"
        else:
            if len(chdata[chapter]) + len(line) > 7936:
                if split_count == 1:
                    chdata[chapter + " (" + str(split_count) + ")"] = chdata[chapter]
                    del chdata[chapter]
                elif split_count < 10:
                    chapter = chapter[:-4]
                else:
                    chapter = chapter[:-5]
                if chapter[0:2] != "  ":
                    chapter = "  " + chapter
                split_count += 1
                chapter = chapter + " (" + str(split_count) + ")"
                if chapter not in chdata:
                    chdata[chapter] = b"\r\n"
            chdata[chapter] += bytes(line, 'ascii') + b"\r\n"

if len(chdata.keys()) > 128:
    print("Too many sections")
    sys.exit()
        
with open(sys.argv[1][:-3] + ".hlp", mode="wb") as file:
    chtitles = b""
    listdata = b""
    data = b""
    for chapter in chdata:
        if len(chapter) > 0:
            chname = chapter[chapter.find(" ")+1:]
            oldlen = len(data)
            data += b'\x00' # font = multi
            data += b'\x00' # number of links = 0
            data += b'\x0D\x0A' + bold(bytes(chname.strip().replace("`", '"').upper(), 'ascii')) + b'\x0D\x0A'
            data += format_text(chdata[chapter]).replace(b"\r\n\r\n\r\n", b"\r\n\r\n").replace(b"\r\n____________\r\n", underline(b"\r\n            \r\n")) + b'\x00'
            listdata += word16(len(data) - oldlen + (0 if chname[:2] == "  " else 8192))
            listdata += word16(len(chtitles))
            chtitles += bytes(chname.replace("`", '"'), 'ascii')[:24] + b'\x00'
    file.write(b"SYMHLP10")
    file.write(word16(len(listdata)))
    file.write(word16(len(chtitles)))
    file.write(listdata)
    file.write(chtitles)
    file.write(data)
