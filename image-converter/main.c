/**
 * File: main.c
 * Author: ts-manuel
 * 
 * Simple commnd line programm to convert image files (.png .bmp .jpg)
 * to a c array with 4 bits per pixel to be used with Waveshare 7 color e-Paper display
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stb_image.h"
#include "array.h"
#include "converter.h"

#define _DESIRED_CHANNELS 3



int main(int argc, char* argv[])
{
    FILE* fp_in;
    FILE* fp_out;
    int width, height, channels;
    stbi_uc* p_pix_in;
    uint8_t* p_pix_out;

    //Check command line arguments
    if(argc != 3)
    {
        printf("Usage: imgconv input_file output_file\n");
        return EXIT_FAILURE;
    }

    //Open input file
    fp_in = fopen(argv[1], "r");
    if(fp_in == NULL)
    {
        printf("ERROR: Unable to open file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    //Load image data
    p_pix_in = stbi_load_from_file(fp_in, &width, &height, &channels, _DESIRED_CHANNELS);
    if(p_pix_in == NULL)
    {
        printf("ERROR: Unable to load image data\n");
        return EXIT_FAILURE;
    }

    //Convert image
    p_pix_out = convert(p_pix_in, width, height);
    if(p_pix_out == NULL)
    {
        printf("ERROR: Unable to convert image data\n");
        return EXIT_FAILURE;
    }

    //Open output file
    fp_out = fopen(argv[2], "w");
    if(fp_out == NULL)
    {
        printf("ERROR: Unable to open output file: %s\n", argv[3]);
        return EXIT_FAILURE;
    }

    //Write to file
    write_array(fp_out, p_pix_out, "image", width, height);

    //Close open files and free memory
    fclose(fp_in);
    fclose(fp_out);
    free(p_pix_in);
    free(p_pix_out);

    return EXIT_SUCCESS;
}