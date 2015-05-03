/****************************************************************************
 * examples/nxui/nxui.h
 ****************************************************************************/

#ifndef __APPS_EXAMPLES_NXUI_NXUI_H
#define __APPS_EXAMPLES_NXUI_NXUI_H

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <debug.h>
#include <unistd.h>
#include <fixedmath.h>

#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxfonts.h>
#include <nuttx/video/rgbcolors.h>

#include <pthread.h>



/* outer configuration ******************************************************/
#ifndef CONFIG_NX
#  error "NX is not enabled (CONFIG_NX)"
#endif

#ifndef CONFIG_EXAMPLES_NXUI_VPLANE
#    define CONFIG_EXAMPLES_NXUI_VPLANE 0
#endif

#ifndef CONFIG_EXAMPLES_NXUI_BPP
#  define CONFIG_EXAMPLES_NXUI_BPP 32
#endif

#ifndef CONFIG_EXAMPLES_NXUI_BGCOLOR
#  if CONFIG_EXAMPLES_NXUI_BPP == 24 || CONFIG_EXAMPLES_NXUI_BPP == 32
#    define CONFIG_EXAMPLES_NXUI_BGCOLOR 0x8ee000//0x808000//0xC4BFF6//RGB24_SILVER//0x007b68ee
#  elif CONFIG_EXAMPLES_NXUI_BPP == 16
#    define CONFIG_EXAMPLES_NXUI_BGCOLOR 0x7b5d
#  elif CONFIG_EXAMPLES_NXUI_BPP < 8
#    define CONFIG_EXAMPLES_NXUI_BGCOLOR 0x00
#  else
#    define CONFIG_EXAMPLES_NXUI_BGCOLOR ' '
# endif
#endif

/* font configuration */
#ifndef CONFIG_EXAMPLES_NXUI_FONTID
#  define CONFIG_EXAMPLES_NXUI_FONTID NXFONT_DEFAULT
#endif

#ifndef CONFIG_EXAMPLES_NXUI_FONTCOLOR
#  if CONFIG_EXAMPLES_NXUI_BPP == 24 || CONFIG_EXAMPLES_NXUI_BPP == 32
#    define CONFIG_EXAMPLES_NXUI_FONTCOLOR 0x00000000
#  elif CONFIG_EXAMPLES_NXUI_BPP == 16
#    define CONFIG_EXAMPLES_NXUI_FONTCOLOR 0x0000
#  elif CONFIG_EXAMPLES_NXUI_BPP < 1
#    define CONFIG_EXAMPLES_NXUI_FONTCOLOR 0x01
#  else
#    define CONFIG_EXAMPLES_NXUI_FONTCOLOR 'F'
#  endif
#endif
/* end of font configuration */

/* image configuration */
#if defined(CONFIG_EXAMPLES_NXIMAGE_XSCALEp5)
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p5
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE2p0
#elif defined(CONFIG_EXAMPLES_NXIMAGE_XSCALE1p5)
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALEp5
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE2p0
#elif defined(CONFIG_EXAMPLES_NXIMAGE_XSCALE2p0)
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALEp5
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p5
#else
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALEp5
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE1p5
#  undef CONFIG_EXAMPLES_NXIMAGE_XSCALE2p0
#  define CONFIG_EXAMPLES_NXIMAGE_XSCALE1p0 1
#endif

#if defined(CONFIG_EXAMPLES_NXIMAGE_YSCALEp5)
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p5
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE2p0
#elif defined(CONFIG_EXAMPLES_NXIMAGE_YSCALE1p5)
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALEp5
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE2p0
#elif defined(CONFIG_EXAMPLES_NXIMAGE_YSCALE2p0)
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALEp5
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p5
#else
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALEp5
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p0
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE1p5
#  undef CONFIG_EXAMPLES_NXIMAGE_YSCALE2p0
#  define CONFIG_EXAMPLES_NXIMAGE_YSCALE1p0 1
#endif
/* end of image configuration */

/* internal configuration ***************************************************/

#define NXUI_NAME_LEN 8
#define NXUI_HOSPITALIZED_NO_LEN 32
#define NXUI_DIAGNOSIS_INFO_LEN 32
#define NXUI_PATIENT_POSITION_LEN 8
#define NXUI_DRUG_ALLERGY_LEN 8
#define NXUI_DIET_LEN 8
#define NXUI_MEDICARE_NO_LEN 32
#define NXUI_ID_LEN 32
#define NXUI_PHONE_LEN 13
#define NXUI_SPECIAL_NOTE_LEN 128

#define NXUI_DESCRIPTION_LEN 128

#define NXUI_FRAME_NUM 20
/****************************************************************************
 * Public Types
 ****************************************************************************/
enum exitcode_e
{
  NXEXIT_SUCCESS = 0,
  NXEXIT_EXTINITIALIZE,
  NXEXIT_FBINITIALIZE,
  NXEXIT_FBGETVPLANE,
  NXEXIT_LCDINITIALIZE,
  NXEXIT_LCDGETDEV,
  NXEXIT_NXOPEN,
  NXEXIT_FONTOPEN,
  NXEXIT_NXREQUESTBKGD,
  NXEXIT_NXSETBGCOLOR
};


struct nxui_data_s
{
  /* The NX handles */

  NXHANDLE hnx;
  NXHANDLE hbkgd;
  //NXHANDLE hfont;

  /* The screen resolution */

  nxgl_coord_t xres;
  nxgl_coord_t yres;

  volatile bool havepos;
  sem_t sem;
  volatile int code;
};

struct nxui_frame_s {
	struct nxui_textrect_s *text_rect;
	struct nxui_imagerect_s *image_rect;
};
/*Describes one patient*/

enum gender_e {
	NXUI_GENDER_MALE,
	NXUI_GENDER_FEMALE
};

struct time_s {
	uint16_t year;
	uint8_t month;
	uint8_t day;
};

enum level_e {
	NXUI_LEVEL_ONE,										 /* 一级护理 */
	NXUI_LEVEL_TWO,									     /* 二级护理 */
	NXUI_LEVEL_THREE								     /* 三级护理 */
};

struct nxui_patient_s {
	char name[NXUI_NAME_LEN + 1];                        /* 姓名 */
	enum gender_e gender;						         /* 性别 */
	uint16_t age;								         /* 年龄 */
	uint16_t bed_no;							         /* 床位号 */
	struct time_s admmission_time;                       /* 入院时间 */
	char hospitalized_no[NXUI_HOSPITALIZED_NO_LEN + 1];  /* 住院号码 */
	char diagnosis_info[NXUI_DIAGNOSIS_INFO_LEN + 1];    /* 住院信息 */
    enum level_e care_level;							 /* 护理级别 */
	char position[NXUI_PATIENT_POSITION_LEN + 1];        /* 病人体位 */
	char drug_allergy[NXUI_DRUG_ALLERGY_LEN + 1];        /* 药物过敏信息 */
	char diet[NXUI_DIET_LEN + 1];					     /* 饮食 */
	char medicare_no[NXUI_MEDICARE_NO_LEN + 1];			 /* 医保号 */
	char ID[NXUI_ID_LEN + 1];							 /* 身份证号 */
	char contact_phone[NXUI_PHONE_LEN + 1];				 /* 联系电话 */
	char special_note[NXUI_SPECIAL_NOTE_LEN + 1];        /* 特别提醒 */
};

/* Describes one doctor */

struct nxui_doctor_s {
	char name[NXUI_NAME_LEN + 1];						 /* 姓名 */
	char desc[NXUI_DESCRIPTION_LEN + 1];				 /* 描述 */
	FAR uint8_t *image;									 /* 照片 */
};

/* Frame display *************************************************************/

enum rect_type_e {
	NXUI_PATIENT_NAME,
	NXUI_PATIENT_GENDER,
	NXUI_PATIENT_AGE,
	NXUI_DOCTOR_PHOTO,
	NXUI_SPECIAL_NOTE,
	NXUI_PATIENT_DIET,
	NXUI_DRUG_ALLERGY,
	NXUI_PATIENT_POSITION,
	NXUI_CARE_LEVEL,
	NXUI_BED_NO
};

/* 用于显示图像 */
struct nxui_imagerect_s {
	enum rect_type_e rect_type;					         /* 矩形框标识，表明该矩形框的用途，例如显示病人姓名 */
	uint8_t x_offset;									 /* 显示内容区域离矩形框左上角的x方向的偏移 */
	uint8_t y_offset;									 /* 显示内容区域离矩形框左上角的y方向的偏移 */
	nxgl_mxpixel_t bkgd_color;							 /* 矩形区域的背景色 */
	nxgl_mxpixel_t line_color; 						     /* 矩形的边框颜色 */
	nxgl_coord_t line_width;					         /* 边框的宽度 */
	struct nxgl_rect_s rect;                             /* 指定矩形在屏幕上的位置 */
	struct nxui_imagerect_s *next;                       /* 指向下一个矩形结构 */

};

/* 用于显示文本 */
struct nxui_textrect_s {
	enum rect_type_e rect_type;
	uint8_t x_offset;
	uint8_t y_offset;
	nxgl_mxpixel_t bkgd_color;
	nxgl_mxpixel_t line_color;
	nxgl_coord_t line_width;
	struct nxgl_rect_s rect; 
	struct nxui_textrect_s *next;	
	enum nx_fontid_e font_id;
	uint8_t font_height;
	nxgl_mxpixel_t font_color;
	char *text;
	uint16_t col_num;                                    /* 每行能够显示的文本数，如果行数为1,那么超过此值则滚动显示，否则就换行显示，如果还显示不完则每次向上滚动一行显示 */
	uint16_t row_num;                                    /* 能够用来显示的行数 */
	uint16_t text_index;                                  /* 指向当前要显示的文本的开始位置，每次滚动其值加1 */
	pthread_t text_scroll_handle;                        /* 如果为NULL则不支持文本的滚动显示，否则指向滚动播放的线程 */
};

struct pix_run_s
{
  uint32_t npix;  /* Number of pixels */
  uint32_t code;  /* Pixel RGB code */
};

struct nxui_rgb_s {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
/****************************************************************************
 * Public Variables
 ****************************************************************************/
extern struct nxui_frame_s g_frame[NXUI_FRAME_NUM];
extern struct nxui_frame_s *g_current;
extern struct nxui_patient_s g_patient;
extern const struct pix_run_s g_nuttx[];
extern const struct pix_run_s bitmap[];
extern const struct nxui_rgb_s  palette[];
/* NXHELLO state data */

extern struct nxui_data_s g_nxui;

/* NX callback vtables */

extern const struct nx_callback_s g_nxuicb;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_EXAMPLES_NXHELLO_EXTERNINIT
extern FAR NX_DRIVERTYPE *up_nxdrvinit(unsigned int devno);
#endif

FAR void nxui_frame_init(void);
FAR int nxui_draw_frame(int which_frame);

/* Image interfaces */
extern void nximage_blitrow(FAR nxgl_mxpixel_t *run, FAR const void **state, nxgl_coord_t image_width);
#endif /* __APPS_EXAMPLES_NXUI_NXUI_H */
