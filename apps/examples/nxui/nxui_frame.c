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

static void nxui_initglyph(FAR uint8_t *glyph, uint8_t height, uint8_t width, uint8_t stride,
		nxgl_mxpixel_t bkgd_color)
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
          *ptr++ = bkgd_color;//CONFIG_EXAMPLES_NXUI_BGCOLOR;
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
          nxui_initglyph(glyph, fheight, fwidth, fstride, rect_desc->bkgd_color);
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

static inline int draw_rect(NXWINDOW hwnd, struct nxgl_rect_s *dest, nxgl_mxpixel_t line_color,
		nxgl_mxpixel_t line_width, nxgl_mxpixel_t bkgd_color)
{
	int ret = 0;
	struct nxgl_vector_s line;
    nxgl_mxpixel_t color[CONFIG_NX_NPLANES];
    color[0] = line_color;
	line.pt1.x = dest->pt1.x;
	line.pt1.y = dest->pt1.y;
	line.pt2.x = dest->pt2.x;
	line.pt2.y = dest->pt1.y;
	ret |= nx_drawline(hwnd, &line, line_width, color);
	line.pt1.x = dest->pt1.x;
	line.pt1.y = dest->pt1.y;
	line.pt2.x = dest->pt1.x;
	line.pt2.y = dest->pt2.y;
	ret |= nx_drawline(hwnd, &line, line_width, color);
	//nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	line.pt1.x = dest->pt2.x;
	line.pt1.y = dest->pt2.y;
	line.pt2.x = dest->pt1.x;
	line.pt2.y = dest->pt2.y;
	ret |= nx_drawline(hwnd, &line, line_width, color);
	//nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	line.pt1.x = dest->pt2.x;
	line.pt1.y = dest->pt2.y;
	line.pt2.x = dest->pt2.x;
	line.pt2.y = dest->pt1.y;
	ret |= nx_drawline(hwnd, &line, line_width, color);
	//nx_drawline(g_nxui.hbkgd, &line, dest->line_width, color);
	
	color[0] = bkgd_color;
	struct nxgl_rect_s rect = {
		dest->pt1.x + line_width - 1,
		dest->pt1.y + line_width - 1,
		dest->pt2.x - line_width + 1,
		dest->pt2.y - line_width + 1
	};/* 为了防止背景色把边框覆盖掉 */
	ret |= nx_fill(hwnd, &rect, color);
	return ret;
}

void *scroll_text_left(void *rect) {
    nxgl_mxpixel_t color[CONFIG_NX_NPLANES];
	struct nxui_textrect_s *dest = (struct nxui_textrect_s *)rect;
	uint8_t row_text_num = dest->col_num;
	uint8_t src_text_num = strlen(dest->text);
	for(;;) {
		char dest_text[row_text_num + 1];
		int rest_len = src_text_num - dest->text_index;
		if(rest_len >= row_text_num) {
			strncpy(dest_text, dest->text + dest->text_index, row_text_num);
		}
		else {
			memcpy(dest_text, dest->text + dest->text_index, rest_len);
		    strncpy(dest_text + rest_len, dest->text, row_text_num - rest_len);
		}
		color[0] = dest->bkgd_color;
		struct nxgl_rect_s rect = {
			dest->rect.pt1.x + dest->line_width - 1,
			dest->rect.pt1.y + dest->line_width - 1,
			dest->rect.pt2.x - dest->line_width + 1,
			dest->rect.pt2.y - dest->line_width + 1
		};/* 为了防止背景色把边框覆盖掉 */
		nx_fill(g_nxui.hbkgd, &rect, color);
		draw_text(dest, dest_text);
		dest->text_index++;
		if(src_text_num > 0)
			dest->text_index %= src_text_num;
		sleep(1);
	}
}

void *scroll_text_up(void *rect) {
	nxgl_mxpixel_t color[CONFIG_NX_NPLANES];
	struct nxui_textrect_s *dest = (struct nxui_textrect_s *)rect;
	const char *ptr;
	int count = 0;
	int row_num = 0;
	for(ptr = dest->text; *ptr; ptr++) {
		if(count < dest->col_num - 1) {
			if(*ptr != '\n') {
				count++;
			}
			else {
				row_num++;
				count = 0;
			}
		}
		else {
			count = 0;
			row_num++;
		}
	}
	char text_array[row_num][dest->col_num + 2];
	memset(text_array, 0, row_num * (dest->col_num + 2) * sizeof(char));
	int row = 0;
	int col = 0;
	count = 0;
	for(ptr = dest->text; *ptr; ptr++) {
		if(count < dest->col_num - 1) {
			if(*ptr != '\n') {
				text_array[row][col++] = *ptr;
				text_array[row][col] = '\0';
				count++;
			}
			else {
				text_array[row++][col] = '\0';
				col = 0;
				count = 0;
			}
		}
		else {
			text_array[row][col++] = *ptr;
			text_array[row][col] = '\0';
			col = 0;
			count = 0;
			row++;
		}
	}

	for(;;) {
		int y_offset = dest->y_offset;
		color[0] = dest->bkgd_color;
		struct nxgl_rect_s rect = {
			dest->rect.pt1.x + dest->line_width - 1,
			dest->rect.pt1.y + dest->line_width - 1,
			dest->rect.pt2.x - dest->line_width + 1,
			dest->rect.pt2.y - dest->line_width + 1
		};/* 为了防止背景色把边框覆盖掉 */
		nx_fill(g_nxui.hbkgd, &rect, color);

		int rest_row = row_num - dest->text_index;
		if(rest_row >= dest->row_num) {
			count = 0;
			while(count < dest->row_num) {
				assert(dest->text_index < row_num);
				draw_text(dest, &text_array[dest->text_index + count]);
				dest->y_offset += dest->font_height; 	
				count++;
			}
		}
		else {
			count = 0;
			while(count < rest_row) {
				assert(dest->text_index < row_num);
				draw_text(dest, &text_array[dest->text_index + count]);
				dest->y_offset += dest->font_height; 	
				count++;
			}

			count = 0;
			while(count < dest->row_num - rest_row) {
				assert(dest->text_index < row_num);
				draw_text(dest, &text_array[count]);
				dest->y_offset += dest->font_height; 	
				count++;
			}
		}
		dest->text_index ++;
		dest->text_index %= row_num;
		dest->y_offset = y_offset;
		sleep(1);
	}

}

static void text_init(struct nxui_textrect_s *dest) {
	char buf[5];
	switch(dest->rect_type) {
		case NXUI_PATIENT_GENDER:
								if(dest->text != NULL)
									free(dest->text);
								dest->text = (g_patient.gender == NXUI_GENDER_MALE ? strdup("Male") : strdup("Female"));
								break; 
		case NXUI_SPECIAL_NOTE: dest->text = g_patient.special_note; break;
		case NXUI_PATIENT_NAME: dest->text = g_patient.name; break;
		case NXUI_PATIENT_AGE: 
								if(dest->text != NULL)
									free(dest->text);
								sprintf(buf, "%d", g_patient.age);
								dest->text = strdup(buf);
								break;
		case NXUI_PATIENT_DIET: dest->text = g_patient.diet; break;
		case NXUI_DRUG_ALLERGY: dest->text = g_patient.drug_allergy; break;
		case NXUI_PATIENT_POSITION:
								dest->text = g_patient.position;
								break;
		case NXUI_CARE_LEVEL:   if(dest->text != NULL)
									free(dest->text);
								dest->text = g_patient.care_level == NXUI_LEVEL_ONE ? strdup("level one") :
										     g_patient.care_level == NXUI_LEVEL_TWO ? strdup("level two") :
												                                      strdup("level three");
								break;
		case NXUI_BED_NO:       if(dest->text != NULL)
									free(dest->text);
								sprintf(buf, "%d", g_patient.bed_no);
								dest->text = strdup(buf);
								break;
		default:return;
	}
}

static int draw_text_wrap(struct nxui_textrect_s *dest) {
	text_init(dest);
	/*单行向左滚动*/
	if(dest->row_num == 1) {
		uint16_t row_text_num = dest->col_num;
		uint16_t src_text_num = strlen(dest->text);
		if(row_text_num < src_text_num) {
			char dest_text[row_text_num + 1];
			int rest_len = src_text_num - dest->text_index;
			if(rest_len >= row_text_num) {
				strncpy(dest_text, dest->text + dest->text_index, row_text_num);
			}
			else {
				memcpy(dest_text, dest->text + dest->text_index, rest_len);
				strncpy(dest_text + rest_len, dest->text, row_text_num - rest_len);
			}
			draw_text(dest, dest_text);
			dest->text_index++;
			if(src_text_num > 0)
				dest->text_index %= src_text_num;
			if(dest->text_scroll_handle == NULL) {
				pthread_create(&dest->text_scroll_handle, NULL, scroll_text_left, dest);
			}
		}
		else {
			draw_text(dest, dest->text);
			if(dest->text_scroll_handle != NULL) {
				if(pthread_cancel(dest->text_scroll_handle) != 0) {
					printf("pthread_cancel error in scroll_text!\n");
				}
			}
		}
	}
	/*每次向上滚动一整行*/
	else {
		const char *ptr;
		int count = 0;
		int row_num = 0;
		for(ptr = dest->text; *ptr; ptr++) {
			if(count < dest->col_num - 1) {
				if(*ptr != '\n') {
					count++;
				}
				else {
					row_num++;
					count = 0;
				}
			}
			else {
				count = 0;
				row_num++;
			}
		}
		char text_array[row_num][dest->col_num + 2];
		memset(text_array, 0, row_num * (dest->col_num + 2) * sizeof(char));
		int row = 0;
		int col = 0;
		count = 0;
		for(ptr = dest->text; *ptr; ptr++) {
			if(count < dest->col_num - 1) {
				if(*ptr != '\n') {
					text_array[row][col++] = *ptr;
					text_array[row][col] = '\0';
					count++;
				}
				else {
					text_array[row++][col] = '\0';
					col = 0;
					count = 0;
				}
			}
			else {
				text_array[row][col++] = *ptr;
				text_array[row][col] = '\0';
				col = 0;
				count = 0;
				row++;
			}
		}

		int y_offset = dest->y_offset;
		for(row = 0; row < dest->row_num; row++) {
			draw_text(dest, &text_array[row]);
			dest->y_offset += dest->font_height; 
		}
		dest->y_offset = y_offset;
			
		dest->text_index++;
		dest->text_index %= dest->row_num;

		if(row_num > dest->row_num) {
			if(dest->text_scroll_handle == NULL) {
				pthread_create(&dest->text_scroll_handle, NULL, scroll_text_up, dest);
			}
		}
		else {
			if(dest->text_scroll_handle != NULL) {
				if(pthread_cancel(dest->text_scroll_handle) != 0) {
					printf("pthread_cancel error in scroll_text!\n");
				}
			}
		}
	}
}

static inline int draw_text_rect(struct nxui_textrect_s *dest) {
    draw_rect(g_nxui.hbkgd, &dest->rect, dest->line_color, dest->line_width, dest->bkgd_color);
	draw_text_wrap(dest);
}

void draw_image(NXWINDOW hwnd, struct nxgl_rect_s *rect, nxgl_coord_t image_width,
		nxgl_coord_t image_height,
		const void *image)
{

  nxgl_mxpixel_t run[image_width];
  FAR const void *state = image;
  FAR struct nxgl_point_s pos;
  struct nxgl_rect_s dest;
  FAR const void *src[CONFIG_NX_NPLANES];
  nxgl_coord_t row;
  int ret;

  /* Center the image.  Note: these may extend off the display. */

  /*pos.x = (g_nximage.xres - SCALED_WIDTH) / 2;
  pos.y = (g_nximage.yres - SCALED_HEIGHT) / 2;*/
  pos.x = rect->pt1.x;
  pos.y = rect->pt1.y;

  /* Set up the invariant part of the destination bounding box */

  dest.pt1.x = pos.x;
  dest.pt2.x = pos.x + image_width - 1;
  //dest.pt2.x = rect->pt2.x;

  /* Now output the rows */

  for (row = 0; row < image_height; row++)
    {
      /* Read input row(s) */

     nximage_blitrow(run, &state, image_width);

      /* Output row[0] */

      dest.pt1.y = pos.y;
      dest.pt2.y = pos.y;

      src[0] = (FAR const void *)run;
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
      ret = nx_bitmap((NXWINDOW)hwnd, &dest, src, &pos, image_width*sizeof(nxgl_mxpixel_t));
      if (ret < 0)
        {
          printf("nximage_image: nx_bitmapwindow failed: %d\n", errno);
        }

      /* Increment the vertical position */

      pos.y++;
    }
}

static inline int draw_image_rect(struct nxui_imagerect_s *dest) {
    draw_rect(g_nxui.hbkgd, &dest->rect, dest->line_color, dest->line_width, dest->bkgd_color);

	struct nxgl_rect_s rect = {
		dest->rect.pt1.x + dest->line_width - 1, /*为了防止图像将边框覆盖掉*/
		dest->rect.pt1.y + dest->line_width - 1,
		dest->rect.pt2.x - dest->line_width + 1,
		dest->rect.pt2.y - dest->line_width + 1	
	};
	switch(dest->rect_type) {
		case NXUI_DOCTOR_PHOTO: draw_image(g_nxui.hbkgd, &rect, 160, 160, g_nuttx);
	}
}

int nxui_draw_frame(int which_frame) {
	int ret;
	struct nxui_textrect_s *text_rect;
	struct nxui_imagerect_s *image_rect;

	g_current = &g_frame[which_frame];
	for(text_rect = g_current->text_rect; text_rect; text_rect = text_rect->next) {
		ret = draw_text_rect(text_rect);
	}
	for(image_rect = g_current->image_rect; image_rect; image_rect = image_rect->next) {
		//ret = draw_image_rect(image_rect);
	}
	return ret;
}

static inline void set_rect_pos(struct nxgl_rect_s *dest,
		nxgl_coord_t pt1_x, nxgl_coord_t pt1_y,
		nxgl_coord_t pt2_x, nxgl_coord_t pt2_y) 
{
	dest->pt1.x = pt1_x;
	dest->pt1.y = pt1_y;
	dest->pt2.x = pt2_x;
	dest->pt2.y = pt2_y;
}

static inline void set_bkgd_image(void) {
	//struct nxgl_rect_s rect = {300,0,589,173};
//	draw_image(g_nxui.hbkgd, &rect, 290, 174, bitmap);
	struct nxgl_rect_s rect = {0,0,799,479};
	draw_image(g_nxui.hbkgd, &rect, 800, 480, bitmap);
}


/* 第一屏 */
static void frame_0_init(void) {
	struct nxui_frame_s *frame = &g_frame[0];
	struct nxui_textrect_s *curr_tr; /* current text rect */
	struct nxui_imagerect_s *curr_ir; /* current image rect */
	if(frame->text_rect == NULL) {
		frame->text_rect = (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));

		/* 姓名 *******************************************************************************/
		curr_tr = frame->text_rect;
		curr_tr->rect_type = NXUI_PATIENT_NAME;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 20, 10, 220, 70);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color =0xff3300;//RGB24_YELLOWGREEN;
		curr_tr->text = g_patient.name;
		curr_tr->col_num = 3;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 性别 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_PATIENT_GENDER;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = CONFIG_EXAMPLES_NXUI_BGCOLOR;
		curr_tr->line_color = 0x8cc63f;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 20, 10 + 60, 120, 10 + 60 + 60);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color =0xff3300; //RGB24_YELLOWGREEN;
		curr_tr->col_num = 4;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 年龄 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_PATIENT_AGE;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 120, 70, 220, 130);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->col_num = 4;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 床位号 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_BED_NO;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 220, 10, 300, 130);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->col_num = 4;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 护理级别 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_CARE_LEVEL;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = RGB24_SKYBLUE;//CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 550, 10, 750, 70);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->col_num = 16;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 体位 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_PATIENT_POSITION;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = RGB24_ORANGE;//CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 550, 100, 750, 160);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->col_num = 4;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 药物过敏 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_DRUG_ALLERGY;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = RGB24_YELLOW;//CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 550, 190, 750, 250);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->col_num = 4;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 饮食 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_PATIENT_DIET;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = 0xA020F0;//CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 550, 280, 750, 340);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->col_num = 16;
		curr_tr->row_num = 1;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;

		/* 特殊提醒 ***********************************************************************/
		curr_tr->next =  (struct nxui_textrect_s *)malloc(sizeof(struct nxui_textrect_s));
		curr_tr = curr_tr->next;
		curr_tr->rect_type = NXUI_SPECIAL_NOTE;
		curr_tr->x_offset = 10;
		curr_tr->y_offset = 10;
		curr_tr->bkgd_color = 0xf5f5d5;//CONFIG_EXAMPLES_NXUI_BGCOLOR;//RGB24_BROWN;
		curr_tr->line_color = 0x8cc63f;//RGB24_YELLOWGREEN;//0xdf;
		curr_tr->line_width = 5;
		set_rect_pos(&curr_tr->rect, 20, 360, 750, 460);
		curr_tr->next = NULL;
		curr_tr->font_id = NXFONT_DEFAULT;
		curr_tr->font_color = 0xff3300;//GB24_YELLOWGREEN;
		curr_tr->font_height = 20;
		curr_tr->col_num = 6;
		curr_tr->text_scroll_handle = NULL;
		curr_tr->text_index = 0;
		curr_tr->row_num = 2;
		/************************************************************************/

		frame->image_rect = (struct nxui_imagerect_s *)malloc(sizeof(struct nxui_imagerect_s));
		curr_ir = frame->image_rect;
		curr_ir->rect_type = NXUI_DOCTOR_PHOTO;
		curr_ir->x_offset = 0;
		curr_ir->y_offset = 0;
		curr_ir->bkgd_color = RGB24_GRAY;
		curr_ir->line_color = RGB24_YELLOWGREEN;
		curr_ir->line_width = 0x02;
		set_rect_pos(&curr_ir->rect, 300, 10, 
				120 + curr_ir->line_width * 2 + 160 - 1 , 10 + curr_ir->line_width * 2 + 160 - 1);
		curr_ir->next = NULL;
	}
}

static inline void frame_1_init(void) {
}

void nxui_frame_init(void) {
	strcpy(g_patient.name, "abcd");
	strcpy(g_patient.special_note, "hello world! jack!");
	g_patient.gender = NXUI_GENDER_FEMALE;
	g_patient.age = 38;
	g_patient.bed_no = 22;
	g_patient.care_level = NXUI_LEVEL_ONE;
	set_bkgd_image();
	frame_0_init();
}

