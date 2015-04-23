/*************************************************************************
    > File Name: nxui_frame.c
    > Author: liujie
    > Mail: 1170381285@qq.com 
    > Created Time: Mon 20 Apr 2015 04:56:59 PM CST
 ************************************************************************/

#include "nxui.h"


/* Select renderer -- Some additional logic would be required to support
 * pixel depths that are not directly addressable (1,2,4, and 24).
 */

#if CONFIG_EXAMPLES_NXUI_BPP == 1
#  define RENDERER nxf_convert_1bpp
#elif CONFIG_EXAMPLES_NXUI_BPP == 2
#  define RENDERER nxf_convert_2bpp
#elif CONFIG_EXAMPLES_NXUI_BPP == 4
#  define RENDERER nxf_convert_4bpp
#elif CONFIG_EXAMPLES_NXUI_BPP == 8
#  define RENDERER nxf_convert_8bpp
#elif CONFIG_EXAMPLES_NXUI_BPP == 16
#  define RENDERER nxf_convert_16bpp
#elif CONFIG_EXAMPLES_NXUI_BPP == 24
#  define RENDERER nxf_convert_24bpp
#elif  CONFIG_EXAMPLES_NXUI_BPP == 32
#  define RENDERER nxf_convert_32bpp
#else
#  error "Unsupported CONFIG_EXAMPLES_NXUI_BPP"
#endif

/* Vertical scaling */

#if defined(CONFIG_EXAMPLES_NXIMAGE_YSCALEp5)

/* Read two rows, output one averaged row */

#define NINPUT_ROWS  2
#define NOUTPUT_ROWS 1

#elif defined(CONFIG_EXAMPLES_NXIMAGE_YSCALE1p5)
/* Read two rows, output three rows */

#define NINPUT_ROWS  2
#define NOUTPUT_ROWS 3

#elif defined(CONFIG_EXAMPLES_NXIMAGE_YSCALE2p0)
/* Read one row, output two rows */

#define NINPUT_ROWS  1
#define NOUTPUT_ROWS 2

#else
/* Read one rows, output one or two rows */

#define NINPUT_ROWS  1
#define NOUTPUT_ROWS 1
#endif


struct nxui_frame_s g_frame[NXUI_FRAME_NUM];
struct nxui_frame_s *g_current;

static void nxui_initglyph(FAR uint8_t *glyph, uint8_t height,
                              uint8_t width, uint8_t stride)
{
  FAR nxgl_mxpixel_t *ptr;
#if CONFIG_EXAMPLES_NXUI_BPP < 8
  nxgl_mxpixel_t pixel;
#endif
  unsigned int row;
  unsigned int col;
  /* Initialize the glyph memory to the background color */
#if CONFIG_EXAMPLES_NXUI_BPP < 8
  pixel  = CONFIG_EXAMPLES_NXUI_BGCOLOR;
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
#  if CONFIG_EXAMPLES_NXUI_BPP == 1
  /* Pack 1-bit pixels into a 2-bits */
  pixel &= 0x01;
  pixel  = (pixel) << 1 |pixel;
#  endif
#  if CONFIG_EXAMPLES_NXUI_BPP < 4
  /* Pack 2-bit pixels into a nibble */
  pixel &= 0x03;
  pixel  = (pixel) << 2 |pixel;
#  endif
  /* Pack 4-bit nibbles into a byte */
  pixel &= 0x0f;
  pixel  = (pixel) << 4 | pixel;
  ptr    = (FAR nxgl_mxpixel_t *)glyph;
  for (row = 0; row < height; row++)
    {
      for (col = 0; col < stride; col++)
        {
          /* Transfer the packed bytes into the buffer */
          *ptr++ = pixel;
        }
    }
#elif CONFIG_EXAMPLES_NXUI_BPP == 24
# error "Additional logic is needed here for 24bpp support"
#else /* CONFIG_EXAMPLES_NXUI_BPP = {8,16,32} */
  ptr = (FAR nxgl_mxpixel_t *)glyph;
  for (row = 0; row < height; row++)
    {
      /* Just copy the color value into the glyph memory */
      for (col = 0; col < width; col++)
        {
          *ptr++ = CONFIG_EXAMPLES_NXUI_BGCOLOR;
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
        }
    }
#endif
}

static int draw_text(FAR struct nxui_textrect_s *rect_desc, char *text) {
/* Get the default font handle */
	NXHANDLE hfont;
	hfont = nxf_getfonthandle(rect_desc->font_id);
    if (!hfont) {
		printf("draw_text: Failed to get font handle: %d\n", errno);
		g_nxui.code = NXEXIT_FONTOPEN;
		goto errout;
    }
 FAR const struct nx_font_s *fontset;
  FAR const struct nx_fontbitmap_s *fbm;
  FAR uint8_t *glyph;
  FAR const char *ptr;
  FAR struct nxgl_point_s fpos;
  FAR struct nxgl_rect_s fdest;
  FAR const void *src[CONFIG_NX_NPLANES];
  unsigned int glyphsize;
  unsigned int mxstride;
  int ret;
  /* Get information about the font we are going to use */
  fontset = nxf_getfontset(hfont);
  fpos.x = rect_desc->rect.pt1.x + rect_desc->x_offset;
  fpos.y = rect_desc->rect.pt1.y + rect_desc->y_offset;
  /* Allocate a bit of memory to hold the largest rendered font */
  mxstride  = (fontset->mxwidth * CONFIG_EXAMPLES_NXUI_BPP + 7) >> 3;
  glyphsize = (unsigned int)fontset->mxheight * mxstride;
  glyph     = (FAR uint8_t*)malloc(glyphsize);
  /* NOTE: no check for failure to allocate the memory.  In a real application
   * you would need to handle that event.
   */
for (ptr = text; *ptr; ptr++)
    {
      /* Get the bitmap font for this ASCII code */
      fbm = nxf_getbitmap(hfont, *ptr);
      if (fbm)
        {
          uint8_t fheight;      /* Height of this glyph (in rows) */
          uint8_t fwidth;       /* Width of this glyph (in pixels) */
          uint8_t fstride;      /* Width of the glyph row (in bytes) */
          /* Get information about the font bitmap */
          fwidth  = fbm->metric.width + fbm->metric.xoffset;
          fheight = fbm->metric.height + fbm->metric.yoffset;
          fstride = (fwidth * CONFIG_EXAMPLES_NXUI_BPP + 7) >> 3;
          /* Initialize the glyph memory to the background color */
          nxui_initglyph(glyph, fheight, fwidth, fstride);
          /* Then render the glyph into the allocated memory */
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
          (void)RENDERER((FAR nxgl_mxpixel_t*)glyph, fheight, fwidth,
                         fstride, fbm, rect_desc->font_color);
          /* Describe the destination of the font with a rectangle */
          fdest.pt1.x = fpos.x;
          fdest.pt1.y = fpos.y;
          fdest.pt2.x = fpos.x + fwidth - 1;
          fdest.pt2.y = fpos.y + fheight - 1;
          /* Then put the font on the display */
          src[0] = (FAR const void *)glyph;
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
          ret = nx_bitmap(g_nxui.hbkgd, &fdest, src, &fpos, fstride);
          if (ret < 0)
            {
              printf("draw_text: nx_bitmapwindow failed: %d\n", errno);
            }
           /* Skip to the right the width of the font */
          fpos.x += fwidth;
        }
      else
        {
           /* No bitmap (probably because the font is a space).  Skip to the
            * right the width of a space.
            */
          fpos.x += fontset->spwidth;
        }
    }
  /* Free the allocated glyph */
  free(glyph);	
errout:
	return g_nxui.code;
}

static inline int draw_text_rect(FAR struct nxui_textrect_s *dest) {
	/* draw lines first **************************************************/
	struct nxgl_vector_s line;
    nxgl_mxpixel_t color[CONFIG_NX_NPLANES];
    color[0] = dest->line_color;
	line.pt1.x = dest->rect.pt1.x;
	line.pt1.y = dest->rect.pt1.y;
	line.pt2.x = dest->rect.pt2.x;
	line.pt2.y = dest->rect.pt1.y;
	nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	line.pt1.x = dest->rect.pt1.x;
	line.pt1.y = dest->rect.pt1.y;
	line.pt2.x = dest->rect.pt1.x;
	line.pt2.y = dest->rect.pt2.y;
	nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	line.pt1.x = dest->rect.pt2.x;
	line.pt1.y = dest->rect.pt2.y;
	line.pt2.x = dest->rect.pt1.x;
	line.pt2.y = dest->rect.pt2.y;
	nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	line.pt1.x = dest->rect.pt2.x;
	line.pt1.y = dest->rect.pt2.y;
	line.pt2.x = dest->rect.pt2.x;
	line.pt2.y = dest->rect.pt1.y;
	nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	/* draw test *********************************************************/
	switch(dest->rect_type) {
		case NXUI_PATIENT_NAME:return draw_text(dest, FAR g_patient.name); 
		//case NXUI_PATIENT_AGE: return draw_text(dest, FAR nxui_patient.age);
	}
}

int nxui_draw_frame(int which_frame) {
	int ret;
	struct nxui_textrect_s *pt;

	g_current = &g_frame[which_frame];
	for(ptr = g_current->text_rect; ptr; ptr = ptr->next) {
		ret = draw_text_rect(ptr);
	}
	return ret;
}

static inline void frame_0_init() {
	struct nxui_frame_s *frame = &g_frame[0];
	struct nxui_textrect_s *curr;
	frame->text_rect = (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
	curr = frame->text_rect;
	curr->rect_type = NXUI_PATIENT_NAME;
	curr->x_offset = 0x05;
	curr->y_offset = 0x05;
	curr->bkgd_color = 0x00;
	curr->line_color = RGB24_YELLOWGREEN;//0xdf;
	curr->line_width = 0x02;
	curr->rect.pt1.x = 10;
	curr->rect.pt1.y = 10;
	curr->rect.pt2.x = 110;
	curr->rect.pt2.y = 60;
	curr->next = NULL;
	curr->font_id = NXFONT_DEFAULT;
	curr->font_color = RGB24_GREEN;
}

void nxui_frame_init() {
	strcpy(g_patient.name, "xiao hong");
	frame_0_init();
}

