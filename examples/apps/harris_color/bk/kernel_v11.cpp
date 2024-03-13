#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include "bsg_mcs_mutex.h"
#include <algorithm>

#ifdef WARM_CACHE
__attribute__((noinline))
static void warmup(short *A, short *Output,int N_X, int N_Y)
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
      asm volatile ("sw x0, %[p]" :: [p] "m" (Output[global_index]));
    }
  }
  bsg_fence();
}
#endif

//load data from local or global
__attribute__((inline))
void get_surrounding_data(int* ret, int* gm, int local_x,int local_y,int x_tile_len, int y_tile_len, volatile int *gm_top_left,volatile int *gm_up,volatile int *gm_top_right,volatile int* gm_le, volatile int* gm_rt, volatile int* gm_bot_left, volatile int * gm_dn, volatile int* gm_bot_right){

  int x=local_x;
  int y=local_y;

  int gm_top_left_index=(x-1)+(y-1)*x_tile_len;
  int gm_top_index=(x)+(y-1)*x_tile_len;
  int gm_top_right_index=(x+1)+(y-1)*x_tile_len;
  int gm_left_index=(x-1)+(y)*x_tile_len;
  int gm_middle_index=(x)+(y)*x_tile_len;
  int gm_right_index=(x+1)+(y)*x_tile_len;
  int gm_bot_left_index=(x-1)+(y+1)*x_tile_len;
  int gm_bot_index=x+(y+1)*x_tile_len;
  int gm_bot_right_index=(x+1)+(y+1)*x_tile_len;

  ret[0]=gm[gm_top_left_index];
  ret[1]=gm[gm_top_index];
  ret[2]=gm[gm_top_right_index];
  ret[3]=gm[gm_left_index];
  ret[4]=gm[gm_middle_index];
  ret[5]=gm[gm_right_index];
  ret[6]=gm[gm_bot_left_index];
  ret[7]=gm[gm_bot_index];
  ret[8]=gm[gm_bot_right_index];

  if(x==0){
    gm_top_left_index=x_tile_len-1+(y-1)*x_tile_len; //left tile's right column up one
    ret[0]=*(gm_le+gm_top_left_index);
    gm_left_index= x_tile_len-1+y*x_tile_len; //left tile's right most column
    ret[3]=*(gm_le+gm_left_index);
    gm_bot_left_index=x_tile_len-1+(y+1)*x_tile_len; //left tile's right column dn one
    ret[6]=*(gm_le+gm_bot_left_index);
    //top left corner
    if(y==0){
      gm_top_left_index=x_tile_len*y_tile_len-1; //up left tile's bot right corner
      ret[0]=*(gm_top_left+gm_top_left_index);
      gm_top_index=x+(y_tile_len-1)*x_tile_len; //up tile's last row
      ret[1]=*(gm_up+gm_top_index);
      gm_top_right_index=1+gm_top_index; //up tile's last row
      ret[2]=*(gm_up+gm_top_right_index);
    } else if (y==(y_tile_len-1)){
      gm_bot_left_index=x_tile_len-1; //bot left tile's up right corner
      ret[6]=*(gm_bot_left+gm_bot_left_index);
      gm_bot_index=x; //dn tile's first row
      ret[7]=*(gm_dn+gm_bot_index);
      gm_bot_right_index=1+gm_bot_index; //down tile's last row
      ret[8]=*(gm_dn+gm_bot_right_index);
    } 
  } else if(x==(x_tile_len-1)){
    //normally need data from left side
    gm_top_right_index=(y-1)*x_tile_len; //right tile's left column up one
    ret[2]=*(gm_rt+gm_top_right_index);
      
    gm_right_index= y*x_tile_len; //left tile's right most column
    ret[5]=*(gm_rt+gm_right_index);
    gm_bot_right_index=+(y+1)*x_tile_len; //left tile's right column up one
    ret[8]=*(gm_rt+gm_bot_right_index);
    //top right corner
    if(y==0){
      gm_top_right_index=x_tile_len*(y_tile_len-1); //up right tile's bot right corner
      ret[2]=*(gm_top_right+gm_top_right_index);
      
      gm_top_index=x+(y_tile_len-1)*x_tile_len; //up tile's last row
      ret[1]=*(gm_up+gm_top_index);
      gm_top_left_index=gm_top_index-1; //up tile's last row
      ret[0]=*(gm_up+gm_top_left_index);
      
    } else if (y==(y_tile_len-1)){
      gm_bot_right_index=0; //bot right tile's up left corner
      ret[8]=*(gm_bot_right+gm_bot_right_index);
      
      gm_bot_index=x; //dn tile's first row
      ret[7]=*(gm_dn+gm_bot_index);
      gm_bot_left_index=gm_bot_index-1; //down tile's last row
      ret[6]=*(gm_dn+gm_bot_left_index);
    }
  } 
  //need 1 line from upper core
  else if(y==0){
      gm_top_right_index=x+1+x_tile_len*(y_tile_len-1); //up tile's last row
      ret[2]=*(gm_up+gm_top_right_index);
      
      gm_top_index=x+x_tile_len*(y_tile_len-1); //up tile's last row
      ret[1]=*(gm_up+gm_top_index);
      gm_top_left_index=x-1+x_tile_len*(y_tile_len-1); //up tile's last row
      ret[0]=*(gm_up+gm_top_left_index);
  }//need 1 line from down core 
  else if(y==(y_tile_len-1)){
      gm_bot_right_index=x+1; //bot right tile's up left corner
      ret[8]=*(gm_dn+gm_bot_right_index);
      
      gm_bot_index=x; //dn tile's first row
      ret[7]=*(gm_dn+gm_bot_index);
      gm_bot_left_index=x-1; //down tile's last row
      ret[6]=*(gm_dn+gm_bot_left_index);
  }
} 


// harris_color: C = A + B
// N = # of frames
extern "C" __attribute__ ((noinline))
int
kernel_harris_color(short * A, short *Output, int N_X, int N_Y) {

  bsg_barrier_hw_tile_group_init();
#ifdef WARM_CACHE
  warmup(A,Output, N_X,N_Y);
#endif
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  //original program
  // Each tile does a portion of vector_add
  int x_tile_len = N_X/bsg_tiles_X ; //64/8=8
  int y_tile_len = N_Y/bsg_tiles_Y; //64/4=16


  //volatile int* gm = (int *)((__bsg_id%bsg_tiles_X)<<16 + (__bsg_id/bsg_tiles_X)<<23 + 1<<30) ;
  int bsg_id_x=__bsg_id% bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_y=__bsg_id/ bsg_tiles_X+bsg_tiles_Y;

  int init_index_x=(__bsg_id% bsg_tiles_X)*x_tile_len;
  int init_index_y=(__bsg_id/ bsg_tiles_X)*y_tile_len;
  int init_index=init_index_x+init_index_y*N_X;

  //volatile int *gm = reinterpret_cast<int*>(0x00000400);
  int gm_gray[x_tile_len*y_tile_len];
  int gm_lxx[x_tile_len*y_tile_len]; //0x00000600
  int gm_lyy[x_tile_len*y_tile_len]; //0x00000800
  int gm_lxy[x_tile_len*y_tile_len]; //0x00000a00
  int gm_cim[x_tile_len*y_tile_len]; //0x00000c00

  //bsg_print_int(reinterpret_cast<int>(gm_gray));
  //bsg_print_int(reinterpret_cast<int>(gm_lxx));
  //bsg_print_int(reinterpret_cast<int>(gm_lyy));
  //bsg_print_int(reinterpret_cast<int>(gm_lxy));
  //bsg_print_int(reinterpret_cast<int>(gm_cim));


  int bsg_id_up;
  if(__bsg_id<bsg_tiles_X) {
    bsg_id_up=__bsg_id;
  } else {
    bsg_id_up=__bsg_id-8;
  }

  int bsg_id_up_x=bsg_id_up % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_up_y=bsg_id_up / bsg_tiles_X+bsg_tiles_Y;

  int base0=bsg_id_up_x<<16;
  int base1=bsg_id_up_y<<23;
  int base2=0x40000000;
  int base3= reinterpret_cast<int>(gm_gray);
  int base4= reinterpret_cast<int>(gm_lxx);
  int base5= reinterpret_cast<int>(gm_lyy);
  int base6= reinterpret_cast<int>(gm_lxy);
  int base7= reinterpret_cast<int>(gm_cim);
  int base_address_up=base0+base1+base2+base3;//+bsg_id_up_x<<16+(bsg_id_up_y)<<23;
  int base_address_up_lxx=base0+base1+base2+base4;
  int base_address_up_lyy=base0+base1+base2+base5;
  int base_address_up_lxy=base0+base1+base2+base6;
  int base_address_up_cim=base0+base1+base2+base7;

  //bsg_print_int(base0);
  //bsg_print_int(base1);
  //bsg_print_int(base2);
  //bsg_print_int(base3);
  //bsg_print_int(base_address_up);

  volatile int *gm_gray_up=reinterpret_cast<int*>(base_address_up);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_up));
  volatile int *gm_lxx_up= reinterpret_cast<int*>(base_address_up_lxx);
  volatile int *gm_lyy_up= reinterpret_cast<int*>(base_address_up_lyy);
  volatile int *gm_lxy_up= reinterpret_cast<int*>(base_address_up_lxy);
  volatile int *gm_cim_up= reinterpret_cast<int*>(base_address_up_cim);

  //the coretile (x,y) processing the image tile below this core
  int bsg_id_dn;
  if(__bsg_id>=bsg_tiles_X*(bsg_tiles_Y-1)) {
    bsg_id_dn=__bsg_id;
  } else {
    bsg_id_dn=__bsg_id+8;
  }
  int bsg_id_dn_x=bsg_id_dn % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_dn_y=bsg_id_dn / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_dn_x<<16;
  base1=bsg_id_dn_y<<23;

  int base_address_dn=base0+base1+base2+base3;
  int base_address_dn_lxx=base0+base1+base2+base4;
  int base_address_dn_lyy=base0+base1+base2+base5;
  int base_address_dn_lxy=base0+base1+base2+base6;
  int base_address_dn_cim=base0+base1+base2+base7;


  volatile int *gm_gray_dn=reinterpret_cast<int*>(base_address_dn);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_dn= reinterpret_cast<int*>(base_address_dn_lxx);
  volatile int *gm_lyy_dn= reinterpret_cast<int*>(base_address_dn_lyy);
  volatile int *gm_lxy_dn= reinterpret_cast<int*>(base_address_dn_lxy);
  volatile int *gm_cim_dn= reinterpret_cast<int*>(base_address_dn_cim);


  int bsg_id_le;
  if(__bsg_id%bsg_tiles_X==0) {
    bsg_id_le=__bsg_id;
  } else {
    bsg_id_le=__bsg_id-1;
  }
  int bsg_id_le_x=bsg_id_le % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_le_y=bsg_id_le / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_le_x<<16;
  base1=bsg_id_le_y<<23;

  int base_address_le=base0+base1+base2+base3;
  int base_address_le_lxx=base0+base1+base2+base4;
  int base_address_le_lyy=base0+base1+base2+base5;
  int base_address_le_lxy=base0+base1+base2+base6;
  int base_address_le_cim=base0+base1+base2+base7;


  volatile int *gm_gray_le=reinterpret_cast<int*>(base_address_le);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_le= reinterpret_cast<int*>(base_address_le_lxx);
  volatile int *gm_lyy_le= reinterpret_cast<int*>(base_address_le_lyy);
  volatile int *gm_lxy_le= reinterpret_cast<int*>(base_address_le_lxy);
  volatile int *gm_cim_le= reinterpret_cast<int*>(base_address_le_cim);


  int bsg_id_rt;
  if(__bsg_id%bsg_tiles_X==(bsg_tiles_X-1)) {
    bsg_id_rt=__bsg_id;
  } else {
    bsg_id_rt=__bsg_id+1;
  }
  int bsg_id_rt_x=bsg_id_rt % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_rt_y=bsg_id_rt / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_rt_x<<16;
  base1=bsg_id_rt_y<<23;

  int base_address_rt=base0+base1+base2+base3;
  int base_address_rt_lxx=base0+base1+base2+base4;
  int base_address_rt_lyy=base0+base1+base2+base5;
  int base_address_rt_lxy=base0+base1+base2+base6;
  int base_address_rt_cim=base0+base1+base2+base7;


  volatile int *gm_gray_rt=reinterpret_cast<int*>(base_address_rt);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_rt= reinterpret_cast<int*>(base_address_rt_lxx);
  volatile int *gm_lyy_rt= reinterpret_cast<int*>(base_address_rt_lyy);
  volatile int *gm_lxy_rt= reinterpret_cast<int*>(base_address_rt_lxy);
  volatile int *gm_cim_rt= reinterpret_cast<int*>(base_address_rt_cim);

//top left
  int bsg_id_top_left;
  if((__bsg_id%bsg_tiles_X==0) || (__bsg_id<bsg_tiles_X)) {
    bsg_id_top_left=__bsg_id;
  } else {
    bsg_id_top_left=__bsg_id-9;
  }
  int bsg_id_top_left_x=bsg_id_top_left % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_top_left_y=bsg_id_top_left / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_top_left_x<<16;
  base1=bsg_id_top_left_y<<23;

  int base_address_top_left=base0+base1+base2+base3;
  int base_address_top_left_lxx=base0+base1+base2+base4;
  int base_address_top_left_lyy=base0+base1+base2+base5;
  int base_address_top_left_lxy=base0+base1+base2+base6;
  int base_address_top_left_cim=base0+base1+base2+base7;


  volatile int *gm_gray_top_left=reinterpret_cast<int*>(base_address_top_left);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_top_left= reinterpret_cast<int*>(base_address_top_left_lxx);
  volatile int *gm_lyy_top_left= reinterpret_cast<int*>(base_address_top_left_lyy);
  volatile int *gm_lxy_top_left= reinterpret_cast<int*>(base_address_top_left_lxy);
  volatile int *gm_cim_top_left= reinterpret_cast<int*>(base_address_top_left_cim);


//top right
  int bsg_id_top_right;
  if((__bsg_id%bsg_tiles_X==(bsg_tiles_X-1)) || (__bsg_id<bsg_tiles_X)) {
    bsg_id_top_right=__bsg_id;
  } else {
    bsg_id_top_right=__bsg_id-7;
  }
  int bsg_id_top_right_x=bsg_id_top_right % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_top_right_y=bsg_id_top_right / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_top_right_x<<16;
  base1=bsg_id_top_right_y<<23;

  int base_address_top_right=base0+base1+base2+base3;
  int base_address_top_right_lxx=base0+base1+base2+base4;
  int base_address_top_right_lyy=base0+base1+base2+base5;
  int base_address_top_right_lxy=base0+base1+base2+base6;
  int base_address_top_right_cim=base0+base1+base2+base7;


  volatile int *gm_gray_top_right=reinterpret_cast<int*>(base_address_top_right);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_top_right= reinterpret_cast<int*>(base_address_top_right_lxx);
  volatile int *gm_lyy_top_right= reinterpret_cast<int*>(base_address_top_right_lyy);
  volatile int *gm_lxy_top_right= reinterpret_cast<int*>(base_address_top_right_lxy);
  volatile int *gm_cim_top_right= reinterpret_cast<int*>(base_address_top_right_cim);

//bot right
  int bsg_id_bot_right;
  if((__bsg_id%bsg_tiles_X==(bsg_tiles_X-1)) || (__bsg_id/bsg_tiles_X==(bsg_tiles_Y-1))) {
    bsg_id_bot_right=__bsg_id;
  } else {
    bsg_id_bot_right=__bsg_id+9;
  }
  int bsg_id_bot_right_x=bsg_id_bot_right % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_bot_right_y=bsg_id_bot_right / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_bot_right_x<<16;
  base1=bsg_id_bot_right_y<<23;

  int base_address_bot_right=base0+base1+base2+base3;
  int base_address_bot_right_lxx=base0+base1+base2+base4;
  int base_address_bot_right_lyy=base0+base1+base2+base5;
  int base_address_bot_right_lxy=base0+base1+base2+base6;
  int base_address_bot_right_cim=base0+base1+base2+base7;


  volatile int *gm_gray_bot_right=reinterpret_cast<int*>(base_address_bot_right);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_bot_right= reinterpret_cast<int*>(base_address_bot_right_lxx);
  volatile int *gm_lyy_bot_right= reinterpret_cast<int*>(base_address_bot_right_lyy);
  volatile int *gm_lxy_bot_right= reinterpret_cast<int*>(base_address_bot_right_lxy);
  volatile int *gm_cim_bot_right= reinterpret_cast<int*>(base_address_bot_right_cim);

//bot left
  int bsg_id_bot_left;
  if((__bsg_id%bsg_tiles_X==0) ||(__bsg_id/bsg_tiles_X==(bsg_tiles_Y-1))) {
    bsg_id_bot_left=__bsg_id;
  } else {
    bsg_id_bot_left=__bsg_id+7;
  }
  int bsg_id_bot_left_x=bsg_id_bot_left % bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_bot_left_y=bsg_id_bot_left / bsg_tiles_X+bsg_tiles_Y;

  base0=bsg_id_bot_left_x<<16;
  base1=bsg_id_bot_left_y<<23;

  int base_address_bot_left=base0+base1+base2+base3;
  int base_address_bot_left_lxx=base0+base1+base2+base4;
  int base_address_bot_left_lyy=base0+base1+base2+base5;
  int base_address_bot_left_lxy=base0+base1+base2+base6;
  int base_address_bot_left_cim=base0+base1+base2+base7;


  volatile int *gm_gray_bot_left=reinterpret_cast<int*>(base_address_bot_left);
  //bsg_print_int(reinterpret_cast<int>(gm_gray_dn));
  volatile int *gm_lxx_bot_left= reinterpret_cast<int*>(base_address_bot_left_lxx);
  volatile int *gm_lyy_bot_left= reinterpret_cast<int*>(base_address_bot_left_lyy);
  volatile int *gm_lxy_bot_left= reinterpret_cast<int*>(base_address_bot_left_lxy);
  volatile int *gm_cim_bot_left= reinterpret_cast<int*>(base_address_bot_left_cim);


//begin
 
  //int init_index_inline=init_index_x+init_index_y*N_X;
  short int myA[12];
  //calculate gray scale image
  for (int y = 0; y< y_tile_len; y++) {
    for (int x = 0; x < x_tile_len; x+=4) {
      int global_index=(x+init_index_x)+(y+init_index_y)*N_X;
      int local_index=x+y*x_tile_len;
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

      gm_gray[local_index]=(int) ((77 *myA[0] + 150 * myA[1]+ 29 * myA[2]) >> 8);
      gm_gray[local_index+1]=(int) ((77 *myA[3] + 150 * myA[4]+ 29 * myA[5]) >> 8);
      gm_gray[local_index+2]=(int) ((77 *myA[6] + 150 * myA[7]+ 29 * myA[8]) >> 8);
      gm_gray[local_index+3]=(int) ((77 *myA[9] + 150 * myA[10]+ 29 * myA[11]) >> 8);
      //bsg_print_int(reinterpret_cast<int>(&gm_gray[x]));
      //bsg_print_int(reinterpret_cast<int>(&gm_gray[x+1]));
    }
  }
  //wait for global grey processing down
  bsg_barrier_hw_tile_group_sync();
  bsg_print_int(10000);

  int grad_x_local;
  int grad_y_local;

  for(int y=0;y<y_tile_len;y++){
    if(((y+init_index_y)==0) || ((y+init_index_y)==(N_Y-1))) continue;
    for(int x=0;x<x_tile_len;x++){
      if(((x+init_index_x)==0) || ((x+init_index_x)==(N_X-1))) continue;
      int gm_index=x+y*x_tile_len;

      int gray[9];
      get_surrounding_data(gray,gm_gray,x,y,x_tile_len,y_tile_len,gm_gray_top_left,gm_gray_up,gm_gray_top_right,gm_gray_le, gm_gray_rt,gm_gray_bot_left,gm_gray_dn,gm_gray_bot_right);

      grad_x_local=gray[2]-gray[0]-2*gray[3]+2*gray[5]-gray[6]+gray[8];
      if(grad_x_local<-180) grad_x_local=-180;
      else if(grad_x_local>180) grad_x_local=180;

      grad_y_local=gray[0]+2*gray[1]+gray[2]-gray[6]-2*gray[7]-gray[8];
      if(grad_y_local<-180) grad_y_local=-180;
      else if(grad_y_local>180) grad_y_local=180;

      gm_lxx[gm_index]=grad_x_local*grad_x_local>>6; //lxx
      gm_lyy[gm_index]=grad_y_local*grad_y_local>>6; //lxx
      gm_lxy[gm_index]=grad_x_local*grad_y_local>>6; //lxx
    }
  }

  bsg_barrier_hw_tile_group_sync();
  bsg_print_int(20000);
  //window size 3x3
  //calculate cim
  //short cim_local[(x_tile_len)*(y_tile_len)]={0};



  for(int y=0;y<y_tile_len;y++){
    if(((y+init_index_y)<2) || ((y+init_index_y)>N_Y-3)) continue;
    for(int x=0;x<x_tile_len;x++){
      if(((x+init_index_x)<2) || ((x+init_index_x)>N_X-3)) continue;
      int gm_index=x+y*x_tile_len;
      //bsg_print_int(gm_index);
      int lxx[9];
      int lyy[9];
      int lxy[9];

      int lgxx8;
      int lgyy8;
      int lgxy8;

      get_surrounding_data(lxx,gm_lxx,x,y,x_tile_len,y_tile_len,gm_lxx_top_left,gm_lxx_up,gm_lxx_top_right,gm_lxx_le, gm_lxx_rt,gm_lxx_bot_left,gm_lxx_dn,gm_lxx_bot_right);

      lgxx8 = lxx[0];
      lgxx8 +=lxx[1];
      lgxx8 +=lxx[2];
      lgxx8 +=lxx[3];
      lgxx8 +=lxx[4];
      lgxx8 +=lxx[5];
      lgxx8 +=lxx[6];
      lgxx8 +=lxx[7];
      lgxx8 +=lxx[8];
      lgxx8 =lgxx8>>6;

      get_surrounding_data(lyy,gm_lyy,x,y,x_tile_len,y_tile_len,gm_lyy_top_left,gm_lyy_up,gm_lyy_top_right,gm_lyy_le, gm_lyy_rt,gm_lyy_bot_left,gm_lyy_dn,gm_lyy_bot_right);

      lgyy8= lyy[0];
      lgyy8+=lyy[1];
      lgyy8+=lyy[2];
      lgyy8+=lyy[3];
      lgyy8+=lyy[4];
      lgyy8+=lyy[5];
      lgyy8+=lyy[6];
      lgyy8+=lyy[7];
      lgyy8+=lyy[8];
      lgyy8=lgyy8>>6;

      get_surrounding_data(lxy,gm_lxy,x,y,x_tile_len,y_tile_len,gm_lxy_top_left,gm_lxy_up,gm_lxy_top_right,gm_lxy_le, gm_lxy_rt,gm_lxy_bot_left,gm_lxy_dn,gm_lxy_bot_right);
      lgxy8= lxy[0];
      lgxy8+=lxy[1];
      lgxy8+=lxy[2];
      lgxy8+=lxy[3];
      lgxy8+=lxy[4];
      lgxy8+=lxy[5];
      lgxy8+=lxy[6];
      lgxy8+=lxy[7];
      lgxy8+=lxy[8];
      lgxy8=lgxy8>>6;

      short det=lgxx8*lgyy8 - lgxy8*lgxy8;
      gm_cim[gm_index] = det- ((lgxx8+lgyy8)*(lgxx8+lgyy8) >> 4);
    }
  }
 

  bsg_barrier_hw_tile_group_sync();
  bsg_print_int(30000);
  //bsg_fence();


  int cim[9];
  //check local max
  //short output_local[total_pixel_num]={0};
  for(int y=0;y<y_tile_len;y++){
    if(((y+init_index_y)<3) || ((y+init_index_y)>N_Y-4)) continue;
    //bsg_print_int(y);
    for(int x=0;x<x_tile_len;x++){
      if(((x+init_index_x)<3) || ((x+init_index_x)>N_X-4)) continue;
      int is_max;
      int x_cor=x+init_index_x;
      int y_cor=(y+init_index_y)*N_X;
      int global_index=x_cor+y_cor;
      //if(__bsg_id==22) bsg_print_int(global_index);
      int local_index=x+y_tile_len;
      //bsg_print_int(local_index);
      //bsg_print_int(40);

      //if(__bsg_id==22) bsg_print_int(8800+x);
      get_surrounding_data(cim,gm_cim,x,y,x_tile_len,y_tile_len,gm_cim_top_left,gm_cim_up,gm_cim_top_right,gm_cim_le, gm_cim_rt,gm_cim_bot_left,gm_cim_dn,gm_cim_bot_right);
      //if(__bsg_id==22) bsg_print_int(7700+x);
      int current_cim=cim[4];
      //if(__bsg_id==22) bsg_print_int(cim[0]);
      //if(__bsg_id==22) bsg_print_int(cim[1]);
      //if(__bsg_id==22) bsg_print_int(cim[2]);
      //if(__bsg_id==22) bsg_print_int(cim[3]);
      //if(__bsg_id==22) bsg_print_int(cim[4]);
      //if(__bsg_id==22) bsg_print_int(cim[5]);
      //if(__bsg_id==22) bsg_print_int(cim[6]);
      //if(__bsg_id==22) bsg_print_int(cim[7]);
      //if(__bsg_id==22) bsg_print_int(cim[8]);
      //if(__bsg_id==22) bsg_print_int(current_cim);
      if(current_cim>1){
        is_max = (current_cim >= cim[0]) && (current_cim >= cim[1]) &&(current_cim >= cim[2]) && (current_cim >= cim[3]) &&(current_cim >= cim[5]) && (current_cim >= cim[6]) && (current_cim >= cim[7]) && (current_cim >= cim[8]);
        //if(__bsg_id==22) bsg_print_int(current_cim);
        Output[global_index]=is_max ? 255:0;
      } else {
        //if(__bsg_id==22) bsg_print_int(44000000+x);
        Output[global_index]=0;
      }
      //if(__bsg_id==22) bsg_print_int(5500+x);
    }
  }

  bsg_print_int(40000);
  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}

