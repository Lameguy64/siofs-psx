/* 
 * File:   GraphicsClass.h
 * Author: Lameguy64
 *
 * Created on June 7, 2018, 7:33 PM
 */

#ifndef GRAPHICSCLASS_H
#define GRAPHICSCLASS_H

#include <sys/types.h>
#include <LIBETC.H>
#include <LIBGTE.H>
#include <LIBGPU.H>

#ifndef nullptr
#define nullptr	0
#endif

#define OT_LENGTH		128
#define PACKET_LENGTH	4096

class GraphicsClass {
public:
	
	GraphicsClass();
	virtual ~GraphicsClass();
	
	void Init(int xres, int yres);
	void InitFont();
	
	void Display();
	
	void SortBox(int x, int y, int w, int h, int r, int g, int b);
	void SortText(char* text, int x, int y);
	
	int LoadTim(u_long *timaddr);
	
	DISPENV	ds_disp[2];
	DRAWENV ds_draw[2];
	
	u_long ds_ot[2][OT_LENGTH];
	u_long ds_packet[2][PACKET_LENGTH];
	char* ds_next;
	int ds_active;
	
private:
	
	u_short fnt_tpage,fnt_clut;
	
};

#endif /* GRAPHICSCLASS_H */

