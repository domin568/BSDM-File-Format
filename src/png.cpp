#include "headers/png.h"


uint8_t * readPngFile(FILE * fp, png_structp & png_ptr, png_infop & info_ptr)
{
    fseek (fp,8,0);
    png_bytep * row_pointers;

    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    int number_of_passes;
    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        printf("[read_png_file] png_create_read_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        printf("[read_png_file] png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[read_png_file] Error during init_io");
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[read_png_file] Error during read_image");
    }

    uint32_t line = png_get_rowbytes(png_ptr,info_ptr);

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++)
    {
        row_pointers[y] = (png_byte*) malloc(line);
    }

    png_read_image(png_ptr, row_pointers);

    uint8_t * data = (uint8_t *) malloc (height * line);
    for (int y = 0; y < height; y++)
    {
        memcpy (data + y * line,*(row_pointers + y),line);
    }
    fclose(fp);
    return data;
}

void writePngFile(FILE * fOUT, uint8_t * data, const BSDMHEADER & info)
{
    png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * info.height);
    for (uint32_t y = 0; y < info.height; y++)
    {
         row_pointers[y] = &data[y * info.width * 3];
    }
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        printf("[write_png_file] png_create_write_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        printf("[write_png_file] png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[write_png_file] Error during init_io");
    }

    png_init_io(png_ptr, fOUT);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[write_png_file] Error during writing header");
    }

    png_set_IHDR(png_ptr, info_ptr, info.width, info.height,
                  24, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[write_png_file] Error during writing bytes");
    }

    png_write_image(png_ptr, row_pointers);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[write_png_file] Error during end of write");
    }

    png_write_end(png_ptr, NULL);

    free(row_pointers);
    fclose(fOUT);

}

/*
void processFile()
{
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
        {
            printf("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
                       "(lacks the alpha channel)");
        }

        if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
        {
            printf("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
                       PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
        }

        for (y=0; y<height; y++) 
        {
            png_byte* row = row_pointers[y];
            for (x=0; x<width; x++) 
            {
                png_byte* ptr = &(row[x*4]);
                printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

                ptr[0] = 0;
                ptr[1] = ptr[2];
            }
        }
}

*/
