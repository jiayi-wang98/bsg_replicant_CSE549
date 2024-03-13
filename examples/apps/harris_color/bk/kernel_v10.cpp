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
  int x_tile_len = N_X ; //64
  int y_tile_len = N_Y / bsg_tiles_Y /bsg_tiles_X; //64/32=2

  int Cinit;//image tile initial

  //volatile int* gm = (int *)((__bsg_id%bsg_tiles_X)<<16 + (__bsg_id/bsg_tiles_X)<<23 + 1<<30) ;
  int bsg_id_x=__bsg_id% bsg_tiles_X+bsg_tiles_X; //pod(1,1)
  int bsg_id_y=__bsg_id/ bsg_tiles_X+bsg_tiles_Y;

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

  //the coretile (x,y) processing the image tile above this core

  int bsg_id_up;
  if(__bsg_id==0) {
    bsg_id_up=0;
  } else {
    if(__bsg_id==8){
      bsg_id_up=23;
    } else if((__bsg_id<4) || ((__bsg_id>8) && (__bsg_id<12))) {
      bsg_id_up=__bsg_id+19;
    } else if((__bsg_id%8>3)) {
      bsg_id_up=__bsg_id-4;
    } else {
      bsg_id_up=__bsg_id-12;
    }
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
  if(__bsg_id==(bsg_tiles_X*bsg_tiles_Y-1)) {
    bsg_id_dn=bsg_tiles_X*bsg_tiles_Y-1;
  } else {
    if(__bsg_id==23){
      bsg_id_dn=8;
    } else if((__bsg_id>27) || ((__bsg_id>19) && (__bsg_id<23))) {
      bsg_id_dn=__bsg_id-19;
    } else if((__bsg_id%8<4)) {
      bsg_id_dn=__bsg_id+4;
    } else {
      bsg_id_dn=__bsg_id+12;
    }
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


//begin
 
  int init_index_x=0;
  int init_index_y=(Cinit/2);
  int init_index=init_index_x+init_index_y*N_X;
  //int init_index_inline=init_index_x+init_index_y*N_X;
  short int myA[12];
  //calculate gray scale image
  for (int x = 0; x < x_tile_len*y_tile_len; x+=4) {
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

      gm_gray[x]=(int) ((77 *myA[0] + 150 * myA[1]+ 29 * myA[2]) >> 8);
      gm_gray[x+1]=(int) ((77 *myA[3] + 150 * myA[4]+ 29 * myA[5]) >> 8);
      gm_gray[x+2]=(int) ((77 *myA[6] + 150 * myA[7]+ 29 * myA[8]) >> 8);
      gm_gray[x+3]=(int) ((77 *myA[9] + 150 * myA[10]+ 29 * myA[11]) >> 8);
      //bsg_print_int(reinterpret_cast<int>(&gm_gray[x]));
      //bsg_print_int(reinterpret_cast<int>(&gm_gray[x+1]));
  }
  //wait for global grey processing down
  bsg_fence();
  bsg_print_int(1);
  int top_left;
  int top;
  int top_right;
  int left;
  int right;
  int bot_left;
  int bot;
  int bot_right;
  int grad_x_local;
  int grad_y_local;

  for(int y=0;y<y_tile_len;y++){
    for(int x=1;x<x_tile_len-1;x++){

      if(((y+init_index_y)==0) || ((y+init_index_y)==N_Y-1)) continue;

      int gm_index=x+y*N_X;

      int gm_top_left_index=(x-1)+(y-1)*N_X;
      int gm_top_index=(x)+(y-1)*N_X;
      int gm_top_right_index=(x+1)+(y-1)*N_X;
      int gm_left_index=(x-1)+(y)*N_X;
      //int middle_index=(x)+(y+1)*N_X;
      int gm_right_index=(x+1)+(y)*N_X;
      int gm_bot_left_index=(x-1)+(y+1)*N_X;
      int gm_bot_index=x+(y+1)*N_X;
      int gm_bot_right_index=(x+1)+(y+1)*N_X;

      //need 1 line from upper core
      if(y==0){
        gm_top_left_index=(x-1)+(y_tile_len-1)*N_X;
        gm_top_index=(x)+(y_tile_len-1)*N_X;
        gm_top_right_index=(x+1)+(y_tile_len-1)*N_X;
        //bsg_print_int(reinterpret_cast<int>(gm_gray_up+gm_top_left_index));
        //bsg_print_int(reinterpret_cast<int>(gm_gray_up+gm_top_index));
        //bsg_print_int(reinterpret_cast<int>(gm_gray_up+gm_top_right_index));
        top_left=*(gm_gray_up+gm_top_left_index);
        top=*(gm_gray_up+gm_top_index);
        top_right=*(gm_gray_up+gm_top_right_index);
        bot_left=gm_gray[gm_bot_left_index];
        bot=gm_gray[gm_bot_index];
        bot_right=gm_gray[gm_bot_right_index];
      }//need 1 line from down core 
      else if(y==1){
        gm_bot_left_index=(x-1);
        gm_bot_index=(x);
        gm_bot_right_index=(x+1);
        top_left=gm_gray[gm_top_left_index];
        top=gm_gray[gm_top_index];
        top_right=gm_gray[gm_top_right_index];
        //bsg_print_int(reinterpret_cast<int>(gm_gray_dn+gm_bot_left_index));
        //bsg_print_int(reinterpret_cast<int>(gm_gray_dn+gm_bot_index));
        //bsg_print_int(reinterpret_cast<int>(gm_gray_dn+gm_bot_right_index));
        bot_left=*(gm_gray_dn+gm_bot_left_index);
        bot=*(gm_gray_dn+gm_bot_index);
        bot_right=*(gm_gray_dn+gm_bot_right_index);
      } else 
      {
        top_left=gm_gray[gm_top_left_index];
        top=gm_gray[gm_top_index];
        top_right=gm_gray[gm_top_right_index];

        bot_left=gm_gray[gm_bot_left_index];
        bot=gm_gray[gm_bot_index];
        bot_right=gm_gray[gm_bot_right_index];
      }

      left=gm_gray[gm_left_index];
      right=gm_gray[gm_right_index];


      grad_x_local=top_right-top_left-2*left+2*right-bot_left+bot_right;
      if(grad_x_local<-180) grad_x_local=-180;
      else if(grad_x_local>180) grad_x_local=180;

      grad_y_local=top_left+2*top+top_right-bot_left-2*bot-bot_right;
      if(grad_y_local<-180) grad_y_local=-180;
      else if(grad_y_local>180) grad_y_local=180;

      gm_lxx[gm_index]=grad_x_local*grad_x_local>>6; //lxx
      gm_lyy[gm_index]=grad_y_local*grad_y_local>>6; //lxx
      gm_lxy[gm_index]=grad_x_local*grad_y_local>>6; //lxx
    }
  }

  bsg_fence();
  bsg_print_int(2);
  //window size 3x3
  //calculate cim
     
  //short cim_local[(x_tile_len)*(y_tile_len)]={0};
  short lgxx8;
  short lgyy8;
  short lgxy8;

  short int lxxyy[9];

  for(int y=0;y<y_tile_len;y++){
    for(int x=2;x<x_tile_len-2;x++){
      if(((y+init_index_y)<2) || ((y+init_index_y)>N_Y-3)) continue;

      int gm_index=x+y*N_X;

      int gm_top_left_index=(x-1)+(y-1)*N_X;
      int gm_top_index=(x)+(y-1)*N_X;
      int gm_top_right_index=(x+1)+(y-1)*N_X;
      int gm_left_index=(x-1)+(y)*N_X;
      int gm_middle_index=(x)+(y)*N_X;
      int gm_right_index=(x+1)+(y)*N_X;
      int gm_bot_left_index=(x-1)+(y+1)*N_X;
      int gm_bot_index=x+(y+1)*N_X;
      int gm_bot_right_index=(x+1)+(y+1)*N_X;


      //need 1 line from upper core
      if(y==0){
        gm_top_left_index=(x-1)+(y_tile_len-1)*N_X;
        gm_top_index=(x)+(y_tile_len-1)*N_X;
        gm_top_right_index=(x+1)+(y_tile_len-1)*N_X;

        lxxyy[0]=*(gm_lxx_up+gm_top_left_index);
        lxxyy[1]=*(gm_lxx_up+gm_top_index);
        lxxyy[2]=*(gm_lxx_up+gm_top_right_index);
        lxxyy[3]=gm_lxx[gm_left_index];
        lxxyy[4]=gm_lxx[gm_middle_index];
        lxxyy[5]=gm_lxx[gm_right_index];
        lxxyy[6]=gm_lxx[gm_bot_left_index];
        lxxyy[7]=gm_lxx[gm_bot_index];
        lxxyy[8]=gm_lxx[gm_bot_right_index];

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

        lxxyy[0]=*(gm_lyy_up+gm_top_left_index);
        lxxyy[1]=*(gm_lyy_up+gm_top_index);
        lxxyy[2]=*(gm_lyy_up+gm_top_right_index);
        lxxyy[3]=gm_lyy[gm_left_index];
        lxxyy[4]=gm_lyy[gm_middle_index];
        lxxyy[5]=gm_lyy[gm_right_index];
        lxxyy[6]=gm_lyy[gm_bot_left_index];
        lxxyy[7]=gm_lyy[gm_bot_index];
        lxxyy[8]=gm_lyy[gm_bot_right_index];

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


        lxxyy[0]=*(gm_lxy_up+gm_top_left_index);
        lxxyy[1]=*(gm_lxy_up+gm_top_index);
        lxxyy[2]=*(gm_lxy_up+gm_top_right_index);
        lxxyy[3]=gm_lxy[gm_left_index];
        lxxyy[4]=gm_lxy[gm_middle_index];
        lxxyy[5]=gm_lxy[gm_right_index];
        lxxyy[6]=gm_lxy[gm_bot_left_index];
        lxxyy[7]=gm_lxy[gm_bot_index];
        lxxyy[8]=gm_lxy[gm_bot_right_index];

        
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


      }//need 1 line from down core 
      else if(y==y_tile_len-1){
        gm_bot_left_index=(x-1);
        gm_bot_index=(x);
        gm_bot_right_index=(x+1);

        lxxyy[0]=gm_lxx[gm_top_left_index];
        lxxyy[1]=gm_lxx[gm_top_index];
        lxxyy[2]=gm_lxx[gm_top_right_index];
        lxxyy[3]=gm_lxx[gm_left_index];
        lxxyy[4]=gm_lxx[gm_middle_index];
        lxxyy[5]=gm_lxx[gm_right_index];
        lxxyy[6]=*(gm_lxx_dn+gm_bot_left_index);
        lxxyy[7]=*(gm_lxx_dn+gm_bot_index);
        lxxyy[8]=*(gm_lxx_dn+gm_bot_right_index);

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

        lxxyy[0]=gm_lyy[gm_top_left_index];
        lxxyy[1]=gm_lyy[gm_top_index];
        lxxyy[2]=gm_lyy[gm_top_right_index];
        lxxyy[3]=gm_lyy[gm_left_index];
        lxxyy[4]=gm_lyy[gm_middle_index];
        lxxyy[5]=gm_lyy[gm_right_index];
        lxxyy[6]=*(gm_lyy_dn+gm_bot_left_index);
        lxxyy[7]=*(gm_lyy_dn+gm_bot_index);
        lxxyy[8]=*(gm_lyy_dn+gm_bot_right_index);

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

        lxxyy[0]=gm_lxy[gm_top_left_index];
        lxxyy[1]=gm_lxy[gm_top_index];
        lxxyy[2]=gm_lxy[gm_top_right_index];
        lxxyy[3]=gm_lxy[gm_left_index];
        lxxyy[4]=gm_lxy[gm_middle_index];
        lxxyy[5]=gm_lxy[gm_right_index];
        lxxyy[6]=*(gm_lxy_dn+gm_bot_left_index);
        lxxyy[7]=*(gm_lxy_dn+gm_bot_index);
        lxxyy[8]=*(gm_lxy_dn+gm_bot_right_index);

        
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

      } else {
        lxxyy[0]=gm_lxx[gm_top_left_index];
        lxxyy[1]=gm_lxx[gm_top_index];
        lxxyy[2]=gm_lxx[gm_top_right_index];
        lxxyy[3]=gm_lxx[gm_left_index];
        lxxyy[4]=gm_lxx[gm_middle_index];
        lxxyy[5]=gm_lxx[gm_right_index];
        lxxyy[6]=gm_lxx[gm_bot_left_index];
        lxxyy[7]=gm_lxx[gm_bot_index];
        lxxyy[8]=gm_lxx[gm_bot_right_index];

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

        lxxyy[0]=gm_lyy[gm_top_left_index];
        lxxyy[1]=gm_lyy[gm_top_index];
        lxxyy[2]=gm_lyy[gm_top_right_index];
        lxxyy[3]=gm_lyy[gm_left_index];
        lxxyy[4]=gm_lyy[gm_middle_index];
        lxxyy[5]=gm_lyy[gm_right_index];
        lxxyy[6]=gm_lyy[gm_bot_left_index];
        lxxyy[7]=gm_lyy[gm_bot_index];
        lxxyy[8]=gm_lyy[gm_bot_right_index];

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

        lxxyy[0]=gm_lxy[gm_top_left_index];
        lxxyy[1]=gm_lxy[gm_top_index];
        lxxyy[2]=gm_lxy[gm_top_right_index];
        lxxyy[3]=gm_lxy[gm_left_index];
        lxxyy[4]=gm_lxy[gm_middle_index];
        lxxyy[5]=gm_lxy[gm_right_index];
        lxxyy[6]=gm_lxy[gm_bot_left_index];
        lxxyy[7]=gm_lxy[gm_bot_index];
        lxxyy[8]=gm_lxy[gm_bot_right_index];

        
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

      }
      short int det=lgxx8*lgyy8 - lgxy8*lgxy8;
      gm_cim[gm_index] = (short) det- ((lgxx8+lgyy8)*(lgxx8+lgyy8) >> 4);
    }
  }
 
  bsg_fence();
  bsg_print_int(3);

  short int cim[9];
  //check local max
  //short output_local[total_pixel_num]={0};
  for(int y=0;y<y_tile_len;y++){
    for(int x=3;x<x_tile_len-3;x++){
      if(((y+init_index_y)<3) || ((y+init_index_y)>N_Y-4)) continue;
      int is_max;

      int global_index=x+(y+init_index_y)*N_X;
      int gm_index=x+y*N_X;

      int gm_top_left_index=(x-1)+(y-1)*N_X;
      int gm_top_index=(x)+(y-1)*N_X;
      int gm_top_right_index=(x+1)+(y-1)*N_X;
      int gm_left_index=(x-1)+(y)*N_X;
      int gm_middle_index=(x)+(y)*N_X;
      int gm_right_index=(x+1)+(y)*N_X;
      int gm_bot_left_index=(x-1)+(y+1)*N_X;
      int gm_bot_index=x+(y+1)*N_X;
      int gm_bot_right_index=(x+1)+(y+1)*N_X;

      int current_cim=gm_cim[gm_index];

      if(y==0){
        gm_top_left_index=(x-1)+(y_tile_len-1)*N_X;
        gm_top_index=(x)+(y_tile_len-1)*N_X;
        gm_top_right_index=(x+1)+(y_tile_len-1)*N_X;

        top_left=*(gm_cim_up+gm_top_left_index);
        top=*(gm_cim_up+gm_top_index);
        top_right=*(gm_cim_up+gm_top_right_index);

        bot_left=gm_cim[gm_bot_left_index];
        bot=gm_cim[gm_bot_index];
        bot_right=gm_cim[gm_bot_right_index];
      }//need 1 line from down core 
      else if(y==y_tile_len-1){
        gm_bot_left_index=(x-1);
        gm_bot_index=(x);
        gm_bot_right_index=(x+1);

        top_left=gm_cim[gm_top_left_index];
        top=gm_cim[gm_top_index];
        top_right=gm_cim[gm_top_right_index];

        bot_left=*(gm_cim_dn+gm_bot_left_index);
        bot=*(gm_cim_dn+gm_bot_index);
        bot_right=*(gm_cim_dn+gm_bot_right_index);

      } else {
        top_left=gm_cim[gm_top_left_index];
        top=gm_cim[gm_top_index];
        top_right=gm_cim[gm_top_right_index];

        bot_left=gm_cim[gm_bot_left_index];
        bot=gm_cim[gm_bot_index];
        bot_right=gm_cim[gm_bot_right_index];
      }

      left=gm_cim[gm_left_index];
      right=gm_cim[gm_right_index];

      if(current_cim>1){
        is_max=current_cim >= top_left && current_cim >= top &&
          current_cim >= left && current_cim >= right &&
          current_cim >= bot_left && current_cim >= bot &&
          current_cim >= bot_right && current_cim >= top_right;
        Output[global_index]=is_max ? 255:0;
      } else Output[global_index]=0;
    }
  }
  bsg_print_int(4);

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}

