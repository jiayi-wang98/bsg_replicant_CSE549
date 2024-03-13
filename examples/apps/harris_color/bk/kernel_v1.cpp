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
      //asm volatile ("sw x0, %[p]" :: [p] "m" (cim[global_index]));
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
  int tile_image_size_x=x_tile_len+6;
  int tile_image_size_y=y_tile_len+6;

  int total_pixel_num=(x_tile_len+6)*(y_tile_len+6);
  //calculate gray scale image
  for (int x = -3; x < x_tile_len+3; x++) {
    for (int y = -3; y < y_tile_len+3; y++) {
      int global_x=x+init_index_x;
      int global_y=y+init_index_y;
      int global_index=global_x+global_y*N_X;
      if((global_x<0) || (global_x>=N_X)){
        continue;
      } else if((global_y<0) || (global_y>=N_Y)){
        continue;
      } else{
        G[global_index]=(short int) ((77 *A[global_index*3] + 150 * A[global_index*3+1]+ 29 * A[global_index*3+2]) >> 8);
      }
    }
  }

  //wait for global grey processing down
  bsg_fence();

  //read back local piece of image (x+4, y+4)
  short gray[total_pixel_num]={0};
  //if the tile is at the left/right/top/bottom boundary
  if((__bsg_id%bsg_tiles_X==0) || (__bsg_id%bsg_tiles_X==(bsg_tiles_X-1))|| (__bsg_id/bsg_tiles_X==0)|| (__bsg_id/bsg_tiles_X==(bsg_tiles_Y-1))){
    for(int x=0;x<tile_image_size_x;x++){
      for(int y=0;y<tile_image_size_y;y++){
        int local_middle_index=x+y*tile_image_size_x;
        int global_x=x-3+init_index_x;
        int global_y=y-3+init_index_y;
        int global_index=global_x+global_y*N_X;
        if(global_x<0) {
          //top left corner
          if(global_y<0) gray[local_middle_index]=G[0];
          //bot left corner
          else if(global_y>N_Y-1) gray[local_middle_index]=G[(N_Y-1)*N_X];
          //left boundary
          else gray[local_middle_index]=G[global_y*N_X];
        } else if (global_x>N_X-1) {
          //bot right corner
          if(global_y>N_Y-1) gray[local_middle_index]=G[N_X*N_Y-1];
          //top right corner
          else if(global_y<0) gray[local_middle_index]=G[N_X-1];
          //right boundary
          else gray[local_middle_index]=G[N_X-1+global_y*N_X];
        }
        //top boundary
        else if(global_y<0) gray[local_middle_index]=G[global_x];
        //bot boundary
        else if(global_y>N_Y-1) gray[local_middle_index]=G[global_x+(N_Y-1)*N_X];
        //normal
        else gray[local_middle_index]=G[global_index];
      }
    }
  } else {
    for(int x=0;x<tile_image_size_x;x++){
      for(int y=0;y<tile_image_size_y;y++){
        int local_middle_index=x+y*tile_image_size_x;
        int global_x=x-3+init_index_x;
        int global_y=y-3+init_index_y;
        int global_index=global_x+global_y*N_X;
        gray[local_middle_index]=G[global_index];
      }
    }
  }

//test only
/*
  for (int x = 0; x < x_tile_len; x++) {
    for (int y = 0; y < y_tile_len; y++) {
      int global_x=x-2+init_index_x;
      int global_y=y-2+init_index_y;
      int global_index=global_x+global_y*N_X;
      int local_middle_index=x+y*tile_image_size_x;
      if((global_x<0) || (global_x>=N_X)){
        continue;
      } else if((global_y<0) || (global_y>=N_Y)){
        continue;
      } else{
        G[global_index]=gray[local_middle_index];
      }
    }
  }
*/
//test end


  short grad_x[total_pixel_num]={0};
  short grad_y[total_pixel_num]={0};


  //apply sobel filter
  short int kernel_x[9]={-1,0,1,-2,0,2,-1,0,1};
  short int kernel_y[9]={1,2,1,0,0,0,-1,-2,-1};

  //calculate convolution

  for(int x=0;x<tile_image_size_x;x++){
    for(int y=0;y<tile_image_size_y;y++){
      if(x==0|y==0|(x==tile_image_size_x-1)|(y==tile_image_size_y-1)){
        grad_x[x+y*tile_image_size_x]=0; 
        continue;
      } 
      int top_left_index=(x-1)+(y-1)*tile_image_size_x;
      int top_index=(x)+(y-1)*tile_image_size_x;
      int top_right_index=(x+1)+(y-1)*tile_image_size_x;
      int left_index=(x-1)+y*tile_image_size_x;
      int middle_index=(x)+y*tile_image_size_x;
      int right_index=(x+1)+y*tile_image_size_x;
      int bot_left_index=(x-1)+(y+1)*tile_image_size_x;
      int bot_index=x+(y+1)*tile_image_size_x;
      int bot_right_index=(x+1)+(y+1)*tile_image_size_x;
      grad_x[middle_index]=gray[top_left_index]*kernel_x[0];
      grad_x[middle_index]+=gray[top_index]*kernel_x[1];
      grad_x[middle_index]+=gray[top_right_index]*kernel_x[2];
      grad_x[middle_index]+=gray[left_index]*kernel_x[3];
      grad_x[middle_index]+=gray[middle_index]*kernel_x[4];
      grad_x[middle_index]+=gray[right_index]*kernel_x[5];
      grad_x[middle_index]+=gray[bot_left_index]*kernel_x[6];
      grad_x[middle_index]+=gray[bot_index]*kernel_x[7];
      grad_x[middle_index]+=gray[bot_right_index]*kernel_x[8];

      grad_y[middle_index]=gray[top_left_index]*kernel_y[0];
      grad_y[middle_index]+=gray[top_index]*kernel_y[1];
      grad_y[middle_index]+=gray[top_right_index]*kernel_y[2];
      grad_y[middle_index]+=gray[left_index]*kernel_y[3];
      grad_y[middle_index]+=gray[middle_index]*kernel_y[4];
      grad_y[middle_index]+=gray[right_index]*kernel_y[5];
      grad_y[middle_index]+=gray[bot_left_index]*kernel_y[6];
      grad_y[middle_index]+=gray[bot_index]*kernel_y[7];
      grad_y[middle_index]+=gray[bot_right_index]*kernel_y[8];
    }
  }

  //clamp to make sure 180^2 does not exceed 15bits
  for(int x=0;x<tile_image_size_x;x++){
    for(int y=0;y<tile_image_size_y;y++){
      int middle_index=(x)+y*tile_image_size_x;
      if(grad_x[middle_index]<-180) grad_x[middle_index]=-180;
      else if(grad_x[middle_index]>180) grad_x[middle_index]=180;
      if(grad_y[middle_index]<-180) grad_y[middle_index]=-180;
      else if(grad_y[middle_index]>180) grad_y[middle_index]=180;
    }
  }

  //calculate product
  //short lxx[total_pixel_num]={0};
  //short lyy[total_pixel_num]={0};
  //short lxy[total_pixel_num]={0};
  for(int x=3;x<tile_image_size_x-3;x++){
    for(int y=3;y<tile_image_size_y-3;y++){
      int middle_index=(x)+y*tile_image_size_x;
      int global_x=x-3+init_index_x;
      int global_y=y-3+init_index_y;
      int global_index=global_x+global_y*N_X;
      lxx[global_index]=grad_x[middle_index]*grad_x[middle_index]>>6;
      lyy[global_index]=grad_y[middle_index]*grad_y[middle_index]>>6;
      lxy[global_index]=grad_x[middle_index]*grad_y[middle_index]>>6;
    }
  }

  bsg_fence();
  
  //window size 3x3
  //calculate cim
  short cim_local[total_pixel_num]={0};
  short lgxx8;
  short lgyy8;
  short lgxy8;
  for(int x=2;x<tile_image_size_x-2;x++){
    for(int y=2;y<tile_image_size_y-2;y++){
      int global_x=x-3+init_index_x;
      int global_y=y-3+init_index_y;
      //clamp index
      int global_x_left=(global_x-1)<0? 0:global_x-1;
      int global_x_mid=(global_x)<0? 0:(global_x>N_X-1)? N_X-1:global_x;
      int global_x_right=(global_x+1>N_X-1)? N_X-1:global_x+1;

      int global_y_top=(global_y-1)<0? 0:global_y-1;
      int global_y_mid=(global_y)<0? 0:(global_y>N_Y-1)? N_Y-1:global_y;
      int global_y_bot=(global_y+1>N_Y-1)? N_Y-1:global_y+1;

      int local_middle_index=x+y*tile_image_size_x;

      int top_left_index=global_x_left+global_y_top*N_X;
      int top_index=(global_x_mid)+global_y_top*N_X;
      int top_right_index=(global_x_right)+global_y_top*N_X;
      int left_index=global_x_left+global_y_mid*N_X;
      int middle_index=(global_x_mid)+global_y_mid*N_X;
      int right_index=(global_x_right)+global_y_mid*N_X;
      int bot_left_index=global_x_left+(global_y_bot)*N_X;
      int bot_index=global_x_mid+(global_y_bot)*N_X;
      int bot_right_index=(global_x_right)+(global_y_bot)*N_X;
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
/*


//test only

  for(int x=0;x<tile_image_size_x-6;x++){
    for(int y=0;y<tile_image_size_y-6;y++){
      int local_inline_index=(x+3)+(y+3)*tile_image_size_x;
      int global_x=x+init_index_x;
      int global_y=y+init_index_y;
      int global_index=global_x+global_y*N_X;
      cim[global_index]=cim_local[local_inline_index];
    }
  }

//testend
*/
  //check local max
  //short output_local[total_pixel_num]={0};
  for(int x=0;x<tile_image_size_x-6;x++){
    for(int y=0;y<tile_image_size_y-6;y++){
      int is_max;

      int local_inline_index=(x+3)+(y+3)*tile_image_size_x;
      int current_cim=cim_local[local_inline_index];

      int global_x=x+init_index_x;
      int global_y=y+init_index_y;
      int global_index=global_x+global_y*N_X;

      int top_left_index=(x+2)+(y+2)*tile_image_size_x;
      int top_index=(x+3)+(y+2)*tile_image_size_x;
      int top_right_index=(x+4)+(y+2)*tile_image_size_x;
      int left_index=(x+2)+(y+3)*tile_image_size_x;
      
      int right_index=(x+4)+(y+3)*tile_image_size_x;
      int bot_left_index=(x+2)+(y+4)*tile_image_size_x;
      int bot_index=(x+3)+(y+4)*tile_image_size_x;
      int bot_right_index=(x+4)+(y+4)*tile_image_size_x;

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

