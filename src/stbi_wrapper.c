#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int SAVE_RGB(char const *filename, int w, int h, const void *data) { return stbi_write_png(filename, w, h, 3, data, w*3); }

int SAVE_RGBA(char const *filename, int w, int h, const void *data) { 

    stbi_flip_vertically_on_write(1);
    return stbi_write_png(filename, w, h, 4, data, w*4);
}
