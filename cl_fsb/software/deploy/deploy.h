#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

// our software stack
#include "../host.h"
#include "../device.h"
#include "driver/bsg_dma_driver.h"
#include "fifo.h"

// define DEBUG

static const uint32_t BUF_SIZE = 4 * 1024 * 1024;
static const uint32_t ALIGNMENT = 64;
static const uint32_t POP_SIZE = 64;

static char *dev_path = "/dev/bsg_dma_driver";
int dev_fd; 
char *ocl_base = 0;

static uint32_t get_tail (struct Host *host) {
	int tail = 0;
	if (ioctl(dev_fd, IOCTL_TAIL, &tail) != 0 ) // error
			printf("ioctl error. returning tail as 0.\n");	

	return tail; 
}

static void write_wr_head (struct Host *host, uint32_t val) {
	if (ocl_base) {
		uint32_t *reg = (uint32_t *) (ocl_base + WR_HEAD);
		*reg = val;
	}
	else {
		if (ioctl(dev_fd, IOCTL_WR_HEAD, val) != 0) 
			printf("ioctl error.\n"); 
	}
}

static void write_wr_len (struct Host *host, uint32_t val) {
	if (ocl_base) {
		uint32_t *reg = (uint32_t *) (ocl_base + WR_LEN);
		*reg = val;
	}
	else {
		if (ioctl(dev_fd, IOCTL_WR_LEN, val) != 0) 
			printf("ioctl error.\n"); 
	}
}

static void write_wr_buf_size (struct Host *host, uint32_t val) {
	if (ioctl(dev_fd, IOCTL_WR_BUF_SIZE, val) != 0) 
		printf("ioctl error.\n"); 
}

static void start_write (struct Host *host) {
	if (ocl_base) {
		uint32_t *reg = (uint32_t *) (ocl_base + CFG_REG);
		*reg = 0x10;
		reg = (uint32_t *) (ocl_base + CNTL_REG);
		*reg = 0x1;
	}
	else {
		if (ioctl(dev_fd, IOCTL_CFG, 0x10) != 0) 
			printf("ioctl error when writing to CFG_REG.\n"); 
		if (ioctl(dev_fd, IOCTL_CNTL, 0x1) != 0) 
			printf("ioctl error when writing to CNTL_REG.\n"); 
	}
}

static void stop (struct Host *host) {
	if (ioctl(dev_fd, IOCTL_CFG, STOP) != 0) 
		printf("ioctl error.\n"); 
}

/*
 * TODO: return read's return value. 
 * */
static bool pop (struct Host *host, uint32_t pop_size) {

	uint32_t tail;
	ioctl(dev_fd, IOCTL_TAIL, &tail);
	
	uint32_t head = host->head; 
	
	bool can_read;
	uint32_t unused, num_cpy;
	int result;
	
	if (tail >= head)
		unused = tail - head;
	else
		unused = tail - head + DMA_BUFFER_SIZE;

	uint32_t free_space;
	free_space = DMA_BUFFER_SIZE - unused;
	
	if (free_space <= 5120) {
		printf("WARNNING!!!! : CL does not have enough place to write. %d\n", free_space);
	}
	
	can_read = unused >= pop_size;

	if (!can_read) {
//		printf("host: can't read %u bytes because (Head, Tail) = (%u, %u);\n only %u bytes available.\n", pop_size, head, tail, unused); 
//		printf("..(%u, %u)\n", head, tail);
		return false;
	}
	
	/* there is enough unread data; first, read data that lies before the end of system memory buffer */
	num_cpy = (DMA_BUFFER_SIZE - head >= pop_size) ? pop_size : DMA_BUFFER_SIZE - head; 
	result = read(dev_fd, host->buf_cpy + head, num_cpy);
	if (result != num_cpy)	{
		printf("host: could not copy %u bytes.\n", pop_size);
		return false;
	}
	head = (head + num_cpy) % DMA_BUFFER_SIZE;
	num_cpy = pop_size - num_cpy;  /* data that wraps over the end of system memory buffer */
	if (num_cpy > 0) { /* if there is still data to read */
		result = read(dev_fd, host->buf_cpy, num_cpy);
		if (result != num_cpy)	{
			printf("host: could not copy %u bytes.\n", pop_size);
			return false;
		}
		head = head + num_cpy;	
	}

	ioctl(dev_fd, IOCTL_WR_HEAD, head); /* update head register on device */
	host->head = head; /* update its own copy of head */	
	return true;
} 


static uint32_t get_pkt_num (struct Host *host) {
	uint32_t pkt_num = 0;
	if (ioctl(dev_fd, IOCTL_PKT_NUM, &pkt_num) != 0)
		printf("ioctl error. return pkt_num as 0.\n");
	return pkt_num;
}


/* 
 * prints data as a sequence of unsigned chars.
 * */
void print(struct Host *host, uint32_t ofs, uint32_t size) {
	if (!host->buf_cpy)
		printf("Host::print: buf_cpy is null. can't print.\n");
	else if (!size)
		printf("Host::print: size is 0. can't print.\n");
	else if (ofs + size > host->buf_size)
		printf("Host::print: invalid range to print. can't print. \n");

	for (int i = ofs; i < size; i++) {
		printf("0x%02X", host->buf_cpy[i]);
		if ((i + 1) % 10 == 0)
			printf("\n");
		else
			printf(" ");
	}
	printf("\n");
}

void deploy_init_host (struct Host *host, uint32_t buf_size, uint32_t align) {
	dev_fd = open(dev_path, O_RDWR);
	if (dev_fd == -1) {
		printf("Unable to open device.\n");
		return; 
	}
	if (buf_size > 0) {
		host->buf_size = DMA_BUFFER_SIZE; /* global buffer */
		host->buf = NULL; /* TODO: rename to mmap_buf */
		ioctl(dev_fd, IOCTL_CLEAR_BUFFER);
		host->buf_cpy = (char *) aligned_alloc(align, buf_size + 64); /* user copy of buffer */
		memset(host->buf_cpy, 0, buf_size + 64);
		
		host->head = 0;
		
		host->get_tail = get_tail;	

		host->write_wr_head = write_wr_head;
		host->write_wr_len = write_wr_len;
		host->write_wr_buf_size = write_wr_buf_size;
		host->start_write = start_write;
		host->stop = stop;
		host->pop = pop;
		host->print = print;
	}
}

char *deploy_mmap_ocl () {
	if (dev_fd == -1)
		return 0;
	char *addr = mmap(NULL, 0x2000, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, 0); 
	if (addr == MAP_FAILED) {
		printf("mmap failed.\n");
		return 0;
	}
	ocl_base = addr;
	return addr;
} 

void deploy_close () {
	if (ocl_base)
		munmap(ocl_base, 0x1000);
	if (dev_fd != -1)
		close(dev_fd);
}

/*
 * writes 128B to the nth fifo
 * returns true on success and false on failure.
 * */
bool deploy_write_fifo (uint8_t n, int *val) {
	
	if (n >= NUM_FIFO) {
		printf("invalid fifo.\n");
		return false;
	}

	uint16_t *reg = (uint16_t *) (ocl_base + fifo[n][FIFO_VACANCY]);
	#ifdef DEBUG
	printf("write(): vacancy is %u\n", *reg);	
	#endif

	if (*reg < 4) {
		printf("not enough space in fifo.\n");
		return false;
	}
	
	for (int i = 0; i < 4; i++) {
		//printf("writing %d to reg %d\n", val[i], fifo[n][FIFO_WRITE]);
		*((int *) (ocl_base + fifo[n][FIFO_WRITE])) = val[i];
	}
	while (*reg != 508) {
		*((uint16_t *) (ocl_base + fifo[n][FIFO_TRANSMIT_LENGTH])) = (uint16_t) (16);
	//	printf("write(): vacancy not yet updated.\n");
	}

	return true;
}

/*
 * reads 128B from the nth fifo
 * returns dequeued element on success and INT_MAX on failure.
 * */
int *deploy_read_fifo (uint8_t n) {
	if (n >= NUM_FIFO) {
		printf("Invalid fifo.\n.");
		return NULL;
	}

	uint16_t *reg = (uint16_t *) (ocl_base + fifo[n][FIFO_OCCUPANCY]);
;
	while (*reg < 1) {
		printf("Occupancy still 0.\n");
	}
//	#ifdef DEBUG
//	printf("read(): occupancy is %u\n", *reg);	
//	#endif
//	if (*reg < 1) {
//		printf("not enough data in fifo.\n");
//		return NULL;
//	}

	reg = ((uint16_t *) (ocl_base + fifo[n][FIFO_RECEIVE_LENGTH]));
	#ifdef DEBUG
	printf("read(): read the receive length register @ %u to be %u\n", fifo[n][FIFO_RECEIVE_LENGTH], *reg);
	#endif
	int *val = (int *) calloc(4, sizeof(int));
	for (int i = 0; i < 4; i++) {
		val[i] = *((int *) (ocl_base + fifo[n][FIFO_READ]));
	}

	/* dummy reads */
	return val;

}

/* clears interrupts for the nth fifo */
void clear_int (uint8_t n) {
	if (n >= NUM_FIFO) { 
		printf("Invalid fifo.\n");
		return;
	}
	*((int *) (ocl_base + fifo[n][FIFO_ISR])) = 0xFFFFFFFF;
}
