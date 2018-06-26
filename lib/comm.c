/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <SYS/TYPES.H>
#include <MALLOC.H>
#include <LIBETC.H>
#include <LIBSIO.H>
#include "comm.h"

static u_char *_bytes_dest;
static u_char _byte_buffer[COMM_BUFFER_MAX];
static int _bytes_read = 0;
static int _bytes_toread = 0;
static int _async_received = 0;
static int _async_done = 0;


static void _serialCallback() {
	
	if ( _bytes_read < COMM_BUFFER_MAX ) {
		_byte_buffer[_bytes_read] = _sio_control(0, 4, 0);
		_bytes_read++;
	} else {
		_sio_control(0, 4, 0);
	}
	
	_sio_control(2, 1, 0);
	
}

static void _serialAsyncCallback() {
	
	if ( _bytes_read < _bytes_toread ) {
		_bytes_dest[_bytes_read] = _sio_control( 0, 4, 0 );
		_bytes_read++;
		if ( _bytes_read == _bytes_toread ) {
			_async_done = 1;
		}
	} else {
		_sio_control(0, 4, 0);
	}
	
	_sio_control(2, 1, 0);	
	
}

void commInit() {
	
	/* 8bit, no-parity, 1 stop-bit */
	_sio_control( 1, 2, MR_SB_01 | MR_CHLEN_8 | 0x02 );
	/* Set baud rate */
	_sio_control( 1, 3, 9600 );
	/* No handshaking */
	_sio_control( 1, 1, CR_RXEN | CR_RXIEN | CR_TXEN );
	
	Sio1Callback( _serialCallback );
}

void commSetRate(int baud) {
	
	_sio_control( 1, 3, baud );
	
}

int commReadBytes(u_char* buffer, int bytes) {
	
	int read_total = 0;
	int timeout_counter = 0;
	
	while( read_total < bytes ) {
		
		if ( _bytes_read > 0 ) {
			memcpy( buffer+read_total, _byte_buffer, _bytes_read );
			read_total += _bytes_read;
			_bytes_read = 0;
		}
		
		if ( timeout_counter > COMM_TIMEOUT ) {
			break;
		}
		
		VSync( 0 );
		timeout_counter++;
	}
	
	return read_total;
}

void commReadBytesAsync(u_char* buffer, int bytes) {
	
	_bytes_dest = buffer;
	_bytes_read = 0;
	_bytes_toread = bytes;
	_async_done = 0;
	
	Sio1Callback( _serialAsyncCallback );
	
}

int commAsyncReceived() {
	
	int v = _async_received;
	_async_received = 0;
	
	return v;
	
}

int commAsyncDone() {
	return _async_done;
}

void commEndAsync() {
	
	_bytes_read = 0;
	Sio1Callback( _serialCallback );
	
}

int commBytesPending() {
	
	return _bytes_read;
	
}

void commWriteBytes(u_char* buffer, int bytes) {
	
	int i;
	
	for( i=0; i<bytes; i++ ) {
		
		while((_sio_control(0, 0, 0) & (SR_TXU|SR_TXRDY)) != (SR_TXU|SR_TXRDY));
		_sio_control(1, 4, buffer[i]);
		
	}
	
	VSync( 0 );
	
}