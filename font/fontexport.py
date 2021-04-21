#!/usr/bin/python

#Link me to python plugins dir
#ex: `ln -s fontexport.py ~/snap/gimp/current/.config/GIMP/2.10/plug-ins/fontexport.py`

# `.vscode/settings.json` example:
# {
#     "python.pythonPath": "/usr/bin/python2",
#     "python.envFile": "${workspaceFolder}/.vscode/.env"
# }
#
# `.vscode/.env` example:
# PYTHONPATH="/snap/gimp/current/usr/bin/python"

# Renders characters using the configured values, outputs a C header with the integer representation and png preview.
# Each char is represented by an array of bit-masks representing each row in the char render either in 0b or hex literals.

import math
from gimpfu import *
import binascii
import os
import math

font_name = "Ubuntu Mono, Bold"
font_size = 16
char_start = ' '
char_end = '~'


# This only affects how the preview png is generated.
col_count = 16

col_width = font_size/2
row_height = font_size

# Write to header with 0b integer rather than hex.
write_binary_ints = True

base_file_name = "ata_monobold{0}".format(row_height)

img_out_path = os.path.abspath("{0}.png".format(base_file_name))
header_out_path = os.path.abspath("{0}.h".format(base_file_name))

white_color = gimpcolor.RGB(255, 255, 255)
black_color = gimpcolor.RGB(0, 0, 0)

def char_range(start, end):
    for c in range(ord(start), ord(end)+1):
        yield(chr(c))

def layer_to_introws(layer):
    rgn = layer.get_pixel_rgn(0, 0, layer.width, layer.height)

    rows = []

    for y in range(0, layer.height):
        row_val = 0
        for x in range(0, layer.width):
            pixel = bytearray(rgn[x, y])
            if pixel[0] > 0:
                row_val |= 1<<x
            
        rows.append(row_val)

    return rows

def write_ints_to_c_header(int_rows):
    f = open(header_out_path, "w")

    f.write("#pragma once\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write("// Generated by fontexport.py plugin for Gimp\n")

    f.write("#define {0}_STARTCHAR '{1}'\n".format(base_file_name.upper(), char_start))
    f.write("#define {0}_ENDCHAR '{1}'\n".format(base_file_name.upper(), char_end))
    f.write("#define {0}_HEIGHT {1}\n".format(base_file_name.upper(), row_height))
    f.write("#define {0}_WIDTH {1}\n".format(base_file_name.upper(), col_width))
    f.write("\n")

    f.write("// Each character is represented by {0} rows of integers representing the bit mask for each rows.\n".format(row_height))
    f.write(("uint32_t {0}[ ][{1}] = {{\n").format(base_file_name, row_height))

    for row_name in int_rows:
        f.write("    //char '{0}'\n".format(row_name))
        
        if write_binary_ints:
            f.write("    {\n")
            f.write(",\n".join(map(lambda int_val: "        0b{0:0{1}b}".format(int_val, col_width), int_rows[row_name])))
            f.write("\n    },\n")
        else:
            f.write("    {")
            f.write(", ".join(map(lambda int_val: hex(int_val), int_rows[row_name])))
            f.write("}\n")

    f.write("};")
    f.close()

    print("Saved to {0}".format(header_out_path))

def render_char(char, img):
    text_layer = pdb.gimp_text_fontname(img, None, 0, 0, "\xC2\xb0", 0, False, font_size, PIXELS, font_name)

    dx = col_width - text_layer.width
    dy = row_height - text_layer.height

    text_layer.resize(col_width, row_height, dx/2, dy/2)

    pdb.gimp_layer_set_offsets(text_layer, 0, 0)

    return text_layer

def init_background_img():
    char_count = sum(1 for _ in char_range(char_start, char_end))
    row_count = math.ceil(float(char_count) / col_count)
    img_width = col_width * col_count
    img_height = row_height * row_count

    img=pdb.gimp_image_new(img_width, img_height, RGB)
    pdb.gimp_context_set_foreground(black_color)
    layer=pdb.gimp_layer_new(img, img_width, img_height, RGB, "base", 100, NORMAL_MODE)
    pdb.gimp_image_add_layer(img, layer, 0)

    return img

def run_export():
    int_rows = {}
    img = init_background_img()
    char_idx = 0

    pdb.gimp_context_set_foreground(white_color)

    for c in char_range(char_start, char_end):
        text_layer = render_char(c, img)
        
        int_rows[c] = layer_to_introws(text_layer)
        
        col_idx = char_idx % col_count
        row_idx = char_idx / col_count

        pdb.gimp_layer_set_offsets(text_layer, col_idx * col_width, row_idx * row_height)

        char_idx += 1
        
    write_ints_to_c_header(int_rows)

    combined = pdb.gimp_image_merge_visible_layers(img, CLIP_TO_IMAGE)
    pdb.gimp_file_save(img, combined, img_out_path, '?')

    pdb.gimp_display_new(img)

    gimp.message("Saved to {0}".format(header_out_path))

register(
    "Ata_Export_Font",
    "Render a font as a C header",
    "Renders the specified character range in gimp, exports a C header representation and a png preview.",
    "Alex Raboud",
    "Alex Raboud",
    "2021",
    "<Toolbox>/ATA Export Font",
    "",
    [],
    [],
    run_export
)

main()