/** Very simple bitstream programmer for Lattice iCE40 FPGAs
 * using plain FTDI C232HM cable
 *
 * david.siorpaes@st.com
 *
 * Pinout
 * CLK        Orange
 * MOSI       Yellow
 * CS         White
 * RESET      Blue
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mpsse.h>


int main(int argc, char** argv)
{
	int filefd;
	struct stat in_stat;
	void* fileaddr;	
	struct mpsse_context* context = NULL;
	int err;

	uint32_t id = 0x7eaa997e;
	uint8_t trailer[4];
	
	if(argc < 2){
		fprintf(stderr, "Usage: %s <file> [file] ...\n", argv[0]);
		exit(1);
	}


	/* Open and mmap file */
	if((filefd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
		return errno;
	}

	fstat(filefd, &in_stat);
	printf("File length: %u\n", (int)in_stat.st_size);

	fileaddr = mmap(NULL, in_stat.st_size,
		PROT_READ, MAP_SHARED, filefd, 0);
	if(fileaddr == MAP_FAILED){
		fprintf(stderr, "Failed to mmap %s: %s\n", argv[1], strerror(errno));
		return errno;
	}


	/* Open FTDI device for SPI */
	context = Open(0x0403, 0x6014, SPI0, SIX_MHZ, MSB, IFACE_A, NULL, NULL);
	if(context == NULL || context->open == 0){
		fprintf(stderr, "MPSSE context not valid!\n");
		exit(1);
	}
	else
		printf("Context ok\n");

	/* Set iCE40 to SPI slave */
	PinLow(context, GPIOL2); //spi slave, White wire
	PinLow(context, GPIOL3); //reset, Blue wire
	usleep(200000);
	
	/* Leave reset */
	PinHigh(context, GPIOL3);
	usleep(100000);

	
	/* Start SPI communication */
	err = Start(context);
	if(err != MPSSE_OK){
		fprintf(stderr, "Error on Start condition: %i\n", err);
		return -1;
	}

	/* First, send Synchronization Pattern. Cfr TN1248 */
	err = FastWrite(context, (char*)&id, sizeof(id));
	if(err != MPSSE_OK){
		fprintf(stderr, "Error writing SPI: %i\n", err);
		return -1;
	}

	/* Send bitstream */
	err = FastWrite(context, fileaddr, in_stat.st_size);
	if(err != MPSSE_OK){
		fprintf(stderr, "Error writing SPI: %i\n", err);
		return -1;
	}

	/* Send dummy bits */
	err = FastWrite(context, (char*)trailer, sizeof(trailer));
	if(err != MPSSE_OK){
		fprintf(stderr, "Error writing SPI: %i\n", err);
		return -1;
	}
	
	/* Clean up */
	Stop(context);
	Close(context);
	munmap(fileaddr, in_stat.st_size);
	
	return 0;
}
