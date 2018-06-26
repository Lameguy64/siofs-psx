/* 
 * File:   main.cpp
 * Author: Lameguy64
 *
 * Created on April 25, 2018, 8:13 PM
 * 
 * Simple sample/test program for the SioFS library.
 */

#include <sys/TYPES.H>
#include <STDIO.H>
#include <STRING.H>
#include <LIBETC.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBSIO.H>
#include "comm.h"
#include "siofs.h"
#include "font.h"
#include "graphics.h"

GraphicsClass graph;

void message(char* text) {
	
	// Very simple message display routine
	
	for(int i=0; i<120; i++) {
		graph.SortText(text, 8, 16);
		graph.Display();
	}
	
}

void fileTest() {
	
	// A file write demo
	
	char *text = "The quick brown fox jumps over the lazy dog.";
	char buff[64];
	int fd;
	
	for(int i=0; i<4; i++) {
		graph.SortText("Writing text to file...", 8, 16);
		graph.Display();
	}
	
	fd = fsOpen("ps.txt", FS_WRITE);
	
	fsPuts(fd, text);
	fsPuts(fd, "\n\nConsole Info:\n");
	fsPuts(fd, (char*)0xbfc0012c);
	fsPuts(fd, "\n");
	fsPuts(fd, (char*)0xbfc7ff32);
	fsPuts(fd, "\n");
	fsPuts(fd, (char*)0xbfc7ff54);
	
	fsClose(fd);
	
	for(int i=0; i<4; i++) {
		graph.SortText("Reading text from file...", 8, 16);
		graph.Display();
	}
	
	fd = fsOpen("ps.txt", FS_READ);
	fsGets(buff, 64, fd);
	fsClose(fd);
	
	if ( strncmp(buff, text, strlen(text)) ) {
		message("ERROR: Read string mismatch.");
		return;
	}
	
	message("Test successful.");
	
}

void dumpBIOS() {
	
	// Another file write demo
	
	int fd = fsOpen("bios.bin", FS_WRITE|FS_BINARY);
	
	if ( fd < 0 ) {
		message("ERROR: Cannot create bios.bin file.");
		return;
	}
	
	char* bios_addr = (char*)0xbfc00000;
	int bios_wrote = 0;
	int bios_len = 524288;
	
	while( bios_wrote < bios_len) {
		
		graph.SortText("Dumping BIOS to bios.bin...", 8, 16);
		
		graph.SortBox(52, 120-14, (196*((ONE*bios_wrote)/bios_len))/ONE, 28, 255, 255, 0);
		graph.SortBox(50, 120-16, 200, 32, 0, 0, 0);
		
		graph.Display();
		
		fsWrite(fd, (u_char*)(bios_addr+bios_wrote), 16384);
		bios_wrote += 16384;
		
	}
	
	fsClose(fd);
	
	message("BIOS dumped successfully.");
	
}

void fileBrowser(char* wildcard) {
	
	// File browser demo
	
	int press_up=0;
	int press_down=0;
	int press_left=0;
	int press_right=0;
	int press_open=1;
	
	int selection=0;
	
	char textbuff[128];
	char cwd[128];
	FS_DIRENT dir[22];
	int dir_items;
	int dir_total;
	int dir_offset = 0;
	int path_offset;
	int query = 0;
	
	for(int i=0; i<4; i++) {
		graph.SortText("Parsing directory...", 8, 16);
		graph.Display();
	}
	
	// Get a file listing
	dir_items = fsListFiles(wildcard, 22, 0, dir);
	
	if ( dir_items < 0 ) {
		message("ERROR: Cannot parse directory.");
		return;
	}
	
	// Get total number of files from the last listing
	dir_total = fsListTotal();
	
	if ( fsGetcwd(cwd) ) {
		message("ERROR: Cannot get current working directory.");
		return;
	}
	
	while(1) {
		
		int pad = PadRead(0);
		
		if ( pad & PADLup ) {
			if ( ( !press_up ) ) {
				if ( selection > 0 ) {
					selection--;
				} else if ( dir_offset > 0 ) {
					dir_offset -= 22;
					selection = 21;
					query = 1;
				}
				press_up = 1;
			}
		} else {
			press_up = 0;
		}
		
		if ( pad & PADLdown ) {
			if ( !press_down ) {
				if ( ( selection < ( dir_items-1 ) ) ) {
					selection++;
				} else {
					if ( ( dir_offset+dir_items ) < dir_total ) {
						dir_offset += 22;
						selection = 0;
						query = 1;
					}
				}
				press_down = 1;
			}
		} else {
			press_down = 0;
		}
		
		if ( pad & PADLleft ) {
			if ( ( !press_left ) && ( dir_offset > 0 ) ) {
				dir_offset -= 22;
				query = 1;
				press_left = 1;
				selection = 0;
			}
		} else {
			press_left = 0;
		}
		
		if ( pad & PADLright ) {
			if ( ( !press_right ) && ( ( dir_offset+22) < dir_total ) ) {
				dir_offset += 22;
				query = 1;
				press_right = 1;
				selection = 0;
			}
		} else {
			press_right = 0;
		}
		
		// Go to selected directory and rescan
		if ( pad & PADRdown ) {
			if ( ( !press_open ) && ( dir[selection].flags&0x1 ) ) {
				fsChdir( dir[selection].filename );
				dir_offset = 0;
				query = 1;
				fsGetcwd(cwd);
				press_open = 1;
				selection = 0;
			}
		} else {
			press_open = 0;
		}
		
		if ( pad & PADRright ) {
			
			break;
			
		}
		
		// Rescan directory
		if ( query ) {
			
			dir_items = fsListFiles(wildcard, 22, dir_offset, dir);
			
			if ( dir_items < 0 ) {
				message("ERROR: Cannot parse directory.");
				return;
			}
			
			dir_total = fsListTotal();
			query = 0;
			
		}
		
		if ( strlen(cwd) > 32 ) {
			
			path_offset = strlen(cwd)-29;
			
			if ( path_offset < 0 ) {
				path_offset = 0;
			}
			
			sprintf(textbuff, "Path: ...%s\n", cwd+path_offset);
			
		} else {
			
			sprintf(textbuff, "Path: %s\n", cwd);
			
		}
		
		graph.SortText(textbuff, 8, 8);
		
		sprintf(textbuff, "Files: %d", dir_total);
		graph.SortText(textbuff, 8, 24);
		
		sprintf(textbuff, "Page: %d/%d", 1+(dir_offset/22), (dir_total+21)/22);
		graph.SortText(textbuff, 8, 32);
		
		for(int i=0; i<22; i++) {
			
			if ( i >= dir_items ) {
				break;
			}
			
			if ( dir[i].flags&0x1 ) {
				graph.SortText("~", 0, 48+(8*i));
			}
			
			graph.SortText(dir[i].filename, 8, 48+(8*i));
			
			if ( i == selection ) {
				graph.SortBox(8, 48+(8*i), 304, 8, 0, 0, 127);
			}
			
		}
		
		graph.Display();
		
	}
	
}

void init() {
	
	PadInit( 0 );
	graph.Init(320, 240);
	
}

int main(int argc, char** argv) {
	
	init();
	
	for(int i=0; i<4; i++) {
		graph.SortText("Initializing SIOFS...", 8, 16);
		graph.Display();
	}
	
	if ( fsInit(115200) == 0 ) {
		
		for(int i=0; i<4; i++) {
			graph.SortText("ERROR: No SIOFS host detected...", 8, 16);
			graph.Display();
		}
		
		return 0;
		
	}
	
	char buffer[128];
	int res = fsQuickRead("bios.bin", buffer, 128, 128);
	printf("Result = %d\n", res);
	
	if ( memcmp((char*)0xbfc00080, buffer, 128) == 0 ) {
		printf("Data matched.");
	}
	
	char* menu[] = {
		"Save and read a text file",
		"Dump console BIOS",
		"File browser",	
	};
	int selection = 0;
	int press_up=0;
	int press_down=0;
	
	while(1) {
		
		int pad = PadRead(0);
		
		if ( pad & PADLup ) {
			
			if ( ( press_down == 0 ) & ( selection > 0 ) ) {
				selection--;
				press_down = 1;
			}
			
		} else {
			
			press_down = 0;
			
		}
		
		if ( pad & PADLdown ) {
			
			if ( ( press_up == 0 ) && ( selection < 2 ) ) {
				selection++;
				press_up = 1;
			}
			
		} else {
			
			press_up = 0;
					
		}
		
		if ( pad & PADRdown ) {
			
			switch(selection) {
				case 0:
					fileTest();
					break;
				case 1:
					dumpBIOS();
					break;
				case 2:
					fileBrowser("*.*");
					break;
			}
			
		}
		
		graph.SortText("SIOFS Test Program", 8, 16);
		
		for(int i=0; i<3; i++) {
			graph.SortText(menu[i], 8, 32+(8*i));
		}
		
		graph.SortBox(8, 32+(8*selection), 304, 8, 0, 0, 127);
		graph.Display();
		
	}
	
	return 0;
}

