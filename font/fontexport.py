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

import math
from gimpfu import *
import binascii
import os

def run_export(timg, tdrawable):
    # Expecting an image 7x8 with a layer for each character.
    col_skip = 1
    col_count = 5
    row_count = 8
    out_path = os.path.abspath("ata_monospace.h")
    
    print("Saving to {0}\n".format(out_path))

    f = open(out_path, "w")

    f.write("#pragma once\n\n")
    f.write("// Generated by fontexport.py plugin for Gimp\n")
    f.write("// Each character is represented by {0} columns of 8 bit int's representing the bit mask for rows with pixels.\n".format(col_count))
    f.write(("unsigned char ata_monospace[ ][{0}] = {{\n").format(col_count))

    layers = timg.layers
    layers.sort(key=lambda l: l.name)

    layer_count=0
    for layer in layers:
        if len(layer.name) > 1:
            continue

        if layer_count > 0:
            f.write(",\n")

        layer_count += 1


        pixels = layer.get_pixel_rgn(col_skip, 0, col_count, row_count, False)

        print("Layer: {0}\n".format(layer.name))
        f.write("    // {0}\n".format(layer.name))
        f.write("    {")


        for i in range(col_skip, col_count+col_skip):
            col_val = 0
            for j in range(0, row_count):
                pixel = bytearray(pixels[i, j])
                if pixel[3] > 0:
                    col_val |= (1<<j)

            f.write("0x{:02x}".format(col_val))
            if i<col_count+col_skip-1:
                f.write(", ")

        f.write("}")

    f.write("\n};\n")
    f.close()

    gimp.message("Saved to {0}".format(out_path))


register(
    "Ata_Export_Font",
    "Export the font layers as byte-code",
    "Export the font layers as byte-code",
    "Alex Raboud",
    "Alex Raboud",
    "2021",
    "<Image>/Filters/ATA Export Font",
    "RGB*, GRAY*",
    [],
    [],
    run_export
)

main()