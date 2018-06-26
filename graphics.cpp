/* 
 * File:   GraphicsClass.cpp
 * Author: Lameguy64
 * 
 * Created on June 7, 2018, 7:33 PM
 */

#include "font.h"
#include "graphics.h"

GraphicsClass::GraphicsClass() {
	ds_next = nullptr;
	ds_active = 0;
}

GraphicsClass::~GraphicsClass() {
}

void GraphicsClass::Init(int xres, int yres) {
	
	ResetGraph(0);
	
	SetDefDispEnv(&ds_disp[0], 0, 0, xres, yres);
	SetDefDispEnv(&ds_disp[1], xres, 0, xres, yres);
	
	SetDefDrawEnv(&ds_draw[0], xres, 0, xres, yres);
	SetDefDrawEnv(&ds_draw[1], 0, 0, xres, yres);
	
	setRGB0(&ds_draw[0], 0, 63, 0);	ds_draw[0].isbg = 1;
	setRGB0(&ds_draw[1], 0, 63, 0);	ds_draw[1].isbg = 1;
	
	PutDispEnv(&ds_disp[0]);
	PutDrawEnv(&ds_draw[0]);
	
	ClearOTag(ds_ot[0], OT_LENGTH);
	ClearOTag(ds_ot[1], OT_LENGTH);
	
	ds_active = 0;
	ds_next = (char*)ds_packet[0];
	
	InitFont();
	
}

void GraphicsClass::InitFont() {
	
	LoadTim((u_long*)font);
	
	fnt_tpage = getTPage(0, 0, 960, 0);
	fnt_clut = getClut(960, 128);
	
	ds_draw[0].tpage = fnt_tpage;
	ds_draw[1].tpage = fnt_tpage;
	
}

void GraphicsClass::Display() {
	
	ds_active = !ds_active;
	
	DrawSync(0);
	
	VSync(0);
	PutDispEnv(&ds_disp[ds_active]);
	PutDrawEnv(&ds_draw[ds_active]);
	SetDispMask(1);
	
	DrawOTag(ds_ot[!ds_active]);
	ClearOTag(ds_ot[ds_active], OT_LENGTH);
	
	ds_next = (char*)ds_packet[ds_active];
	
}

void GraphicsClass::SortBox(int x, int y, int w, int h, int r, int g, int b) {
	
	TILE *box = (TILE*)ds_next;
	
	setTile(box);
	setXY0(box, x, y);
	setWH(box, w, h);
	setRGB0(box, r, g, b);
	addPrim(ds_ot[ds_active], box);
	
	ds_next = (char*)(box+1);
	
}

void GraphicsClass::SortText(char* txt, int x, int y) {
	
	SPRT_8 *csprt = (SPRT_8*)ds_next;
	int c;
	
	while( *txt != 0 ) {
		
		c = (*txt)-33;
		if ( c >= 0 ) {
			
			setSprt8(csprt);
			setXY0(csprt, x, y);
			setUV0(csprt, 8*(c&7), 8*(c>>3));
			setRGB0(csprt, 128, 128, 128);
			csprt->clut = fnt_clut;
			addPrim(ds_ot[ds_active], csprt);
			csprt++;
			
		}
		
		x += 8;
		txt++;
		
	}
	
	ds_next = (char*)csprt;
	
}

int GraphicsClass::LoadTim(u_long *timaddr) {

	TIM_IMAGE timinfo;

	// Get the TIM's header
	if (OpenTIM(timaddr) != 0)
	{
		#if DEBUG == 2
		printf("Error getting TIM info in address %p\n", timaddr);
		#endif
		return 1;
	}
	
	ReadTIM(&timinfo);

	// Upload the TIM image data to the framebuffer
	LoadImage(timinfo.prect, timinfo.paddr);
	DrawSync(0);

	// If TIM has a CLUT (if color depth is lower than 16-bit), upload it as well
	if ((timinfo.mode & 3) < 2)
	{
		LoadImage(timinfo.crect, timinfo.caddr);
		DrawSync(0);
	}

	return 0;

}