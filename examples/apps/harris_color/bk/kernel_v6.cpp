#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

#ifdef WARM_CACHE
__attribute__((noinline))
static void warmup(short *A, short *G,short *Output,int N_X, int N_Y)
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
kernel_harris_color(short * A, short *G,short *Output, int N_X, int N_Y) {

  bsg_barrier_hw_tile_group_init();
#ifdef WARM_CACHE
  warmup(A, G, Output, N_X,N_Y);
#endif
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  //original program
  // Each tile does a portion of vector_add
  int x_tile_len = N_X ; //64
  int y_tile_len = N_Y / bsg_tiles_Y /bsg_tiles_X; //64/32=2

  int Cinit;//image tile initial
  //up plane
  if(__bsg_id/16<1){
    //left plane
    if(__bsg_id%8<4) {
        Cinit=((__bsg_id/8)*4+(__bsg_id%8))*16;
    } else {
        Cinit=((__bsg_id/8)*4+(__bsg_id%8-4))*16+4;
    }	
  } else { // bot plane
  //left plane
      if(__bsg_id%8<4) {
          Cinit=((__bsg_id/8-2)*4+(__bsg_id%8))*16+8;
      } else {
          Cinit=((__bsg_id/8-2)*4+(__bsg_id%8-4))*16+8+4;
      }
  }

  int init_index_x=0;
  int init_index_y=(Cinit/2);
  int init_index=init_index_y*N_X;
  //int init_index_inline=init_index_x+init_index_y*N_X;
  int tile_image_size_x=x_tile_len;
  int tile_image_size_y=y_tile_len+6;

  int total_pixel_num=(x_tile_len)*(y_tile_len+6);
  //calculate gray scale image
  for (int x = 0; x < N_X*y_tile_len; x++) {
      int global_index=init_index+x;
      G[global_index]=(short int) ((77 *A[global_index*3] + 150 * A[global_index*3+1]+ 29 * A[global_index*3+2]) >> 8);
  }

  //wait for global grey processing down
  bsg_fence();

  //read back local piece of image (x, y+6)
  //short gray[total_pixel_num]={0};
  //if the tile is at the left/right/top/bottom boundary
/*
  for(int y=0;y<tile_image_size_y;y++){
    int global_y=y-3+init_index_y;
    //top boundary
    if(global_y<0) global_y=0;
    //bot boundary
    else if(global_y>N_Y-1) global_y=N_Y-1;

    for(int x=0;x<tile_image_size_x;x+=8){
      int local_middle_index=x+y*tile_image_size_x;
      int global_x=x+init_index_x;
      int global_index=global_x+global_y*N_X;
      gray[local_middle_index]=G[global_index];
    }
  }
  */



  short lxx[(x_tile_len)*(y_tile_len+4)]={0};
  short lyy[(x_tile_len)*(y_tile_len+4)]={0};
  short lxy[(x_tile_len)*(y_tile_len+4)]={0};


  //apply sobel filter
  //short int kernel_x[9]={-1,0,1,-2,0,2,-1,0,1};
  //short int kernel_y[9]={1,2,1,0,0,0,-1,-2,-1};

  //calculate convolution
  for(int y=0;y<y_tile_len+4;y++){
    for(int x=0;x<tile_image_size_x;x++){
      if(x==0|(x==tile_image_size_x-1)){
        lxx[x+y*tile_image_size_x]=0; 
        lyy[x+y*tile_image_size_x]=0;
        lxy[x+y*tile_image_size_x]=0;
        continue;
      } 
      int local_middle_index=(x)+y*tile_image_size_x;

      int g_top_left_index=(x-1)+(y+init_index_y-3)*N_X;
      int g_top_index=(x)+(y+init_index_y-3)*N_X;
      int g_top_right_index=(x+1)+(y+init_index_y-3)*N_X;
      int g_left_index=(x-1)+(y+init_index_y-2)*N_X;
      //int middle_index=(x)+(y+1)*N_X;
      int g_right_index=(x+1)+(y+init_index_y-2)*N_X;
      int g_bot_left_index=(x-1)+(y+init_index_y-1)*N_X;
      int g_bot_index=x+(y+init_index_y-1)*N_X;
      int g_bot_right_index=(x+1)+(y+init_index_y-1)*N_X;

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

      lxx[local_middle_index]=grad_x_local*grad_x_local>>6; //lxx
      lyy[local_middle_index]=grad_y_local*grad_y_local>>6; //lxx
      lxy[local_middle_index]=grad_x_local*grad_y_local>>6; //lxx
    }
  }

  //clamp to make sure 180^2 does not exceed 15bits
  //for(int x=0;x<tile_image_size_x;x++){
  //  for(int y=0;y<tile_image_size_y;y++){
  //    int middle_index=(x)+y*tile_image_size_x;
  //    if(grad_x[middle_index]<-180) grad_x[middle_index]=-180;
  //    else if(grad_x[middle_index]>180) grad_x[middle_index]=180;
  //    if(grad_y[middle_index]<-180) grad_y[middle_index]=-180;
  //    else if(grad_y[middle_index]>180) grad_y[middle_index]=180;
  //  }
  //}

  //calculate product
  //short lxx[total_pixel_num]={0};use gray
  //short lxy[total_pixel_num]={0};use grad_x
  //short lyy[total_pixel_num]={0};use grad_y
  
  //for(int x=1;x<tile_image_size_x-1;x++){
  //  for(int y=0;y<y_tile_len+4;y++){
  //    int middle_index=(x)+y*tile_image_size_x;
  //    lxx[middle_index]=grad_x[middle_index]*grad_x[middle_index]>>6; //lxx
  //    lxy[middle_index]=grad_x[middle_index]*grad_y[middle_index]>>6; //lxy
  //    lyy[middle_index]=grad_y[middle_index]*grad_y[middle_index]>>6; //lyy
  //    
  //  }
  //}

  //bsg_fence();

  //window size 3x3
  //calculate cim
     
  short cim_local[(x_tile_len)*(y_tile_len+2)]={0};
  short lgxx8;
  short lgyy8;
  short lgxy8;

  for(int y=0;y<y_tile_len+2;y++){
    for(int x=2;x<tile_image_size_x-2;x++){
      int local_middle_index=x+(y)*tile_image_size_x;

      int top_left_index=(x-1)+(y)*tile_image_size_x;
      int top_index=x+(y)*tile_image_size_x;
      int top_right_index=(x+1)+(y)*tile_image_size_x;
      int left_index=(x-1)+(y+1)*tile_image_size_x;
      int middle_index=x+(y+1)*tile_image_size_x;
      int right_index=(x+1)+(y+1)*tile_image_size_x;
      int bot_left_index=(x-1)+(y+2)*tile_image_size_x;
      int bot_index=x+(y+2)*tile_image_size_x;
      int bot_right_index=(x+1)+(y+2)*tile_image_size_x;

      lgxx8 = lxx[top_left_index];
      lgxx8 +=lxx[top_index];
      lgxx8 +=lxx[top_right_index];
      lgxx8 +=lxx[left_index];
      lgxx8 +=lxx[middle_index];
      lgxx8 +=lxx[right_index];
      lgxx8 +=lxx[bot_left_index];
      lgxx8 +=lxx[bot_index];
      lgxx8 +=lxx[bot_right_index];
      lgxx8 =lgxx8>>6;

      lgyy8= lyy[top_left_index];
      lgyy8+=lyy[top_index];
      lgyy8+=lyy[top_right_index];
      lgyy8+=lyy[left_index];
      lgyy8+=lyy[middle_index];
      lgyy8+=lyy[right_index];
      lgyy8+=lyy[bot_left_index];
      lgyy8+=lyy[bot_index];
      lgyy8+=lyy[bot_right_index];
      lgyy8=lgyy8>>6;

      lgxy8= lxy[top_left_index];
      lgxy8+=lxy[top_index];
      lgxy8+=lxy[top_right_index];
      lgxy8+=lxy[left_index];
      lgxy8+=lxy[middle_index];
      lgxy8+=lxy[right_index];
      lgxy8+=lxy[bot_left_index];
      lgxy8+=lxy[bot_index];
      lgxy8+=lxy[bot_right_index];
      lgxy8=lgxy8>>6;

      short int det=lgxx8*lgyy8 - lgxy8*lgxy8;
      cim_local[local_middle_index] = (short) det- ((lgxx8+lgyy8)*(lgxx8+lgyy8) >> 4);
    
    }
  }
 



  //check local max
  //short output_local[total_pixel_num]={0};
  for(int y=0;y<y_tile_len;y++){
    for(int x=3;x<tile_image_size_x-3;x++){
      int is_max;

      int local_inline_index=(x)+(y+1)*tile_image_size_x;
      int current_cim=cim_local[local_inline_index];

      int global_x=x+init_index_x;
      int global_y=y+init_index_y;
      int global_index=global_x+global_y*N_X;

      int top_left_index=(x-1)+(y)*tile_image_size_x;
      int top_index=(x)+(y)*tile_image_size_x;
      int top_right_index=(x+1)+(y)*tile_image_size_x;
      int left_index=(x-1)+(y+1)*tile_image_size_x;
      
      int right_index=(x+1)+(y+1)*tile_image_size_x;
      int bot_left_index=(x-1)+(y+2)*tile_image_size_x;
      int bot_index=(x)+(y+2)*tile_image_size_x;
      int bot_right_index=(x+1)+(y+2)*tile_image_size_x;

      if(current_cim>1){
        is_max=current_cim >= cim_local[top_left_index] && current_cim >= cim_local[top_index] &&
          current_cim >= cim_local[left_index] && current_cim >= cim_local[right_index] &&
          current_cim >= cim_local[bot_left_index] && current_cim >= cim_local[bot_index] &&
          current_cim >= cim_local[bot_right_index] && current_cim >= cim_local[top_right_index];
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

