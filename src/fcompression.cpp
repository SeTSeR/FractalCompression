#include "fcompression.h"


/// Compress an image
void
Compress(const char *image_filename,
         const char *archive_filename,
         const int block_size,
         const int quality)
{
    // Load the input image
    Image *image = load_image(image_filename);
    const int32_t width = image->info_header.width;
    const int32_t height = image->info_header.height;

    if ((width != 256 && width != 512) || width != height)
        throw std::runtime_error("Input image should be "
                                 "256x256 or 512x512 pixels");

    RGBPixel **image_data = image->pixels;

    /***** PUT YOUR CODE HERE ****/

    /* Example: simple copy of raw data */
    save_image(archive_filename, image);
    /************************************/
}


/// Decompress an image
void
Decompress(const char *archive_filename,
           const char *image_filename)
{
    /***** PUT YOUR CODE HERE ****/
    //Example: simple copy of the data
    Image *image = load_image(archive_filename);
    const int32_t width = image->info_header.width;
    const int32_t height = image->info_header.height;

    if ((width != 256 && width != 512) || width != height)
        throw std::runtime_error("Input image should be "
                                 "256x256 or 512x512 pixels");

    RGBPixel **image_data = image->pixels;
    /*****************************/


    // Save the image on the hard drive
    save_image(image_filename, image);
}
