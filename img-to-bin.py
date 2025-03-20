from PIL import Image
import numpy as np
from itertools import groupby

def png_to_indexed_array(image_path):
    img = Image.open(image_path)
    img = img.resize((320, 200))
    img = img.convert('RGB')
    result = img.quantize(colors=256, method=2)

    palette = result.getpalette()
    #result.show()
    pixels = np.array(result)

    new_pal = []

    for i in range(0, len(palette), 3):
        new_pal.append(f"0x{palette[i]:02x}{palette[i+1]:02x}{palette[i+2]:02x}")

    with open("indexed_array.txt", 'w') as f:
        f.write('static const char image[] = {\n')
        for i, value in enumerate(pixels.flatten()):
            if i != 0:
                f.write(', ')
            f.write(f'0x{value:02X}')
        f.write('\n};\n')
    
    with open("compress.txt", 'w') as f:
        f.write('static const int compressed_image[] = {\n')
        start = 1 # Avoid first comma
        cur = 0   # Value currently
        count = 0 # Amount of this value
        for value in pixels.flatten():
            if value == cur:
                count += 1
            else:
                if start == 0:
                    f.write(', ')
                start = 0
                f.write(f'{count}')  # Write the previous count
                cur = value
                count = 1  # Reset count to 1, not 0
        f.write('\n};\n')
        

    with open("pallete.txt", 'w') as f:
        f.write('static const uint32_t PALETTE[256] = {\n')
        for i, v in enumerate(new_pal):
            if i != 0:
                f.write(', ')
            f.write(v)
        f.write('\n};\n')



png_to_indexed_array('input_image.png')
