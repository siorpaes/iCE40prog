/** Very simple bitstream programmer for Lattice iCE40 FPGAs
 * using plain FTDI C232HM cable in bitbang mode so to support
 * Go Board. Uses libftdi directly, not libmpsse
 *
 * See http://jdelfes.blogspot.it/2014/02/spi-bitbang-ft232r.html
 *
 * david.siorpaes@st.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ftdi.h>


#define PIN_ENA  0x04
#define PIN_SCK  0x08
#define PIN_MISO 0x10
#define PIN_SS   0x20
#define PIN_MOSI 0x40

unsigned char gpio_out = PIN_ENA | PIN_SCK | PIN_SS | PIN_MOSI;

void digitalWrite(struct ftdi_context *ftdi, unsigned char pin, int value)
{
	// store current value
	static unsigned char r = 0;
	
	if (value)
		r |= pin;
	else
		r &= ~pin;
	
	ftdi_write_data(ftdi, &r, sizeof(r));
}


int main(int argc, char** argv)
{
	struct ftdi_context *ftdi;
	struct ftdi_version_info version;
	int filefd;
	struct stat in_stat;
	void* fileaddr;	
	int err, r;

	uint32_t sync = 0x7eaa997e;
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

	fileaddr = mmap(NULL, in_stat.st_size, PROT_READ, MAP_SHARED, filefd, 0);
	if(fileaddr == MAP_FAILED){
		fprintf(stderr, "Failed to mmap %s: %s\n", argv[1], strerror(errno));
		return errno;
	}
	ftdi = ftdi_new();
	if (ftdi == 0) {
		fprintf(stderr, "ftdi_new failed\n");
		return EXIT_FAILURE;
	}
	
	// show library version
	version = ftdi_get_library_version();
	printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d,"
		" snapshot ver: %s)\n", version.version_str, version.major,
		version.minor, version.micro, version.snapshot_str);
	
	// try to open usb
	r = ftdi_usb_open(ftdi, 0x0403, 0x6014);
	if (r != 0) {
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", r, ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		return EXIT_FAILURE;
	}
	
	// Read out FTDIChip-ID of R type chips
	if (1 || ftdi->type == TYPE_R) {
		unsigned int chipid = 0;
		
		printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
		printf("FTDI chipid: %X\n", chipid);
	}

	ftdi_set_bitmode(ftdi, gpio_out, BITMODE_SYNCBB);

	
	// free resources
	r = ftdi_usb_close(ftdi);
	if (r != 0) {
		fprintf(stderr, "unable to close ftdi device: %d (%s)\n", r,
			ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		return EXIT_FAILURE;
	}
	
	ftdi_free(ftdi);
	return EXIT_SUCCESS;
	
#if 0
	/* Open FTDI device for SPI */
	context = Open(0x0403, 0x6014, SPI0, SIX_MHZ, MSB, IFACE_A, NULL, NULL);
	if(context == NULL || context->open == 0){
		fprintf(stderr, "MPSSE context not valid!\nMake sure winUSB drivers are installed with Zadig\n");
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
	err = FastWrite(context, (char*)&sync, sizeof(sync));
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

#endif
	munmap(fileaddr, in_stat.st_size);
	
	return 0;
}
