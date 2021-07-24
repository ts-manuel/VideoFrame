/**
 * File: cinveter.c
 * Author: ts-manuel
 * 
 * Convert image from rgb to 7 color
*/

#include "converter.h"

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB8_t;

#define _NUM_COLORS 7
const RGB8_t color_palette[_NUM_COLORS] = 
{
    {  0,   0,   0},    //Black
    {255, 255, 255},    //White
    {  0, 255,   0},    //Green
    {  0,   0, 255},    //Blue
    {255,   0,   0},    //Red
    {255, 255,   0},    //Yellow
    {255, 128,   0}     //Orange
};


static void convert_pixels(uint8_t* out, uint8_t* in, int width, int height);
static int find_closest_color(RGB8_t color);
static int distance2(RGB8_t c1, RGB8_t c2);


uint8_t* convert(uint8_t* pix, int width, int height)
{
    uint8_t* p_out;
    
    //Allocate memory for the output array
    p_out = malloc(width * height * sizeof(uint8_t));
    if(p_out == NULL)
    {
        #ifdef DEBUG
            printf("[converter.c convert()] Memory allocation failed\n");
        #endif
        return NULL;
    }

    //Convert image data
    convert_pixels(p_out, pix, width, height);

    return p_out;
}


static void convert_pixels(uint8_t* out, uint8_t* in, int width, int height)
{
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            RGB8_t src;

            //Extract pixel from input data
            src.r = in[0];
            src.g = in[1];
            src.b = in[2];
            in += 3;

            //Find closest color
            out[0] = find_closest_color(src);
            out += 1;
        }
    }
}


/*
    Searches the palette for the closest color and returns its index
*/
static int find_closest_color(RGB8_t color)
{
    int closest_dst = INT32_MAX;
    int indx;

    for(int i = 0; i < _NUM_COLORS; i++)
    {
        int dst = distance2(color, color_palette[i]);

        if(dst < closest_dst)
        {
            closest_dst = dst;
            indx = i;
        }
    }

    return indx;
}


/*
    Computes the distance sqared betwen two colors
*/
static int distance2(RGB8_t c1, RGB8_t c2)
{
    int dr = (int)c1.r - (int)c2.r;
    int dg = (int)c1.g - (int)c2.g;
    int db = (int)c1.b - (int)c2.b;

    return dr*dr + dg*dg + db*db;
}