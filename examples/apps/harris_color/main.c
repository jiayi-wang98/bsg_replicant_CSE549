#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <bsg_manycore_regression.h>
#include <png.h>

#define ALLOC_NAME "default_allocator"

#ifndef IMAGE_SIZE_X
Please define IMAGE_SIZE_X in Makefile.
#endif

#ifndef IMAGE_SIZE_Y
Please define IMAGE_SIZE_Y in Makefile.
#endif

typedef struct {
    unsigned short type;                 // Magic identifier: 0x4d42
    unsigned int size;                   // File size in bytes
    unsigned short reserved1, reserved2;
    unsigned int offset;                 // Offset to image data, bytes
} HEADER;

typedef struct {
    unsigned int size;                   // Header size in bytes      
    unsigned int width;
    unsigned int height;                    // Width and height of image
    unsigned short planes;               // Number of colour planes   
    unsigned short bits;                 // Bits per pixel            
    unsigned int compression;            // Compression type
    unsigned int imagesize;              // Image size in bytes       
    unsigned int xresolution;
    unsigned int yresolution;         // Pixels per meter
    unsigned int ncolours;               // Number of colours         
    unsigned int importantcolours;       // Important colours         
} INFOHEADER;

typedef struct {
    unsigned char b,g,r;
} PIXEL;

void createBMP(const char *filename, int width, int height, short int *pixels);
int ReadBMP(const char* filename, short* pixelData, int width, int height);
void initializeImages(short int* A_host, short int* gray, int image_size_x, int image_size_y);
void rgb2gray(short int* A_host, short int* gray, int image_size_x, int image_size_y);
void applySobelFilter(const short int* gray, short int* grad_x, short int* grad_y, int image_size_x, int image_size_y);
void clampGradients(short int* grad, int image_size_x, int image_size_y);
void calculateProduct(const short int* grad_x, const short int* grad_y, short int* lxx, short int* lyy, short int* lxy, int image_size_x, int image_size_y);
void windowOperation(const short int* src, short int* dest, int image_size_x, int image_size_y);
void calculateCim(const short int* lgxx8, const short int* lgyy8, const short int* lgxy8, short int* cim, int image_size_x, int image_size_y);
void findLocalMaxima(const short int* cim, short int* Output_expected_host, int image_size_x, int image_size_y);
void printImage(const short int* image, int image_size_x, int image_size_y);

int kernel_harris_color(int argc, char **argv) {

  int rc;
  char *bin_path, *test_name;
  struct arguments_path args = {NULL, NULL};
  
  argp_parse(&argp_path, argc, argv, 0, 0, &args);
  bin_path = args.path;
  test_name = args.name;

  bsg_pr_test_info("Running kernel_harris_color.\n");
  srand(time);
 
  // Initialize Device.
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, test_name, 0));

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    bsg_pr_info("Loading program for pod %d\n.", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // Allocate a block of memory in host.
    int image_size_x;
    int image_size_y;
    //image_size_x=IMAGE_SIZE_X;
    //image_size_y=IMAGE_SIZE_Y;
    image_size_x=64;
    image_size_y=64;
    int size=image_size_x*image_size_y;
    int size_padding=image_size_x*image_size_y;
    short int * A_host = (short int*) malloc(sizeof(short)*size*3);
    short int * gray_expected = (short int*) malloc(sizeof(short int)*size);
    short int * Output_host = (short int*) malloc(sizeof(short int)*size);
    short int * Output_expected_host = (short int*) malloc(sizeof(short int)*size);
    short int * gray = (short int*) malloc(sizeof(short int)*size);
    short int * grad_x = (short int*) malloc(sizeof(short int)*size);
    short int * grad_y = (short int*) malloc(sizeof(short int)*size);
    short int * lxx = (short int*) malloc(sizeof(short int)*size);
    short int * lyy = (short int*) malloc(sizeof(short int)*size);
    short int * lxy = (short int*) malloc(sizeof(short int)*size);
    short int * lxx_d = (short int*) malloc(sizeof(short int)*size);
    short int * lyy_d = (short int*) malloc(sizeof(short int)*size);
    short int * lxy_d = (short int*) malloc(sizeof(short int)*size);
    short int * lgxx8 = (short int*) malloc(sizeof(short int)*size);
    short int * lgyy8 = (short int*) malloc(sizeof(short int)*size);
    short int * lgxy8 = (short int*) malloc(sizeof(short int)*size);
    short int * cim = (short int*) malloc(sizeof(short int)*size);
    short int * cim_d = (short int*) malloc(sizeof(short int)*size);

    //initializeImages(A_host, gray_expected, image_size_x, image_size_y);
    ReadBMP("/local1/jwang710/bsg_bladerunner/bsg_replicant/examples/hb_hammerbench/apps/harris_color/SpaceNeedle_256x256.bmp", A_host, image_size_x, image_size_y);
    rgb2gray(A_host, gray_expected, image_size_x, image_size_y);
    applySobelFilter(gray_expected, grad_x, grad_y, image_size_x, image_size_y);
    clampGradients(grad_x, image_size_x, image_size_y);
    clampGradients(grad_y, image_size_x, image_size_y);
    calculateProduct(grad_x, grad_y, lxx, lyy, lxy, image_size_x, image_size_y);
    windowOperation(lxx, lgxx8, image_size_x, image_size_y);
    windowOperation(lyy, lgyy8, image_size_x, image_size_y);
    windowOperation(lxy, lgxy8, image_size_x, image_size_y);
    calculateCim(lgxx8, lgyy8, lgxy8, cim, image_size_x, image_size_y);
    findLocalMaxima(cim, Output_expected_host, image_size_x, image_size_y);

    printImage(gray_expected, image_size_x, image_size_y);
    printImage(Output_expected_host, image_size_x, image_size_y);
    createBMP("/local1/jwang710/bsg_bladerunner/bsg_replicant/examples/hb_hammerbench/apps/harris_color/tile-x_8__tile-y_4__image-size-x_64__image-size-y_64__warm-cache_yes/output_expected.bmp", image_size_x, image_size_y,Output_expected_host);
    createBMP("/local1/jwang710/bsg_bladerunner/bsg_replicant/examples/hb_hammerbench/apps/harris_color/tile-x_8__tile-y_4__image-size-x_64__image-size-y_64__warm-cache_yes/input.bmp", image_size_x, image_size_y,gray_expected);

    // Make it pod-cache aligned
#define POD_CACHE_ALIGNED
#ifdef POD_CACHE_ALIGNED
    eva_t temp_device1, temp_device2;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, CACHE_LINE_WORDS*sizeof(int), &temp_device1));
    printf("temp Addr: %x\n", temp_device1);
    int align_size = (32)-1-((temp_device1>>2)%(CACHE_LINE_WORDS*32)/CACHE_LINE_WORDS);
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, align_size*sizeof(int)*CACHE_LINE_WORDS, &temp_device2));
#endif


    // Allocate a block of memory in device.
    eva_t A_device, Output_device;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, size * sizeof(short)*3, &A_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, size * sizeof(short), &Output_device));

    printf("A_device Addr: %x\n", A_device);
    printf("Output_device Addr: %x\n", Output_device);
 
    // DMA Transfer to device.

    hb_mc_dma_htod_t htod_job [] = {
      {
        .d_addr = A_device,
        .h_addr = (void *) &A_host[0],
        .size = image_size_x*image_size_y * sizeof(short)*3
      }
    };
    

    BSG_CUDA_CALL(hb_mc_device_dma_to_device(&device, htod_job, 1));


    // CUDA arguments
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 4
    uint32_t cuda_argv[CUDA_ARGC] = {A_device,Output_device,image_size_x,image_size_y};
    
    // Enqueue Kernel.
    BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel_harris_color", CUDA_ARGC, cuda_argv));
    
    // Launch kernel.
    hb_mc_manycore_trace_enable((&device)->mc);
    BSG_CUDA_CALL(hb_mc_device_tile_groups_execute(&device));
    hb_mc_manycore_trace_disable((&device)->mc);


    // Copy result and validate.
    hb_mc_dma_dtoh_t dtoh_job [] = {
      {
        .d_addr = Output_device,
        .h_addr = (void *) &Output_host[0],
        .size = image_size_x*image_size_y * sizeof(short)
      }

    };

    BSG_CUDA_CALL(hb_mc_device_dma_to_host(&device, &dtoh_job, 1));

    createBMP("/local1/jwang710/bsg_bladerunner/bsg_replicant/examples/hb_hammerbench/apps/harris_color/tile-x_8__tile-y_4__image-size-x_64__image-size-y_64__warm-cache_yes/output.bmp", image_size_x, image_size_y,Output_host);
    
    //harris color
    for (int i = 3; i < image_size_x-3; i++) {
        for (int j = 3; j < image_size_y-3; j++) {
          int index=i+j*image_size_x;
          if (Output_expected_host[index] != Output_host[index]) {
            printf("HARRIS FAIL [%d]: expected=%d, actual=%d\n", index, Output_expected_host[index], Output_host[index]);
            BSG_CUDA_CALL(hb_mc_device_finish(&device));
            return HB_MC_FAIL;
          }
        }
    }

    // Freeze tiles.
    BSG_CUDA_CALL(hb_mc_device_program_finish(&device));
  }

  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  return HB_MC_SUCCESS; 
}

//initialize images (chessboard example)
void initializeImages(short int* A_host, short int* gray, int image_size_x, int image_size_y) {
    for(int y = 0; y < image_size_y; y++) {
        for(int x = 0; x < image_size_x; x++) {
            int middle_index = x + y * image_size_x;
            if(((x % 32) < 16 && (y % 32) < 16) || ((x % 32) >= 16 && (y % 32) >= 16)) {
                A_host[middle_index*3] = 0;
                A_host[middle_index*3+1] = 0;
                A_host[middle_index*3+2] = 0;
            } else {
                A_host[middle_index*3] = 255;
                A_host[middle_index*3+1] = 255;
                A_host[middle_index*3+2] = 255;
            }
            gray[middle_index]=(short int) ((77 *A_host[middle_index*3] + 150 * A_host[middle_index*3+1]+ 29 * A_host[middle_index*3+2]) >> 8);
        }
    }
}

void rgb2gray(short int* A_host, short int* gray, int image_size_x, int image_size_y) {
    for(int y = 0; y < image_size_y; y++) {
        for(int x = 0; x < image_size_x; x++) {
            int middle_index = x + y * image_size_x;
            gray[middle_index]=(short int) ((77 *A_host[middle_index*3] + 150 * A_host[middle_index*3+1]+ 29 * A_host[middle_index*3+2]) >> 8);
        }
    }
}

// Apply the Sobel filter to the grayscale image to find gradients
void applySobelFilter(const short int* gray, short int* grad_x, short int* grad_y, int image_size_x, int image_size_y) {
    short int kernel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    short int kernel_y[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    for(int y = 0; y < image_size_y; y++) {
        for(int x = 0; x < image_size_x; x++) {
            short int gx = 0, gy = 0;
            for(int ky = -1; ky <= 1; ky++) {
                for(int kx = -1; kx <= 1; kx++) {
                    int pixel_x,pixel_y;
                    pixel_x=(x + kx);
                    pixel_y=(y + ky);
                    if(pixel_x<0) pixel_x=0;
                    else if(pixel_x> (image_size_x-1)) pixel_x=image_size_x-1;
                    if(pixel_y<0) pixel_y=0;
                    else if(pixel_y> (image_size_y-1)) pixel_y=image_size_y-1;
                    short int pixel = gray[pixel_y * image_size_x + pixel_x];
                    gx += pixel * kernel_x[ky + 1][kx + 1];
                    gy += pixel * kernel_y[ky + 1][kx + 1];
                }
            }
            grad_x[y * image_size_x + x] = gx;
            grad_y[y * image_size_x + x] = gy;
        }
    }
}

// Clamp gradients to ensure values are within a specific range
void clampGradients(short int* grad, int image_size_x, int image_size_y) {
    for(int i = 0; i < image_size_x * image_size_y; i++) {
        if(grad[i] < -180) grad[i] = -180;
        else if(grad[i] > 180) grad[i] = 180;
    }
}

// Calculate the product of gradients
void calculateProduct(const short int* grad_x, const short int* grad_y, short int* lxx, short int* lyy, short int* lxy, int image_size_x, int image_size_y) {
    for(int i = 0; i < image_size_x * image_size_y; i++) {
        lxx[i] = (grad_x[i] * grad_x[i]) >> 6;
        lyy[i] = (grad_y[i] * grad_y[i]) >> 6;
        lxy[i] = (grad_x[i] * grad_y[i]) >> 6;
    }
}

// Perform a window operation
void windowOperation(const short int* src, short int* dest, int image_size_x, int image_size_y) {
    for(int y = 1; y < image_size_y - 1; y++) {
        for(int x = 1; x < image_size_x - 1; x++) {
            int sum = 0;
            for(int ky = -1; ky <= 1; ky++) {
                for(int kx = -1; kx <= 1; kx++) {
                    sum += src[(y + ky) * image_size_x + (x + kx)];
                }
            }
            dest[y * image_size_x + x] = sum >> 6;
        }
    }
    //boundary
    for(int y = 0; y < image_size_y ; y++) {
        for(int x = 0; x < image_size_x; x++) {
            int actual_x,actual_y;
            actual_x=x;
            actual_y=y;
            if(x==0) actual_x=1;
            else if (x== image_size_x-1) actual_x=image_size_x-2;
            if(y==0) actual_y=1;
            else if (y== image_size_y-1) actual_y=image_size_y-2;
            dest[y * image_size_x + x]=dest[actual_y * image_size_x + actual_x];
        }
    }
}

// Calculate the corner response measure (CIM)
void calculateCim(const short int* lgxx8, const short int* lgyy8, const short int* lgxy8, short int* cim, int image_size_x, int image_size_y) {
    for(int i = 0; i < image_size_x * image_size_y; i++) {
        int det = lgxx8[i] * lgyy8[i] - lgxy8[i] * lgxy8[i];
        int trace_sq = (lgxx8[i] + lgyy8[i]) * (lgxx8[i] + lgyy8[i])>>4;
        cim[i] = (det - trace_sq);
    }
}

// Find local maxima and threshold to detect corners
void findLocalMaxima(const short int* cim, short int* Output_expected_host, int image_size_x, int image_size_y) {
    for(int y = 1; y < image_size_y - 1; y++) {
        for(int x = 1; x < image_size_x - 1; x++) {
            short int maxVal = cim[y * image_size_x + x];
            short int isMax = 1;
            for(int ky = -1; isMax && ky <= 1; ky++) {
                for(int kx = -1; kx <= 1; kx++) {
                    if(ky == 0 && kx == 0) continue;
                    if(cim[(y + ky) * image_size_x + (x + kx)] > maxVal) {
                        isMax = 0;
                        break;
                    }
                }
            }
            Output_expected_host[y * image_size_x + x] = (isMax && (maxVal > 1)) ? 255 : 0;
        }
    }
}

// Print the image matrix to console
void printImage(const short int* image, int image_size_x, int image_size_y) {
    for(int y = 0; y < image_size_y; y++) {
        for(int x = 0; x < image_size_x; x++) {
            printf(" %d ", image[y * image_size_x + x] );
        }
        printf(" \n ");
    }
}


void createBMP(const char *filename, int width, int height, short int *pixels) {
    HEADER header;
    INFOHEADER infoheader;
    FILE *file;
    PIXEL pixel = {0,0,0};

    int imagesize = width * height * sizeof(PIXEL);

    // Set up the BMP header
    memcpy(&header.type, "BM", 2);
    //header.type = 0x4D42;
    header.size = sizeof(HEADER) + sizeof(INFOHEADER) + imagesize;
    //printf("BMP SIZE= %d,%x\n",header.size,header.size);
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offset = sizeof(HEADER) + sizeof(INFOHEADER)-2;
    //printf("OFFSET SIZE= %d,%x\n",header.offset,header.offset);

    // Set up the BMP info header
    infoheader.size = sizeof(INFOHEADER);
    //printf("INFO HEADER SIZE= %d,%x\n",infoheader.size,infoheader.size);
    infoheader.width = width;
    infoheader.height = height;
    infoheader.planes = 1;
    infoheader.bits = 24;
    infoheader.compression = 0;
    infoheader.imagesize = imagesize;
    infoheader.xresolution = 0x0B12; // 2835 pixels per meter
    infoheader.yresolution = 0x0B12; // 2835 pixels per meter
    infoheader.ncolours = 0;
    infoheader.importantcolours = 0;

    // Create file
    file = fopen(filename, "wb");
    if (!file) {
        printf("Unable to open file!");
        return;
    }

    // Write headers
    fwrite(&header.type, 1, sizeof(header.type),  file);
    fwrite(&header.size, 1, sizeof(header.size),  file);
    fwrite(&header.reserved1, 1, sizeof(header.reserved1),  file);
    fwrite(&header.reserved2, 1, sizeof(header.reserved2),  file);
    fwrite(&header.offset, 1, sizeof(header.offset),  file);
    fwrite(&infoheader.size , 1,sizeof(infoheader.size ), file);
    fwrite(&infoheader.width , 1,sizeof(infoheader.width ), file);
    fwrite(&infoheader.height , 1,sizeof(infoheader.height ), file);
    fwrite(&infoheader.planes , 1,sizeof(infoheader.planes ), file);
    fwrite(&infoheader.bits , 1,sizeof(infoheader.bits), file);
    fwrite(&infoheader.compression , 1,sizeof(infoheader.compression), file);
    fwrite(&infoheader.imagesize , 1,sizeof(infoheader.imagesize ), file);
    fwrite(&infoheader.xresolution , 1,sizeof(infoheader.xresolution ), file);
    fwrite(&infoheader.yresolution , 1,sizeof(infoheader.yresolution ), file);
    fwrite(&infoheader.ncolours , 1,sizeof(infoheader.ncolours), file);
    fwrite(&infoheader.importantcolours , 1,sizeof(infoheader.importantcolours ), file);

    // Write bitmap
    for (int y = height - 1; y >= 0; y--) { // BMP files are stored bottom-to-top
        for (int x = 0; x < width; x++) {
            int i = y * width + x;
            pixel.b = pixel.g = pixel.r = (unsigned char)(pixels[i]);
            fwrite(&pixel, 1, sizeof(PIXEL),file);
        }
    }

    fclose(file);
}

int ReadBMP(const char* filename, short* pixelData, int width, int height) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    unsigned char *bitmapImage;
    HEADER fileHeader;
    INFOHEADER infoHeader;

    fread(&fileHeader.type, sizeof(fileHeader.type), 1, file);
    fread(&fileHeader.size, sizeof(fileHeader.size), 1, file);
    fread(&fileHeader.reserved1, sizeof(fileHeader.reserved1), 1, file);
    fread(&fileHeader.reserved2, sizeof(fileHeader.reserved2), 1, file);
    fread(&fileHeader.offset, sizeof(fileHeader.offset), 1, file);
    fread(&infoHeader, sizeof(INFOHEADER), 1, file);

    if (fileHeader.type != 0x4D42) {
        printf("Not a valid BMP file\n");
        fclose(file);
        return -2;
    }

    int padding = (4 - ((width) * 3) % 4) % 4;

    fseek(file, fileHeader.offset, SEEK_SET);
    //printf("BMP offset %d\n",fileHeader.offset);

    bitmapImage = (unsigned char*)malloc(width*height*3);

    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(file);
        return NULL;
    }

    fread(bitmapImage,width*height*3,1,file);

    //make sure pixelData image data was read
    if (bitmapImage == NULL)
    {
        fclose(file);
        printf("Reading BMP file error!!!!!\n");
        return NULL;
    }

    char* output;

    for (int imageIdx_y = 0 ;imageIdx_y < height ;imageIdx_y+=1)
    {
        for (int imageIdx_x = 0 ;imageIdx_x < width ;imageIdx_x+=1){
            int index=(imageIdx_x+imageIdx_y*width)*3;
            int index_row_rev=(imageIdx_x+(height-1-imageIdx_y)*width)*3;
    
            pixelData[index_row_rev] = (short) bitmapImage[index + 2];
            pixelData[index_row_rev+1] = (short) bitmapImage[index + 1];
            pixelData[index_row_rev+2] = (short) bitmapImage[index];
        }
    }

    free(bitmapImage);
    fclose(file);

    return 0; // Success
}

declare_program_main("harris_color", kernel_harris_color);
