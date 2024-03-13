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
  register short int myA[12];
  register short int myG[4];
  //calculate gray scale image
  for (int x = 0; x < N_X*y_tile_len; x+=4) {
      int global_index=init_index+x;
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

      myG[0]=(short int) ((77 *myA[0] + 150 * myA[1]+ 29 * myA[2]) >> 8);
      myG[1]=(short int) ((77 *myA[3] + 150 * myA[4]+ 29 * myA[5]) >> 8);
      myG[2]=(short int) ((77 *myA[6] + 150 * myA[7]+ 29 * myA[8]) >> 8);
      myG[3]=(short int) ((77 *myA[9] + 150 * myA[10]+ 29 * myA[11]) >> 8);

      G[global_index]=myG[0];
      G[global_index+1]=myG[1];
      G[global_index+2]=myG[2];
      G[global_index+3]=myG[3];
  }

  //wait for global grey processing down
  bsg_fence();

  //read back local piece of image (x, y+6)
  short gray[total_pixel_num]={0};
  //if the tile is at the left/right/top/bottom boundary

  for(int y=0;y<tile_image_size_y;y++){
    for(int x=0;x<tile_image_size_x;x++){
      int local_middle_index=x+y*tile_image_size_x;
      int global_x=x+init_index_x;
      int global_y=y-3+init_index_y;
      //top boundary
      if(global_y<0) global_y=0;
      //bot boundary
      else if(global_y>N_Y-1) global_y=N_Y-1;

      int global_index=global_x+global_y*N_X;
      gray[local_middle_index]=G[global_index];
    }
  }
  



  short grad_x[(x_tile_len)*(y_tile_len+4)]={0};
  short grad_y[(x_tile_len)*(y_tile_len+4)]={0};


  //apply sobel filter
  short int kernel_x[9]={-1,0,1,-2,0,2,-1,0,1};
  short int kernel_y[9]={1,2,1,0,0,0,-1,-2,-1};

  //calculate convolution
  for(int y=0;y<y_tile_len+4;y++){
    for(int x=0;x<tile_image_size_x;x++){
      if(x==0|(x==tile_image_size_x-1)){
        grad_x[x+y*tile_image_size_x]=0; 
        grad_y[x+y*tile_image_size_x]=0;
        continue;
      } 
      int local_middle_index=(x)+y*tile_image_size_x;

      int top_left_index=(x-1)+(y)*tile_image_size_x;
      int top_index=(x)+(y)*tile_image_size_x;
      int top_right_index=(x+1)+(y)*tile_image_size_x;
      int left_index=(x-1)+(y+1)*tile_image_size_x;
      //int middle_index=(x)+(y+1)*tile_image_size_x;
      int right_index=(x+1)+(y+1)*tile_image_size_x;
      int bot_left_index=(x-1)+(y+2)*tile_image_size_x;
      int bot_index=x+(y+2)*tile_image_size_x;
      int bot_right_index=(x+1)+(y+2)*tile_image_size_x;
      grad_x[local_middle_index]=gray[top_left_index]*kernel_x[0];
      //grad_x[local_middle_index]+=gray[top_index]*kernel_x[1];
      grad_x[local_middle_index]+=gray[top_right_index]*kernel_x[2];
      grad_x[local_middle_index]+=gray[left_index]*kernel_x[3];
      //grad_x[local_middle_index]+=gray[middle_index]*kernel_x[4];
      grad_x[local_middle_index]+=gray[right_index]*kernel_x[5];
      grad_x[local_middle_index]+=gray[bot_left_index]*kernel_x[6];
      //grad_x[local_middle_index]+=gray[bot_index]*kernel_x[7];
      grad_x[local_middle_index]+=gray[bot_right_index]*kernel_x[8];
      if(grad_x[local_middle_index]<-180) grad_x[local_middle_index]=-180;
      else if(grad_x[local_middle_index]>180) grad_x[local_middle_index]=180;

      grad_y[local_middle_index]=gray[top_left_index]*kernel_y[0];
      grad_y[local_middle_index]+=gray[top_index]*kernel_y[1];
      grad_y[local_middle_index]+=gray[top_right_index]*kernel_y[2];
      //grad_y[local_middle_index]+=gray[left_index]*kernel_y[3];
      //grad_y[local_middle_index]+=gray[middle_index]*kernel_y[4];
      //grad_y[local_middle_index]+=gray[right_index]*kernel_y[5];
      grad_y[local_middle_index]+=gray[bot_left_index]*kernel_y[6];
      grad_y[local_middle_index]+=gray[bot_index]*kernel_y[7];
      grad_y[local_middle_index]+=gray[bot_right_index]*kernel_y[8];
      if(grad_y[local_middle_index]<-180) grad_y[local_middle_index]=-180;
      else if(grad_y[local_middle_index]>180) grad_y[local_middle_index]=180;
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
  
  for(int y=0;y<y_tile_len+4;y++){
    for(int x=1;x<tile_image_size_x-1;x++){
      int middle_index=(x)+y*tile_image_size_x;
      gray[middle_index]=grad_x[middle_index]*grad_x[middle_index]>>6; //lxx
      grad_x[middle_index]=grad_x[middle_index]*grad_y[middle_index]>>6; //lxy
      grad_y[middle_index]=grad_y[middle_index]*grad_y[middle_index]>>6; //lyy
      
    }
  }

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

      lgxx8 = gray[top_left_index];
      lgxx8 +=gray[top_index];
      lgxx8 +=gray[top_right_index];
      lgxx8 +=gray[left_index];
      lgxx8 +=gray[middle_index];
      lgxx8 +=gray[right_index];
      lgxx8 +=gray[bot_left_index];
      lgxx8 +=gray[bot_index];
      lgxx8 +=gray[bot_right_index];
      lgxx8 =lgxx8>>6;

      lgyy8= grad_y[top_left_index];
      lgyy8+=grad_y[top_index];
      lgyy8+=grad_y[top_right_index];
      lgyy8+=grad_y[left_index];
      lgyy8+=grad_y[middle_index];
      lgyy8+=grad_y[right_index];
      lgyy8+=grad_y[bot_left_index];
      lgyy8+=grad_y[bot_index];
      lgyy8+=grad_y[bot_right_index];
      lgyy8=lgyy8>>6;

      lgxy8= grad_x[top_left_index];
      lgxy8+=grad_x[top_index];
      lgxy8+=grad_x[top_right_index];
      lgxy8+=grad_x[left_index];
      lgxy8+=grad_x[middle_index];
      lgxy8+=grad_x[right_index];
      lgxy8+=grad_x[bot_left_index];
      lgxy8+=grad_x[bot_index];
      lgxy8+=grad_x[bot_right_index];
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

