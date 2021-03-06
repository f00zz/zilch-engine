#include <cerrno>
#include <cstdio>
#include <cstring>

#include <png.h>

#include "image.h"
#include "panic.h"

Image::Image() = default;

bool Image::load(const std::string &path)
{
    FILE *fp;

    if ((fp = fopen(path.c_str(), "rb")) == 0)
        panic("failed to open `%s': %s", path.c_str(), strerror(errno));

    png_structp png_ptr;

    if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == 0)
        panic("png_create_read_struct failed");

    png_infop info_ptr;

    if ((info_ptr = png_create_info_struct(png_ptr)) == 0)
        panic("png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        panic("png error");

    png_init_io(png_ptr, fp);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (color_type != PNG_COLOR_TYPE_RGBA || bit_depth != 8)
        panic("invalid color type or bit depth in PNG");

    m_width = png_get_image_width(png_ptr, info_ptr);
    m_height = png_get_image_height(png_ptr, info_ptr);

    m_bits.resize(m_width * m_height);

    png_bytep *rows = png_get_rows(png_ptr, info_ptr);
    for (int i = 0; i < m_height; i++)
        memcpy(&m_bits[i * m_width], rows[m_height - i - 1], m_width * sizeof(uint32_t));

    png_destroy_read_struct(&png_ptr, &info_ptr, 0);

    fclose(fp);

    return true;
}
