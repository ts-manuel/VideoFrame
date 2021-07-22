/**
 * File: array.c
 * Author: ts-manuel
 * 
 * Writes array of bytes to file
*/

#include "array.h"

static void write_header(FILE* fp, char* array_name);
static void write_data(FILE* fp, uint8_t* data, int width, int height);
static void write_end(FILE* fp);



void write_array(FILE* fp, uint8_t* data, char* array_name, int width, int height)
{
     write_header(fp, array_name);

    write_data(fp, data, width, height);
    
    write_end(fp);
}


static void write_header(FILE* fp, char* array_name)
{
    fprintf(fp, "\n");
    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "\n");

    //Print array
    fprintf(fp, "const uint8_t %s[] =\n", array_name);
    fprintf(fp, "{\n");
}


static void write_data(FILE* fp, uint8_t* data, int width, int height)
{
    bool dupplicate = false;

    //Dupplicate last pixel if width is odd
    if(width % 2 != 0)
    {
        width += 1;
        dupplicate = true;
    }

    for(int y = 0; y < height; y++)
    {
        fprintf(fp, "\t");

        for(int x = 0; x < width; x += 2)
        {
            uint8_t byte;

            if(dupplicate && x == width - 1)
            {
                byte = (data[0] << 4) | data[0];
                data ++;
            }
            else
            {
                byte = (data[0] << 4) | data[1];
                data += 2;
            }

            fprintf(fp, "0x%02X,", byte);
        }

        fprintf(fp, "\n");
    }
}


static void write_end(FILE* fp)
{
    fprintf(fp, "};\n");
}