#include <sys/types.h>
#include <STDIO.H>
#include <LIBSIO.H>
#include <LIBETC.H>
#include "comm.h"
#include "siofs.h"

static void initTable16(unsigned short* table) {

	int i, j;
    unsigned short crc, c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = (unsigned short) i;

        for (j=0; j<8; j++) {

            if ( (crc ^ c) & 0x0001 )
				crc = ( crc >> 1 ) ^ 0xA001;
            else
				crc =   crc >> 1;

            c = c >> 1;
        }

        table[i] = crc;
    }

}

static unsigned short crc16(void* buff, int bytes, unsigned short crc) {

	int i;
	unsigned short tmp, short_c;
	unsigned short crcTable[256];

	initTable16(crcTable);

	for(i=0; i<bytes; i++) {

		short_c = 0x00ff & (unsigned short)((unsigned char*)buff)[i];

		tmp =  crc       ^ short_c;
		crc = (crc >> 8) ^ crcTable[tmp&0xff];

	}

    return(crc);

}

typedef struct {
	unsigned short flags;
	unsigned short length;
} SFS_OPENSTRUCT;

typedef struct {
	unsigned short fd;
	unsigned short crc16; 
	unsigned int length;
} SFS_WRITESTRUCT;

typedef struct {
	unsigned short fd;
	unsigned short pad;
	int length;
} SFS_READSTRUCT;

typedef struct {
	unsigned short ret;
	unsigned short crc16;
	unsigned int length;
} SFS_READREPLY;

typedef struct {
	unsigned short fd;
	unsigned short mode;
	unsigned int offset;
} SFS_SEEKSTRUCT;

typedef struct {
	short num;
	short offset;
} SFS_DIRPARAM;

typedef struct {
	short response;
	u_short crc;
	int length;
} SFS_QUICKREAD;

static int _eof_hit;
static int _list_total = 0;

int fsInit(int baud) {
	
	int ret;
	
	// Initialize serial
	commInit();
	commSetRate(baud);
	
	// Set init command
	commWriteBytes((u_char*)"~FRS", 4);
	
	ret = 0;
	
	// Wait for acknowledge
	if ( commReadBytes((u_char*)&ret, 2) != 2 ) {
		return 0;
	}
	
	return ret;
	
}

int fsOpen(const char* filename, int flags) {
	
	int ret;
	short handle;
	SFS_OPENSTRUCT op;
	
	// Send command
	commWriteBytes((u_char*)"~FOP", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file flags and file name
	op.flags = flags;
	op.length = strlen(filename);
	commWriteBytes((u_char*)&op, 4);
	commWriteBytes((u_char*)filename, op.length);
	
	// Get handle number
	handle = 0;
	if ( commReadBytes((u_char*)&handle, 1) != 1 ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Sign extension fix for signed bytes
	if ( handle > 127 ) {
		handle |= 0xff80;
	}
	
	switch(handle) {
		case -1:
			return FS_ERR_NOTFOUND;
		case -2:
			return FS_ERR_NOHANDLES;
	}
	
	_eof_hit = 0;
	
	return handle;
	
}

int fsClose(int fd) {
	
	int ret;
	
	// Send command
	commWriteBytes((u_char*)"~FCL", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file descriptor
	commWriteBytes((u_char*)&fd, 1);
	
	// Wait for acknowledge
	ret = 0;
	if ( commReadBytes((u_char*)&ret, 1) != 1 ) {
		return FS_ERR_TIMEOUT;
	}
	
	switch(ret) {
		case 1:
			return FS_ERR_NOTOPEN;
		case 2:
			return FS_ERR_INVALID;
	}
	
	return 0;
	
}

int fsWrite(int fd, u_char* data, int size) {
	
	SFS_WRITESTRUCT info;
	int ret;

	info.crc16 = crc16(data, size, 0);
	
	// Send command
	commWriteBytes((u_char*)"~FWR", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file descriptor and size of data to write
	info.fd = fd;
	info.length = size;
	
	commWriteBytes((u_char*)&info, sizeof(SFS_WRITESTRUCT));
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	
	if ( ret == 1 ) {
		return FS_ERR_NOTOPEN;
	} else if ( ret == 2 ) {
		return FS_ERR_INVALID;
	}
	
	while(1) {
		
		// Send data to write
		commWriteBytes((u_char*)data, size);

		// Wait for acknowledge
		commReadBytes((u_char*)&ret, 4);
		
		if ( ret == -2 ) {
			continue;
		}
		
		if ( ret == -3 ) {
			continue;
		}
		
		break;
		
	}
	
	return ret;
	
}

int fsPuts(int fd, char* text) {
	return fsWrite( fd, (u_char*)text, strlen(text) );
}

int fsRead(int fd, u_char* data, int size) {
	
	int ret,timeout,received;
	SFS_READSTRUCT info;
	SFS_READREPLY reply;
	
	// Send command
	commWriteBytes((u_char*)"~FRD", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file handle and read length
	info.fd = fd;
	info.pad = 0;
	info.length = size;
	commWriteBytes((u_char*)&info, sizeof(SFS_READSTRUCT));
	
	// Get response
	if ( commReadBytes((u_char*)&reply, sizeof(SFS_READREPLY)) != sizeof(SFS_READREPLY) ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( reply.ret == 1 ) {
		return FS_ERR_NOTOPEN;
	} else if ( reply.ret == 2 ) {
		return FS_ERR_INVALID;
	} else if ( reply.ret == 3 ) {
		return FS_ERR_READERROR;
	}
	
	if ( reply.ret == 4 ) {
		_eof_hit = 1;
	}
	
	timeout = 0;
	received = 0;

	commReadBytesAsync(data, reply.length);
	
	ret = 75;
	commWriteBytes((u_char*)&ret, 1);
	
	while(1) {

		while( !commAsyncDone() ) {
			
			if ( commBytesPending() > received ) {
				received = commBytesPending();
				timeout = 0;
			}
			
			if ( timeout > 120 ) {
				break;
			}
			
			VSync(0);
			timeout++;
			
		}
		
		received = commBytesPending();
		
		commEndAsync();
		
		if ( received < reply.length ) {
			ret = 1;
			commWriteBytes((u_char*)&ret, 1);
			commReadBytesAsync(data, reply.length);
			continue;
		}
		
		if ( crc16(data, reply.length, 0) != reply.crc16 ) {
			ret = 2;
			commWriteBytes((u_char*)&ret, 1);
			commReadBytesAsync(data, reply.length);
			continue;
		}
		
		break;
		
	}
	
	ret = 0;
	commWriteBytes((u_char*)&ret, 1);
	
	return 0;
}

int fsGets(char* text, int length, int fd) {
	
	int ret,timeout,received;
	SFS_READSTRUCT info;
	SFS_READREPLY reply;
	
	// Send command
	commWriteBytes((u_char*)"~FGS", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file handle and read length
	info.fd = fd;
	info.pad = 0;
	info.length = length;
	commWriteBytes((u_char*)&info, sizeof(SFS_READSTRUCT));
	
	// Get response
	if ( commReadBytes((u_char*)&reply, sizeof(SFS_READREPLY)) != sizeof(SFS_READREPLY) ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( reply.ret == 1 ) {
		return FS_ERR_NOTOPEN;
	} else if ( reply.ret == 2 ) {
		return FS_ERR_INVALID;
	} else if ( reply.ret == 3 ) {
		return FS_ERR_READERROR;
	}
	
	if ( reply.ret == 4 ) {
		_eof_hit = 1;
	}
	
	timeout = 0;
	received = 0;

	commReadBytesAsync(text, reply.length);
	
	ret = 75;
	commWriteBytes((u_char*)&ret, 1);
	
	while(1) {

		while( !commAsyncDone() ) {
			
			if ( commBytesPending() > received ) {
				received = 
				timeout = 0;
			}
			
			if ( timeout > 120 ) {
				break;
			}
			
			VSync(0);
			timeout++;
			
		}
		
		received = commBytesPending();
		
		commEndAsync();
		VSync(0);
		
		if ( received < reply.length ) {
			ret = 1;
			commWriteBytes((u_char*)&ret, 1);
			commReadBytesAsync(text, reply.length);
			continue;
		}
		
		if ( crc16(text, reply.length, 0) != reply.crc16 ) {
			ret = 2;
			commWriteBytes((u_char*)&ret, 1);
			commReadBytesAsync(text, reply.length);
			continue;
		}
		
		break;
		
	}
	
	ret = 0;
	commWriteBytes((u_char*)&ret, 1);
	
	return 0;
	
}

int fsSeek(int fd, int pos, int mode) {
	
	int ret;
	SFS_SEEKSTRUCT info;
	
	commWriteBytes((u_char*)"~FSK", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return -1;
	}
	
	info.fd		= fd;
	info.mode	= mode;
	info.offset	= pos;
	
	commWriteBytes((u_char*)&info, sizeof(SFS_SEEKSTRUCT));
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)ret, 1);
	switch(ret) {
		case 1:
			return FS_ERR_NOTOPEN;
		case 2:
			return FS_ERR_INVALID;
		case 3:
			return FS_ERR_SEEKERROR;
	}
	
	return 0;
	
}

int fsTell(int fd) {
	
	int ret;
	
	// Send command
	commWriteBytes((u_char*)"~FTL", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file descriptor
	commWriteBytes((u_char*)&fd, 1);
	
	// Wait for acknowledge
	if ( commReadBytes((u_char*)&ret, 4) != 4 ) {
		return FS_ERR_TIMEOUT;
	}
	
	switch(ret) {
		case -1:
			return FS_ERR_NOTOPEN;
		case -2:
			return FS_ERR_INVALID;
	}
	
	return ret;
	
}

int fsEof() {
	
	return _eof_hit;
	
}

int fsFindFirst(char* wildcard, FS_DIRENT* dir) {
	
	int ret;
	
	// Send command
	commWriteBytes((u_char*)"~FLF", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	ret = strlen(wildcard);
	commWriteBytes((u_char*)&ret, 1);
	commWriteBytes((u_char*)wildcard, ret);
	
	memset(dir->filename, 0x0, 64);
	
	if ( commReadBytes((u_char*)dir, 12) != 12 ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( dir->size == -1 ) {
		return FS_ERR_DIRERROR;
	} else if ( dir->size == -2 ) {
		return FS_ERR_DIREMPTY;
	}
	
	ret = 75;
	commWriteBytes((u_char*)&ret, 1);
	
	if (commReadBytes( dir->filename, dir->length ) != dir->length) {
		return FS_ERR_TIMEOUT;
	}
	
	return 0;
	
}

int fsFindNext(FS_DIRENT* dir) {
	
	int ret;
	
	// Send command
	commWriteBytes((u_char*)"~FLN", 4);
	
	memset(dir->filename, 0x0, 64);
	
	if ( commReadBytes((u_char*)dir, 12) != 12 ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( dir->size == -1 ) {
		return FS_ERR_NODIROPEN;
	} else if ( dir->size == -2 ) {
		return FS_ERR_DIRDONE;
	}
	
	ret = 75;
	commWriteBytes((u_char*)&ret, 1);
	
	if (commReadBytes( dir->filename, dir->length ) != dir->length) {
		return FS_ERR_TIMEOUT;
	}
	
	return 0;
	
}

int fsListFiles(char* wildcard, int items, int offset, FS_DIRENT* dir) {
	
	int ret;
	SFS_DIRPARAM param;
	
	// Send command
	commWriteBytes((u_char*)"~FLS", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	param.num		= items;
	param.offset	= offset;
	ret = strlen(wildcard);
	
	commWriteBytes((u_char*)&param, sizeof(SFS_DIRPARAM));
	commWriteBytes((u_char*)&ret, 1);
	commWriteBytes((u_char*)wildcard, ret);
	
	if ( commReadBytes((u_char*)&param, sizeof(SFS_DIRPARAM)) != sizeof(SFS_DIRPARAM) ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( param.num == -1 ) {
		return FS_ERR_DIRERROR;
	}
	
	if ( param.num > 0 ) {
		
		commReadBytesAsync((u_char*)dir, sizeof(FS_DIRENT)*param.num);

		ret = 'K';
		commWriteBytes((u_char*)&ret, 1);

		while( !commAsyncDone() ) {
			VSync(0);
		}

		commEndAsync();

	}
	
	_list_total = param.offset;
	return param.num;
			
}

int fsListTotal() {
	return _list_total;
}

int fsStat(char* filename, FS_STAT* st) {
	
	FS_STAT tst;
	int ret,len;
	
	if ( !st ) {
		st = &tst;
	}
	
	len = strlen(filename);
	memset(st, 0, sizeof(FS_STAT));
	
	// Send command
	commWriteBytes((u_char*)"~FST", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file flags and file name
	commWriteBytes((u_char*)&len, 1);
	commWriteBytes((u_char*)filename, len);
	
	if ( commReadBytes((u_char*)st, 10) != 10 ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( st->size < 0 ) {
		return FS_ERR_NOTFOUND;
	}
	
	return st->size;
	
}

int fsChdir(char* path) {
	
	int ret,len;
		
	len = strlen(path);
	
	// Send command
	commWriteBytes((u_char*)"~FCD", 4);
	
	// Wait for acknowledge
	ret = 0;
	commReadBytes((u_char*)&ret, 1);
	if ( ret != 'K' ) {
		return FS_ERR_TIMEOUT;
	}
	
	// Send file flags and file name
	commWriteBytes((u_char*)&len, 1);
	commWriteBytes((u_char*)path, len);
	
	ret = 0;
	if ( commReadBytes((u_char*)&ret, 1) != 1 ) {
		return FS_ERR_TIMEOUT;
	}
	
	return 0;
	
}

int fsGetcwd(char* path) {
	
	int ret;
	
	memset(path, 0x0, 128);
	
	// Send command
	commWriteBytes((u_char*)"~FWD", 4);
	
	// Wait for acknowledge
	ret = 0;
	if ( commReadBytes((u_char*)&ret, 1) != 1 ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( ret > 0 ) {
		commReadBytes((u_char*)path, ret);
	}
	
	return 0;
	
}

int fsQuickRead(char* file, char* buffer, int length, int offset) {
	
	int ret;
	int timeout,received;
	SFS_QUICKREAD param;
	
	commWriteBytes((u_char*)"~FRQ", 4);
	
	ret = 0;
	if ( commReadBytes((u_char*)&ret, 1) != 1 ) {
		return FS_ERR_TIMEOUT;
	}
	
	ret = strlen(file);
	commWriteBytes((u_char*)&ret, 1);
	commWriteBytes((u_char*)file, ret);
	
	ret = 0;
	if ( commReadBytes((u_char*)&ret, 1) != 1 ) {
		return FS_ERR_TIMEOUT;
	}
	
	if ( ret ) {
		return FS_ERR_NOTFOUND;
	}
	
	commWriteBytes((u_char*)&length, 4);
	commWriteBytes((u_char*)&offset, 4);
	
	if ( commReadBytes((u_char*)&param, sizeof(SFS_QUICKREAD)) 
		!= sizeof(SFS_QUICKREAD) ) {
		
		return FS_ERR_TIMEOUT;
		
	}
	
	if ( param.response == 1 ) {
		return FS_ERR_READERROR;
	} else if ( param.response == 2 ) {
		return FS_ERR_SEEKERROR;
	}
	
	commReadBytesAsync(buffer, param.length);
	
	ret = 75;
	commWriteBytes((u_char*)&ret, 1);
	
	timeout = 0;
	received = 0;
	
	while(1) {

		while( !commAsyncDone() ) {
			
			if ( commBytesPending() > received ) {
				received = commBytesPending();
				timeout = 0;
			}
			
			if ( timeout > 120 ) {
				break;
			}
			
			VSync(0);
			timeout++;
			
		}
		
		received = commBytesPending();
		
		commEndAsync();
		
		if ( received < param.length ) {
			ret = 1;
			commWriteBytes((u_char*)&ret, 1);
			commReadBytesAsync(buffer, param.length);
			continue;
		}
		
		if ( crc16(buffer, param.length, 0) != param.crc ) {
			ret = 2;
			commWriteBytes((u_char*)&ret, 1);
			commReadBytesAsync(buffer, param.length);
			continue;
		}
		
		break;
		
	}
	
	ret = 0;
	commWriteBytes((u_char*)&ret, 2);
	
	return param.length;
}