#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

#ifdef WARM_CACHE
__attribute__((noinline))
static void warmup(short *A, short *G, short *lxx,short *lyy,short *lxy, short *cim,short *Output,int N_X, int N_Y)
{
  int x_tile_len = N_X / bsg_tiles_X ;
  int y_tile_len = N_Y / bsg_tiles_Y ;

  //to calculate the CIM for X*Y block, we need (X+4)(Y+4) block
  int init_index_x=(__bsg_id%bsg_tiles_X)*x_tile_len;
  int init_index_y=(__bsg_id/bsg_tiles_X)*y_tile_len;
  //calculate gray scale image
  for (int x = 0; x < x_tile_len; x++) {
    for (int y = 0; y < y_tile_len; y++) {
      int global_x=x+init_index_x;
      int global_y=y+init_index_y;
      int global_index=global_x+global_y*N_X;
      asm volatile ("lw x0, %[p]" :: [p] "m" (A[global_index*3]));
      asm volatile ("lw x0, %[p]" :: [p] "m" (A[global_index*3+1]));
      asm volatile ("lw x0, %[p]" :: [p] "m" (A[global_index*3+2]));
      asm volatile ("sw x0, %[p]" :: [p] "m" (G[global_index]));
      asm volatile ("sw x0, %[p]" :: [p] "m" (lxx[global_index]));
      asm volatile ("sw x0, %[p]" :: [p] "m" (lyy[global_index]));
      asm volatile ("sw x0, %[p]" :: [p] "m" (lxy[global_index]));
      asm volatile ("sw x0, %[p]" :: [p] "m" (cim[global_index]));
      asm volatile ("sw x0, %[p]" :: [p] "m" (Output[global_index]));
    }
  }
  bsg_fence();
}
#endif


// harris_color: C = A + B
// N = # of frames
extern "C" __attribute__ ((noinline))
int
kernel_harris_color(short * A, short *G,short *lxx,short *lyy,short *lxy,short *cim,short *Output, int N_X, int N_Y) {

  bsg_barrier_hw_tile_group_init();
#ifdef WARM_CACHE
  warmup(A, G, lxx, lyy,lxy,cim,Output, N_X,N_Y);
#endif
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  //original program
  // Each tile does a portion of vector_add
  int x_tile_len = N_X / bsg_tiles_X ;
  int y_tile_len = N_Y / bsg_tiles_Y ;

  //to calculate the CIM for X*Y block, we need (X+4)(Y+4) block
  int init_index_x=(__bsg_id%bsg_tiles_X)*x_tile_len;
  int init_index_y=(__bsg_id/bsg_tiles_X)*y_tile_len;
  //int init_index_inline=init_index_x+init_index_y*N_X;
  int tile_image_size_x=x_tile_len;
  int tile_image_size_y=y_tile_len;

  int total_pixel_num=(x_tile_len)*(y_tile_len);
  //calculate gray scale image
  short myA[12];
  short myG[4];
  //calculate gray scale image
  for (int y = 0; y < tile_image_size_y; y++) {
    int global_x=init_index_x;
    int global_y=y+init_index_y;
    int global_index=global_x+global_y*N_X;
    myA[0]=A[global_index*3];
    myA[1]=A[global_index*3+1];
    myA[2]=A[global_index*3+2];
    myA[3]=A[global_index*3+3];
    myA[4]=A[global_index*3+4];
    myA[5]=A[global_index*3+5];
    myA[6]=A[global_index*3+6];
    myA[7]=A[global_index*3+7];
    myA[8]=A[global_index*3+8];
    myA[9]=A[global_index*3+9];
    myA[10]=A[global_index*3+10];
    myA[11]=A[global_index*3+11];
    myG[0]=((77 *myA[0] + 150 * myA[1]+ 29 * myA[2]) >> 8);
    myG[1]=((77 *myA[3] + 150 * myA[4]+ 29 * myA[5]) >> 8);
    myG[2]=((77 *myA[6] + 150 * myA[7]+ 29 * myA[8]) >> 8);
    myG[3]=((77 *myA[9] + 150 * myA[10]+ 29 * myA[11]) >> 8);
    G[global_index]=myG[0];
    G[global_index+1]=myG[1];
    G[global_index+2]=myG[2];
    G[global_index+3]=myG[3];

    global_index+=4;
    myA[0]=A[global_index*3];
    myA[1]=A[global_index*3+1];
    myA[2]=A[global_index*3+2];
    myA[3]=A[global_index*3+3];
    myA[4]=A[global_index*3+4];
    myA[5]=A[global_index*3+5];
    myA[6]=A[global_index*3+6];
    myA[7]=A[global_index*3+7];
    myA[8]=A[global_index*3+8];
    myA[9]=A[global_index*3+9];
    myA[10]=A[global_index*3+10];
    myA[11]=A[global_index*3+11];
    myG[0]=((77 *myA[0] + 150 * myA[1]+ 29 * myA[2]) >> 8);
    myG[1]=((77 *myA[3] + 150 * myA[4]+ 29 * myA[5]) >> 8);
    myG[2]=((77 *myA[6] + 150 * myA[7]+ 29 * myA[8]) >> 8);
    myG[3]=((77 *myA[9] + 150 * myA[10]+ 29 * myA[11]) >> 8);
    G[global_index]=myG[0];
    G[global_index+1]=myG[1];
    G[global_index+2]=myG[2];
    G[global_index+3]=myG[3];
  }
  //wait for global grey processing down

  bsg_barrier_hw_tile_group_sync();

  //apply sobel filter
  short top_left;
  short top;
  short top_right;
  short left;
  short right;
  short bot_left;
  short bot;
  short bot_right;
  short grad_x_local;
  short grad_y_local;
  short lxx_local;
  short lyy_local;
  short lxy_local;

  for(int y=0;y<y_tile_len;y++){
    if(((y+init_index_y)==0) || ((y+init_index_y)==N_Y-1)) continue;
    for(int x=0;x<x_tile_len;x++){
      if(((x+init_index_x)==0) || ((x+init_index_x)==N_X-1)) continue;

      int global_index=x+init_index_x+(y+init_index_y)*N_X;

      int g_top_left_index=(x+init_index_x-1)+(y+init_index_y-1)*N_X;
      int g_top_index=(x+init_index_x)+(y+init_index_y-1)*N_X;
      int g_top_right_index=(x+init_index_x+1)+(y+init_index_y-1)*N_X;
      int g_left_index=(x+init_index_x-1)+(y+init_index_y)*N_X;

      int g_right_index=(x+init_index_x+1)+(y+init_index_y)*N_X;
      int g_bot_left_index=(x+init_index_x-1)+(y+init_index_y+1)*N_X;
      int g_bot_index=x+init_index_x+(y+init_index_y+1)*N_X;
      int g_bot_right_index=(x+init_index_x+1)+(y+init_index_y+1)*N_X;

      top_left=G[g_top_left_index];
      top=G[g_top_index];
      top_right=G[g_top_right_index];
      left=G[g_left_index];
      right=G[g_right_index];
      bot_left=G[g_bot_left_index];
      bot=G[g_bot_index];
      bot_right=G[g_bot_right_index];

      grad_x_local=top_right-top_left-2*left+2*right-bot_left+bot_right;
      if(grad_x_local<-180) grad_x_local=-180;
      else if(grad_x_local>180) grad_x_local=180;

      grad_y_local=top_left+2*top+top_right-bot_left-2*bot-bot_right;
      if(grad_y_local<-180) grad_y_local=-180;
      else if(grad_y_local>180) grad_y_local=180;

      lxx_local=grad_x_local*grad_x_local>>6; //lxx
      lyy_local=grad_y_local*grad_y_local>>6; //lxx
      lxy_local=grad_x_local*grad_y_local>>6; //lxx
      lxx[global_index]=lxx_local;
      lyy[global_index]=lyy_local;
      lxy[global_index]=lxy_local;
    }
  }

  bsg_barrier_hw_tile_group_sync();
  
  //short cim_local[(x_tile_len)*(y_tile_len)]={0};
  short lgxx8;
  short lgyy8;
  short lgxy8;

  register short int lxxyy[9];

  for(int y=0;y<y_tile_len;y++){
    if(((y+init_index_y)<2) || ((y+init_index_y)>N_Y-3)) continue;
    for(int x=0;x<x_tile_len;x++){
      if(((x+init_index_x)<2) || ((x+init_index_x)>N_X-3)) continue;
      int global_index=x+init_index_x+(y+init_index_y)*N_X;

      int g_top_left_index=(x+init_index_x-1)+(y+init_index_y-1)*N_X;
      int g_top_index=(x+init_index_x)+(y+init_index_y-1)*N_X;
      int g_top_right_index=(x+init_index_x+1)+(y+init_index_y-1)*N_X;
      int g_left_index=(x+init_index_x-1)+(y+init_index_y)*N_X;
      int g_middle_index=(x+init_index_x)+(y+init_index_y)*N_X;
      int g_right_index=(x+init_index_x+1)+(y+init_index_y)*N_X;
      int g_bot_left_index=(x+init_index_x-1)+(y+init_index_y+1)*N_X;
      int g_bot_index=x+init_index_x+(y+init_index_y+1)*N_X;
      int g_bot_right_index=(x+init_index_x+1)+(y+init_index_y+1)*N_X;

      lxxyy[0]=lxx[g_top_left_index];
      lxxyy[1]=lxx[g_top_index];
      lxxyy[2]=lxx[g_top_right_index];
      lxxyy[3]=lxx[g_left_index];
      lxxyy[4]=lxx[g_middle_index];
      lxxyy[5]=lxx[g_right_index];
      lxxyy[6]=lxx[g_bot_left_index];
      lxxyy[7]=lxx[g_bot_index];
      lxxyy[8]=lxx[g_bot_right_index];

      lgxx8 = lxxyy[0];
      lgxx8 +=lxxyy[1];
      lgxx8 +=lxxyy[2];
      lgxx8 +=lxxyy[3];
      lgxx8 +=lxxyy[4];
      lgxx8 +=lxxyy[5];
      lgxx8 +=lxxyy[6];
      lgxx8 +=lxxyy[7];
      lgxx8 +=lxxyy[8];
      lgxx8 =lgxx8>>6;

      lxxyy[0]=lyy[g_top_left_index];
      lxxyy[1]=lyy[g_top_index];
      lxxyy[2]=lyy[g_top_right_index];
      lxxyy[3]=lyy[g_left_index];
      lxxyy[4]=lyy[g_middle_index];
      lxxyy[5]=lyy[g_right_index];
      lxxyy[6]=lyy[g_bot_left_index];
      lxxyy[7]=lyy[g_bot_index];
      lxxyy[8]=lyy[g_bot_right_index];

      lgyy8= lxxyy[0];
      lgyy8+=lxxyy[1];
      lgyy8+=lxxyy[2];
      lgyy8+=lxxyy[3];
      lgyy8+=lxxyy[4];
      lgyy8+=lxxyy[5];
      lgyy8+=lxxyy[6];
      lgyy8+=lxxyy[7];
      lgyy8+=lxxyy[8];
      lgyy8=lgyy8>>6;

      lxxyy[0]=lxy[g_top_left_index];
      lxxyy[1]=lxy[g_top_index];
      lxxyy[2]=lxy[g_top_right_index];
      lxxyy[3]=lxy[g_left_index];
      lxxyy[4]=lxy[g_middle_index];
      lxxyy[5]=lxy[g_right_index];
      lxxyy[6]=lxy[g_bot_left_index];
      lxxyy[7]=lxy[g_bot_index];
      lxxyy[8]=lxy[g_bot_right_index];

      lgxy8= lxxyy[0];
      lgxy8+=lxxyy[1];
      lgxy8+=lxxyy[2];
      lgxy8+=lxxyy[3];
      lgxy8+=lxxyy[4];
      lgxy8+=lxxyy[5];
      lgxy8+=lxxyy[6];
      lgxy8+=lxxyy[7];
      lgxy8+=lxxyy[8];
      lgxy8=lgxy8>>6;
      short int det=lgxx8*lgyy8 - lgxy8*lgxy8;
      cim[global_index] = (short) det- ((lgxx8+lgyy8)*(lgxx8+lgyy8) >> 4);
    }
  }
 
  bsg_barrier_hw_tile_group_sync();

  for(int y=0;y<y_tile_len;y++){
    if(((y+init_index_y)<3) || ((y+init_index_y)>N_Y-4)) continue;
    for(int x=0;x<x_tile_len;x++){
      if(((x+init_index_x)<3) || ((x+init_index_x)>N_X-4)) continue;
      int is_max;

      int global_index=x+init_index_x+(y+init_index_y)*N_X;

      int g_top_left_index=(x+init_index_x-1)+(y+init_index_y-1)*N_X;
      int g_top_index=(x+init_index_x)+(y+init_index_y-1)*N_X;
      int g_top_right_index=(x+init_index_x+1)+(y+init_index_y-1)*N_X;
      int g_left_index=(x+init_index_x-1)+(y+init_index_y)*N_X;
      int g_right_index=(x+init_index_x+1)+(y+init_index_y)*N_X;
      int g_bot_left_index=(x+init_index_x-1)+(y+init_index_y+1)*N_X;
      int g_bot_index=x+init_index_x+(y+init_index_y+1)*N_X;
      int g_bot_right_index=(x+init_index_x+1)+(y+init_index_y+1)*N_X;

      int current_cim=cim[global_index];

      if(current_cim>1){
        is_max=current_cim >= cim[g_top_left_index] && current_cim >= cim[g_top_index] &&
          current_cim >= cim[g_left_index] && current_cim >= cim[g_right_index] &&
          current_cim >= cim[g_bot_left_index] && current_cim >= cim[g_bot_index] &&
          current_cim >= cim[g_bot_right_index] && current_cim >= cim[g_top_right_index];
        Output[global_index]=is_max ? 255:0;
      } else Output[global_index]=0;
    }
  }

  //write_output_back
  //for(int y=0;y<tile_image_size_y-4;y++){
  //  for(int x=0;x<tile_image_size_x-4;x++){
  //    int local_inline_index=(x+2)+(y+2)*tile_image_size_x;
  //    int global_x=x+init_index_x;
  //    int global_y=y+init_index_y;
  //    int global_index=global_x+global_y*N_X;
  //    Output[global_index]=output_local[local_inline_index];
  //  }
  //}

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}

