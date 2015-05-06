/*************************************************************************
    > File Name: nxui.c
    > Author: liujie
    > Mail: 1170381285@qq.com 
    > Created Time: Mon 20 Apr 2015 03:16:27 PM CST
 ************************************************************************/


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <debug.h>

#ifdef CONFIG_NX_LCDDRIVER
#  include <nuttx/lcd/lcd.h>
#else
#  include <nuttx/video/fb.h>
#endif

#include <nuttx/arch.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nxfonts.h>

#include "nxui.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/
/* If not specified, assume that the hardware supports one video plane */

#ifndef CONFIG_EXAMPLES_NXUI_VPLANE
#  define CONFIG_EXAMPLES_NXUI_VPLANE 0
#endif

/* If not specified, assume that the hardware supports one LCD device */

#ifndef CONFIG_EXAMPLES_NXUI_DEVNO
#  define CONFIG_EXAMPLES_NXUI_DEVNO 0
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void nxui_redraw(NXWINDOW hwnd, FAR const struct nxgl_rect_s *rect,
                        bool morem, FAR void *arg);
static void nxui_position(NXWINDOW hwnd, FAR const struct nxgl_size_s *size,
                          FAR const struct nxgl_point_s *pos,
                          FAR const struct nxgl_rect_s *bounds,
                          FAR void *arg);
#ifdef CONFIG_NX_XYINPUT
static void nxui_mousein(NXWINDOW hwnd, FAR const struct nxgl_point_s *pos,
                         uint8_t buttons, FAR void *arg);
#endif

#ifdef CONFIG_NX_KBD
static void nxui_kbdin(NXWINDOW hwnd, uint8_t nch, FAR const uint8_t *ch,
                       FAR void *arg);
#endif


/****************************************************************************
 * Public Data
 ****************************************************************************/

/* Background window call table */

const struct nx_callback_s g_nxuicb =
{
  nxui_redraw,   /* redraw */
  nxui_position  /* position */
#ifdef CONFIG_NX_XYINPUT
  , nxui_mousein /* mousein */
#endif
#ifdef CONFIG_NX_KBD
  , nxui_kbdin   /* my kbdin */
#endif
};

struct nxui_patient_s g_patient;

pthread_mutex_t g_patient_mutex;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxui_redraw
 ****************************************************************************/

static void nxui_redraw(NXWINDOW hwnd, FAR const struct nxgl_rect_s *rect,
                        bool more, FAR void *arg)
{
  gvdbg("hwnd=%p rect={(%d,%d),(%d,%d)} more=%s\n",
         hwnd, rect->pt1.x, rect->pt1.y, rect->pt2.x, rect->pt2.y,
         more ? "true" : "false");
}

/****************************************************************************
 * Name: nxui_position
 ****************************************************************************/

static void nxui_position(NXWINDOW hwnd, FAR const struct nxgl_size_s *size,
                          FAR const struct nxgl_point_s *pos,
                          FAR const struct nxgl_rect_s *bounds,
                          FAR void *arg)
{
  /* Report the position */

  gvdbg("hwnd=%p size=(%d,%d) pos=(%d,%d) bounds={(%d,%d),(%d,%d)}\n",
        hwnd, size->w, size->h, pos->x, pos->y,
        bounds->pt1.x, bounds->pt1.y, bounds->pt2.x, bounds->pt2.y);

  /* Have we picked off the window bounds yet? */

  if (!g_nxui.havepos)
    {
      /* Save the background window handle */

      g_nxui.hbkgd = hwnd;

      /* Save the window limits */

      g_nxui.xres = bounds->pt2.x + 1;
      g_nxui.yres = bounds->pt2.y + 1;

      g_nxui.havepos = true;
      sem_post(&g_nxui.sem);
      gvdbg("Have xres=%d yres=%d\n", g_nxui.xres, g_nxui.yres);
    }
}

/****************************************************************************
 * Name: nxui_mousein
 ****************************************************************************/

#ifdef CONFIG_NX_XYINPUT
static void nxui_mousein(NXWINDOW hwnd, FAR const struct nxgl_point_s *pos,
                         uint8_t buttons, FAR void *arg)
{
  printf("nxui_mousein: hwnd=%p pos=(%d,%d) button=%02x\n",
         hwnd,  pos->x, pos->y, buttons);
}
#endif

/****************************************************************************
 * Name: nxui_kbdin
 ****************************************************************************/

#ifdef CONFIG_NX_KBD
static void nxui_kbdin(NXWINDOW hwnd, uint8_t nch, FAR const uint8_t *ch,
                       FAR void *arg)
{
  gvdbg("hwnd=%p nch=%d\n", hwnd, nch);

   /* In this example, there is no keyboard so a keyboard event is not
    * expected.
    */

   printf("nxui_kbdin: Unexpected keyboard callback\n");
}
#endif
/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

struct nxui_data_s g_nxui =
{
  NULL,          /* hnx */
  NULL,          /* hbkgd */
  0,             /* xres */
  0,             /* yres */
  false,         /* havpos */
  { 0 },         /* sem */
  NXEXIT_SUCCESS /* exit code */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxui_initialize
 ****************************************************************************/

static int nxui_initialize(void)
{
  FAR NX_DRIVERTYPE *dev;

#if defined(CONFIG_EXAMPLES_NXUI_EXTERNINIT)
  /* Use external graphics driver initialization */

  printf("nxui_initialize: Initializing external graphics device\n");
  dev = up_nxdrvinit(CONFIG_EXAMPLES_NXUI_DEVNO);
  if (!dev)
    {
      printf("nxui_initialize: up_nxdrvinit failed, devno=%d\n",
             CONFIG_EXAMPLES_NXUI_DEVNO);
      g_nxui.code = NXEXIT_EXTINITIALIZE;
      goto errout;
    }

#elif defined(CONFIG_NX_LCDDRIVER)
  int ret;

  /* Initialize the LCD device */

  printf("nxui_initialize: Initializing LCD\n");
  ret = up_lcdinitialize();
  if (ret < 0)
    {
      printf("nxui_initialize: up_lcdinitialize failed: %d\n", -ret);
      g_nxui.code = NXEXIT_LCDINITIALIZE;
      goto errout;
    }

  /* Get the device instance */

  dev = up_lcdgetdev(CONFIG_EXAMPLES_NXUI_DEVNO);
  if (!dev)
    {
      printf("nxui_initialize: up_lcdgetdev failed, devno=%d\n", CONFIG_EXAMPLES_NXUI_DEVNO);
      g_nxui.code = NXEXIT_LCDGETDEV;
      goto errout;
    }

  /* Turn the LCD on at 75% power */

  (void)dev->setpower(dev, ((3*CONFIG_LCD_MAXPOWER + 3)/4));
#else
  int ret;

  /* Initialize the frame buffer device */

  printf("nxui_initialize: Initializing framebuffer\n");
  ret = up_fbinitialize();
  if (ret < 0)
    {
      printf("nxui_initialize: up_fbinitialize failed: %d\n", -ret);
      g_nxui.code = NXEXIT_FBINITIALIZE;
      goto errout;
    }

  dev = up_fbgetvplane(CONFIG_EXAMPLES_NXUI_VPLANE);
  if (!dev)
    {
      printf("nxui_initialize: up_fbgetvplane failed, vplane=%d\n", CONFIG_EXAMPLES_NXUI_VPLANE);
      g_nxui.code = NXEXIT_FBGETVPLANE;
      goto errout;
    }
#endif

  /* Then open NX */

  printf("nxui_initialize: Open NX\n");
  g_nxui.hnx = nx_open(dev);
  if (!g_nxui.hnx)
    {
      printf("nxui_initialize: nx_open failed: %d\n", errno);
      g_nxui.code = NXEXIT_NXOPEN;
      goto errout;
    }

 /* Set the background to the configured background color */

  printf("nxui_main: Set background color=%d\n",
         CONFIG_EXAMPLES_NXUI_BGCOLOR);

  nxgl_mxpixel_t color = CONFIG_EXAMPLES_NXUI_BGCOLOR;
  ret = nx_setbgcolor(g_nxui.hnx, &color);
  if (ret < 0)
    {
      printf("nxui_main: nx_setbgcolor failed: %d\n", errno);
      g_nxui.code = NXEXIT_NXSETBGCOLOR;
      goto errout_with_nx;
    }
 /* Get the background window */

  ret = nx_requestbkgd(g_nxui.hnx, &g_nxuicb, NULL);
  if (ret < 0)
    {
      printf("nxui_main: nx_setbgcolor failed: %d\n", errno);
      g_nxui.code = NXEXIT_NXREQUESTBKGD;
      goto errout_with_nx;
    }

  /* Wait until we have the screen resolution.  We'll have this immediately
   * unless we are dealing with the NX server.
   */

  while (!g_nxui.havepos)
    {
      (void)sem_wait(&g_nxui.sem);
    }
  printf("nxui_main: Screen resolution (%d,%d)\n", g_nxui.xres, g_nxui.yres);
  return NXEXIT_SUCCESS;

/* Close NX */
errout_with_nx:
  printf("nxui_main: Close NX\n");
  nx_close(g_nxui.hnx);
errout:
  return g_nxui.code;
}



#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int nxui_main(int argc, char *argv[])
#endif
{
	int ret;
	ret = nxui_initialize();
	if(ret != NXEXIT_SUCCESS) {
		printf("nxui_main: nxui initialize error!\n");
		return;
	}
//	pthread_mutex_init(&g_patient_mutex, NULL);
	nxui_frame_init();
//	nxui_draw_frame(0);
//	sleep(3);
	nxui_draw_frame(2);
//	sleep(3);
//	nxui_draw_frame(0);
}
