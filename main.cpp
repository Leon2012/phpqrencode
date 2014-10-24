#include <phpcpp.h>
#include <iostream>
#include <string>

extern "C" {
    #include <stdio.h>
    #include <stdlib.h>
    #include <qrencode.h>
    #include <png.h>
}

#define INCHES_PER_METER (100.0/2.54)
static int casesensitive = 0;
static int eightbit = 0;
static int version = 0;
int size = 10;
static int margin = -1;
static int dpi = 72;
static int structured = 0;
static int rle = 0;
static int micro = 0;
static QRecLevel level = QR_ECLEVEL_H;
static QRencodeMode hint = QR_MODE_8;
static unsigned int fg_color[4] = {0, 0, 0, 255};
static unsigned int bg_color[4] = {255, 255, 255, 255};
static int verbose = 0;

static int writePNG(QRcode *qrcode, const char *outfile){
    static FILE *fp; // avoid clobbering by setjmp.
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;
    png_byte alpha_values[2];
    unsigned char *row, *p, *q;
    int x, y, xx, yy, bit;
    int realwidth;

    realwidth = (qrcode->width + margin * 2) * size;
    row = (unsigned char *)malloc((realwidth + 7) / 8);
    if(row == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    if(outfile[0] == '-' && outfile[1] == '\0') {
        fp = stdout;
    } else {
        fp = fopen(outfile, "wb");
        if(fp == NULL) {
            Php::error << "Failed to create file" << std::flush;
            return 0;
        }
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL) {
        Php::error << "Failed to initialize PNG writer." << std::flush;
        return 0;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL) {
        Php::error << "Failed to initialize PNG writer." << std::flush;
        return 0;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        Php::error << "Failed to write PNG image." << std::flush;
        return 0;
    }

    palette = (png_colorp) malloc(sizeof(png_color) * 2);
    if(palette == NULL) {
        Php::error << "Failed to allocate memory." << std::flush;
        return 0;
    }
    palette[0].red   = fg_color[0];
    palette[0].green = fg_color[1];
    palette[0].blue  = fg_color[2];
    palette[1].red   = bg_color[0];
    palette[1].green = bg_color[1];
    palette[1].blue  = bg_color[2];
    alpha_values[0] = fg_color[3];
    alpha_values[1] = bg_color[3];
    png_set_PLTE(png_ptr, info_ptr, palette, 2);
    png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr,
            realwidth, realwidth,
            1,
            PNG_COLOR_TYPE_PALETTE,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);
    png_set_pHYs(png_ptr, info_ptr,
            dpi * INCHES_PER_METER,
            dpi * INCHES_PER_METER,
            PNG_RESOLUTION_METER);
    png_write_info(png_ptr, info_ptr);

    /* top margin */
    memset(row, 0xff, (realwidth + 7) / 8);
    for(y=0; y<margin * size; y++) {
        png_write_row(png_ptr, row);
    }

    /* data */
    p = qrcode->data;
    for(y=0; y<qrcode->width; y++) {
        bit = 7;
        memset(row, 0xff, (realwidth + 7) / 8);
        q = row;
        q += margin * size / 8;
        bit = 7 - (margin * size % 8);
        for(x=0; x<qrcode->width; x++) {
            for(xx=0; xx<size; xx++) {
                *q ^= (*p & 1) << bit;
                bit--;
                if(bit < 0) {
                    q++;
                    bit = 7;
                }
            }
            p++;
        }
        for(yy=0; yy<size; yy++) {
            png_write_row(png_ptr, row);
        }
    }
    /* bottom margin */
    memset(row, 0xff, (realwidth + 7) / 8);
    for(y=0; y<margin * size; y++) {
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    free(row);
    free(palette);

    return 1;
}

void qrencode_apiversion(){
    char *v = QRcode_APIVersionString();
    Php::out << v << std::endl;
}

Php::Value qrencode(Php::Parameters &params) {
    Php::Value val = params[0];
    Php::Value len = params[1];
    Php::Value out = params[2];
    
    std::string text = val.stringValue();
    std::string outFile = out.stringValue();
    int length = (int)len.numericValue();
    size = length;

    QRcode *qrcode;
    qrcode = QRcode_encodeString((char *)text.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);


    if(qrcode == NULL) {
        Php::error << "Failed to encode the input data" << std::flush;
        return 0;
    }
    return writePNG(qrcode, outFile.c_str());
}



/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {
    
    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("phpqrencode", "1.0");
        extension.add("qrencode_apiversion", qrencode_apiversion);
        extension.add("qrencode", qrencode, {
            Php::ByVal("text", Php::Type::String, true),
            Php::ByVal("size", Php::Type::Numeric, true),
            Php::ByVal("out", Php::Type::String, true)
        });


        // @todo    add your own functions, classes, namespaces to the extension
        
        // return the extension
        return extension;
    }
}
