/*
   Sample program to read a limited number of BMP file types
   Write out lots of diagnostics
   Write out a 24 bit RAW pixels file

   SourceCode was published at: http://paulbourke.net/dataformats/bmp/parse.c

   modified on XX.11.18 [J]
*/


#include "bmplib.h"


Image *load_image(const char *filename)
{   
   /* Open file */
   FILE *input = fopen(filename, "rb");
   if (input == NULL) {
      fprintf(stderr,"Unable to open BMP file \"%s\"\n", filename);
      return NULL;
   }
   
   // result image
   Image *image = (Image *)calloc(1, sizeof(Image));

   /* Read and check header */
   if (!ReadHeader(input, &(image->header))) {
      fprintf(stderr, "BMP header error\n");
      delete_image(image);
      return NULL;
   }

   /* Read and check info header */
   if (!ReadInfoHeader(input, &(image->info_header))) {
      fprintf(stderr, "BMP InfoHeader error\n");
      delete_image(image);
      return NULL;
   }


   /* Read the lookup table if there is one */
   RGBPixel color_index[256];
   for (int i = 0; i < 255; ++i) {
      color_index[i].r     = rand() % 256;
      color_index[i].g     = rand() % 256;
      color_index[i].b     = rand() % 256;
      color_index[i].junk  = rand() % 256;
   }

   bool got_index = false;
   if (image->info_header.ncolors > 0) {
      for (int i = 0; i < image->info_header.ncolors; ++i) {
         if (!ReadBytes(input, &(color_index[i].b), sizeof(color_index[i].b), false)) {
            fprintf(stderr,"pixels read failed\n");
            delete_image(image);
            return NULL;
         }
         if (!ReadBytes(input, &(color_index[i].g), sizeof(color_index[i].g), false)) {
            fprintf(stderr,"pixels read failed\n");
            delete_image(image);
            return NULL;
         }
         if (!ReadBytes(input, &(color_index[i].r), sizeof(color_index[i].r), false)) {
            fprintf(stderr,"pixels read failed\n");
            delete_image(image);
            return NULL;
         }
         if (!ReadBytes(input, &(color_index[i].junk), sizeof(color_index[i].junk), false)) {
            fprintf(stderr,"pixels read failed\n");
            delete_image(image);
            return NULL;
         }

         fprintf(stderr,"%3d\t%3d\t%3d\t%3d\n",
                 i, color_index[i].r, color_index[i].g, color_index[i].b);
      }
      got_index = true;
   }

   /* Seek to the start of the pixels data */
   fseek(input, image->header.offset, SEEK_SET);

   /* Read the pixels */
   RGBPixel **pixels = (RGBPixel **)calloc(image->info_header.height, sizeof(RGBPixel *));
   for (int i = 0; i < image->info_header.height; ++i) {
      pixels[i] = (RGBPixel *)calloc(image->info_header.width, sizeof(RGBPixel));
   }


   for (int i = 0; i < image->info_header.height; ++i) {
      for (int j = 0; j < image->info_header.width; ++j) {

         uint8_t color = 0;
         switch (image->info_header.bits) {
         case 8:
            if (!ReadBytes(input, &color, sizeof(color), false)) {
               fprintf(stderr,"pixels read failed\n");
               delete_pixels(pixels, image->info_header.height);
               delete_image(image);
               return NULL;
            }

            if (got_index) {
               pixels[i][j].r = color_index[color].r;
               pixels[i][j].g = color_index[color].g;
               pixels[i][j].b = color_index[color].b;
               
               // putchar(color_index[grey].r);
               // putchar(color_index[grey].g);
               // putchar(color_index[grey].b);
            } else {
               pixels[i][j].r = color;
               pixels[i][j].g = color;
               pixels[i][j].b = color;

               // putchar(grey);
            }
            break;

         case 24:
            if (!ReadBytes(input, &(pixels[i][j].b), sizeof(pixels[i][j].b), false)) {
               fprintf(stderr,"pixels read failed\n");
               delete_pixels(pixels, image->info_header.height);
               delete_image(image);
               return NULL;
            }

            if (!ReadBytes(input, &(pixels[i][j].g), sizeof(pixels[i][j].g), false)) {
               fprintf(stderr,"pixels read failed\n");
               delete_pixels(pixels, image->info_header.height);
               delete_image(image);
               return NULL;
            }
            if (!ReadBytes(input, &(pixels[i][j].r), sizeof(pixels[i][j].r), false)) {
               fprintf(stderr,"pixels read failed\n");
               delete_pixels(pixels, image->info_header.height);
               delete_image(image);
               return NULL;
            }

            // printf("(%u, %u, %u) ", pixels[i][j].r, pixels[i][j].g, pixels[i][j].b);
            break;
         
         default:
            break;
         }
      } // putchar('\n');
   }
   fclose(input);

   image->pixels = pixels;
   return image;
}

bool save_image(const char *filename, Image *image)
{
   FILE *output = fopen(filename, "wb");
   if (output == NULL) {
      fprintf(stderr, "Unable to create bmp.\n");
      return false;
   }

   // write header
   if (!WriteHeader(output, &(image->header))) {
      fclose(output);
      return false;
   }

   if (!WriteInfoHeader(output, &(image->info_header))) {
      fclose(output);
      return false;
   }

   for (int i = 0; i < image->info_header.height; ++i) {
      for (int j = 0; j < image->info_header.width; ++j) {
         
         uint8_t color[3];
         color[0] = image->pixels[i][j].r;
         color[1] = image->pixels[i][j].g;
         color[2] = image->pixels[i][j].b;
        
         if (!WriteBytes(output, color, sizeof(*color) * 3, true)) {
            fclose(output);
            return false;
         }
      }
   }

   fclose(output);
   return true;
}


bool draw_solid_image(const char *filename, uint32_t height, uint32_t width, uint8_t color[3]) 
{
   Header header; memset(&header, 0, sizeof(header));
   InfoHeader info_header; memset(&info_header, 0, sizeof(info_header));

   header.type = 'M' * 256 + 'B';
   header.size = 14 + sizeof(info_header) + 3 * width * height;
   header.offset = 14 + sizeof(info_header);

   /* change it for more complicated info_header */
   info_header.size = sizeof(InfoHeader);
   info_header.width = width;
   info_header.height = height;
   info_header.planes = 1;
   info_header.bits = 24;
   info_header.imagesize = width * height * 3;
   info_header.xresolution = 2834;
   info_header.yresolution = 2834;

   RGBPixel **pixels = (RGBPixel **)calloc(sizeof(RGBPixel *), height);
   for (int i = 0; i < height; ++i) {
      pixels[i] = (RGBPixel *)calloc(sizeof(RGBPixel), width);
      for (int j = 0; j < width; ++j) {
         pixels[i][j].r = color[0];
         pixels[i][j].g = color[1];
         pixels[i][j].b = color[2];
      }
   }

   Image *image = (Image *)calloc(1, sizeof(*image));
   image->header = header;
   image->info_header = info_header;
   image->pixels = pixels;

   bool RETURN_VALUE = save_image(filename, image);
   delete_image(image);
   return RETURN_VALUE;
}


// get pixel matrix per channel
ImageMatrix *get_image_matrix(Image *base_image, ColorSpace colorspace)
{
   if (base_image == NULL) {
      return NULL;
   }

   uint32_t width = base_image->info_header.width;
   uint32_t height = base_image->info_header.height;

   ImageMatrix *matrix = (ImageMatrix *)calloc(1, sizeof(*matrix));
   if (matrix == NULL) {
      fprintf(stderr, "Error: can't create ImageMatrix! (memory limit)\n");
      return NULL;
   }

   matrix->width = width;
   matrix->height = height;
   matrix->colorspace = colorspace;

   if (colorspace == RGB) {
      matrix->r_channel = (uint8_t **)calloc(height, sizeof(uint8_t *));
      matrix->g_channel = (uint8_t **)calloc(height, sizeof(uint8_t *));
      matrix->b_channel = (uint8_t **)calloc(height, sizeof(uint8_t *));
   } else if (colorspace == YUV) {
      matrix->y_channel = (uint8_t **)calloc(height, sizeof(uint8_t *));
      matrix->u_channel = (uint8_t **)calloc(height, sizeof(uint8_t *));
      matrix->v_channel = (uint8_t **)calloc(height, sizeof(uint8_t *));
   } else {
      fprintf(stderr, "Unsupported color space!\n");
      delete_image_matrix(matrix);
      return NULL;
   }

   for (uint32_t i = 0; i < height; ++i) {
      if (colorspace == RGB) {
         matrix->r_channel[i] = (uint8_t *)calloc(width, sizeof(uint8_t));
         matrix->g_channel[i] = (uint8_t *)calloc(width, sizeof(uint8_t));
         matrix->b_channel[i] = (uint8_t *)calloc(width, sizeof(uint8_t));
      } else if (colorspace == YUV) {
         matrix->y_channel[i] = (uint8_t *)calloc(width, sizeof(uint8_t));
         matrix->u_channel[i] = (uint8_t *)calloc(width, sizeof(uint8_t));
         matrix->v_channel[i] = (uint8_t *)calloc(width, sizeof(uint8_t));
      }

      for (uint32_t j = 0; j < width; ++j) {
         if (colorspace == RGB) {
            matrix->r_channel[i][j] = base_image->pixels[i][j].r;
            matrix->g_channel[i][j] = base_image->pixels[i][j].g;
            matrix->b_channel[i][j] = base_image->pixels[i][j].b;
         } else if (colorspace == YUV) {
            uint8_t r = base_image->pixels[i][j].r;
            uint8_t g = base_image->pixels[i][j].g;
            uint8_t b = base_image->pixels[i][j].b;

            matrix->y_channel[i][j] = GetYFromRGB(r, g, b);
            matrix->u_channel[i][j] = GetUFromRGB(r, g, b);
            matrix->v_channel[i][j] = GetVFromRGB(r, g, b);
         }
      }
   }

   return matrix;
}


// Read and check the header 
bool ReadHeader(FILE *input, Header *buffer)
{
   bool check = true;
   check &= ReadBytes(input, &(buffer->type), sizeof(buffer->type), false);
   check &= ReadBytes(input, &(buffer->size), sizeof(buffer->size), false);
   check &= ReadBytes(input, &(buffer->reserved1), sizeof(buffer->reserved1), false);
   check &= ReadBytes(input, &(buffer->reserved2), sizeof(buffer->reserved2), false);
   check &= ReadBytes(input, &(buffer->offset), sizeof(buffer->offset), false);

   if (check) {
      fprintf(stderr, "ID is: %d, should be %d\n", buffer->type,'M' * 256 + 'B');
      fprintf(stderr, "File size is %d bytes\n", buffer->size);
      fprintf(stderr, "Offset to pixels data is %d bytes\n", buffer->offset);
      return true;
   } else {
      return false;
   }
}

// read and check InfoHeader
bool ReadInfoHeader(FILE *input, InfoHeader *buffer)
{
   if (ReadBytes(input, buffer, sizeof(*buffer), false)) {
      fprintf(stderr, "header size = %d\n", buffer->size);
      fprintf(stderr, "pixels size = %d x %d\n", buffer->width,buffer->height);
      fprintf(stderr, "Number of color planes is %d\n", buffer->planes);
      fprintf(stderr, "Bits per RGBPixel is %d\n", buffer->bits);
      fprintf(stderr, "Compression type is %d\n", buffer->compression);
      fprintf(stderr, "imagesize = %d\n", buffer->imagesize);
      fprintf(stderr, "xresolution = %d\n", buffer->xresolution);
      fprintf(stderr, "yresolution = %d\n", buffer->yresolution);
      fprintf(stderr, "Number of colors is %d\n", buffer->ncolors);
      fprintf(stderr, "Number of required colors is %d\n",
         buffer->important_colors);

      return true;
   } else {
      return false;
   } 
}

// write Header into a file
bool WriteHeader(FILE *output, Header *header)
{
   bool check = true;
   check &= WriteBytes(output, &(header->type), sizeof(header->type), false);
   check &= WriteBytes(output, &(header->size), sizeof(header->size), false);
   check &= WriteBytes(output, &(header->reserved1), sizeof(header->reserved1), false);
   check &= WriteBytes(output, &(header->reserved2), sizeof(header->reserved2), false);
   check &= WriteBytes(output, &(header->offset), sizeof(header->offset), false);

   if (check) {
      return true;
   } else {
      // error will be shown in WriteBytes function
      return false;
   }
}

// write InfoHeader into a file
bool WriteInfoHeader(FILE *output, InfoHeader *info_header)
{
   if (WriteBytes(output, info_header, sizeof(*info_header), false)) {
      // error will be shown in WriteBytes function
      return true;
   } else {
      return false;
   } 
}


// ========================= utilities ===================================
// read bytes into a buffer; if swap is true, reverse buffer
static bool ReadBytes(FILE *input, void *buffer, size_t size, bool swap)
{
   if (fread(buffer, size, 1, input) < 1) {
      return false;
   }

   if (swap) {
      size_t mid = size / 2;
      for (size_t i = 0; i <= mid; ++i) {
         uint8_t byte = ((uint8_t *)buffer)[i];
         ((uint8_t *)buffer)[i] = ((uint8_t *)buffer)[i + 1];
         ((uint8_t *)buffer)[i + 1] = byte;
      }
   }

   return true;
}


// write bytes from buffer; if swap is true, buffer was reversed 
static bool WriteBytes(FILE *output, void *buffer, size_t size, bool swap)
{
   if (swap) {
      for (size_t i = 0; i < size; ++i) {
         uint8_t byte = ((uint8_t *)buffer)[(size - 1) - i];
         if (fwrite(&byte, 1, 1, output) < 1) {
            fprintf(stderr, "Error. Bytes can't be written.\n");
            return false;
         }
      }
   } else {
      if (fwrite(buffer, size, 1, output) < 1) {
         fprintf(stderr, "Error. Bytes can't be written.\n");
         return false;
      }
   }
   return true;
}


// deep remove 
void delete_image(Image *image) 
{
   if (image) {
      delete_pixels(image->pixels, image->info_header.height);
      free(image);
   }
}


void delete_image_matrix(ImageMatrix *matrix) 
{
   if (matrix) {
      for (int i = 0; i < matrix->height; ++i) {
        if (matrix->colorspace == RGB) {
            free(matrix->r_channel[i]);
            free(matrix->g_channel[i]);
            free(matrix->b_channel[i]);
        } else if (matrix->colorspace == YUV) {
            free(matrix->y_channel[i]);
            free(matrix->u_channel[i]);
            free(matrix->v_channel[i]);
        }
      }

      free(matrix);
   }
}


static void delete_pixels(RGBPixel **pixels, int32_t count) 
{
   if (pixels) {
      for (int i = 0; i < count; ++i) {
         free(pixels[i]);
      }
      free(pixels);
   }
}