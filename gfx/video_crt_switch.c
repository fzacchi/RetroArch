/* LCD SwitchRes Core
 * Copyright (C) 2018 Alphanu / Ben Templeman.
 * Copyright (C) 2019 Francesco Zacchi.
 * 
 * RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "video_driver.h"
#include "video_crt_switch.h"
#include "video_display_server.h"

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#if defined(HAVE_VIDEOCORE)
#include "include/userland/interface/vmcs_host/vc_vchi_gencmd.h"
static void crt_rpi_switch(int width, int height, float hz);
#endif

static unsigned ra_core_width     = 0;
static unsigned ra_core_height    = 0;
static unsigned ra_tmp_width      = 0;
static unsigned ra_tmp_height     = 0;
static unsigned ra_set_core_hz    = 0;
static unsigned orig_width        = 0;
static unsigned orig_height       = 0;
static int crt_center_adjust      = 0;
static int crt_tmp_center_adjust  = 0;
static double p_clock             = 0;

static bool first_run             = true;

static float ra_tmp_core_hz       = 0.0f;
static float fly_aspect           = 0.0f;
static float ra_core_hz           = 0.0f;
static unsigned crt_index         = 0;

static void crt_check_first_run(void)
{
   if (!first_run)
      return;

   first_run   = false;
}

static void switch_crt_hz(void)
{
   if (ra_core_hz == ra_tmp_core_hz)
      return;
   /* convert to integer */
   if (ra_core_hz < 50) ra_set_core_hz = 50;
   
   if (ra_core_hz > 63) ra_set_core_hz = 120;
   
   if (50 < ra_core_hz < 63) ra_set_core_hz = round(ra_core_hz);

   video_monitor_set_refresh_rate(ra_set_core_hz);

   ra_tmp_core_hz = ra_core_hz;
}

void crt_aspect_ratio_switch(unsigned width, unsigned height)
{
   /* send aspect float to videeo_driver */
   fly_aspect = (float)width / height;
   video_driver_set_aspect_ratio_value((float)fly_aspect);
}

static void switch_res_crt(unsigned width, unsigned height)
{
   video_display_server_set_resolution(width, height,
         ra_set_core_hz, ra_core_hz, crt_center_adjust, crt_index, crt_center_adjust);
#if defined(HAVE_VIDEOCORE)
   crt_rpi_switch(width, height, ra_core_hz);
   video_monitor_set_refresh_rate(ra_core_hz);
   crt_switch_driver_reinit();
#endif
   video_driver_apply_state_changes();
}

/* Create correct aspect to fit video if resolution does not exist */
static void crt_screen_setup_aspect(unsigned width, unsigned height)
{
#if defined(HAVE_VIDEOCORE)
   if (height > 300)
      height = height/2;
#endif

   switch_crt_hz();
   /* get original resolution of core */
   if (height == 4)
   {
      /* detect menu only */
      if (width < 700)
         width = 320;

      height = 240;

      crt_aspect_ratio_switch(width, height);
   }

   if (height < 200 && height != 144)
   {
      crt_aspect_ratio_switch(width, height);
      height = 200;
   }

   if (height > 200)
      crt_aspect_ratio_switch(width, height);

   if (height == 144 && ra_set_core_hz == 50)
   {
      height = 288;
      crt_aspect_ratio_switch(width, height);
   }

   if (height > 200 && height < 224)
   {
      crt_aspect_ratio_switch(width, height);
      height = 224;
   }

   if (height > 224 && height < 240)
   {
      crt_aspect_ratio_switch(width, height);
      height = 240;
   }

   if (height > 240 && height < 255)
   {
      crt_aspect_ratio_switch(width, height);
      height = 254;
   }

   if (height == 528 && ra_set_core_hz == 60)
   {
      crt_aspect_ratio_switch(width, height);
      height = 480;
   }

   if (height >= 240 && height < 255 && ra_set_core_hz == 55)
   {
      crt_aspect_ratio_switch(width, height);
      height = 254;
   }

   switch_res_crt(width, height);
}

void crt_switch_res_core(unsigned width, unsigned height,
      float hz, unsigned crt_mode,
      int crt_switch_center_adjust, int monitor_index, bool dynamic)
{
   /* ra_core_hz float passed from within
    * void video_driver_monitor_adjust_system_rates(void) */
   if (width == 4 )
   {
      width = 320;
      height = 240;
   }

   ra_core_height = height;
   ra_core_hz     = hz;

   if (dynamic == true)
      ra_core_width = crt_compute_dynamic_width(width);
   else 
      ra_core_width  = width;

   crt_center_adjust = crt_switch_center_adjust;
   crt_index  = monitor_index;

   if (crt_mode == 2)
   {
      if (hz > 53)
         ra_core_hz = hz * 2;

      if (hz <= 53)
         ra_core_hz = 120.0f;
   }

   crt_check_first_run();

   /* Detect resolution change and switch */
   if (
      (ra_tmp_height != ra_core_height) ||
      (ra_core_width != ra_tmp_width) || (crt_center_adjust != crt_tmp_center_adjust)
      )
      crt_screen_setup_aspect(ra_core_width, ra_core_height);

   ra_tmp_height  = ra_core_height;
   ra_tmp_width   = ra_core_width;
    crt_tmp_center_adjust = crt_center_adjust;

   /* Check if aspect is correct, if not change */
   if (video_driver_get_aspect_ratio() != fly_aspect)
   {
      video_driver_set_aspect_ratio_value((float)fly_aspect);
      video_driver_apply_state_changes();
   }
}

void crt_video_restore(void)
{
   if (first_run)
      return;

   first_run = true;
}

int crt_compute_dynamic_width(int width)
{
   unsigned i;
   int dynamic_width   = 0;
   unsigned min_height = 261;

#if defined(HAVE_VIDEOCORE)
   p_clock             = 32000000;
#else
   p_clock             = 15000000;
#endif

   for (i = 0; i < 10; i++)
   {
      dynamic_width = (width*1.5)*i;
      if ((dynamic_width * min_height * ra_core_hz) > p_clock)
         break;

   }
   return dynamic_width;
}

#if defined(HAVE_VIDEOCORE)
static void crt_rpi_switch(int width, int height, float hz)
{
   char buffer[1024];
   VCHI_INSTANCE_T vchi_instance;
   VCHI_CONNECTION_T *vchi_connection = NULL;
   static char output[250]             = {0};
   static char output1[250]            = {0};
   static char output2[250]            = {0};
   static char set_hdmi[250]           = {0};
   static char set_hdmi_timing[250]    = {0};
   int i              = 0;
   int hfp            = 0;
   int hsp            = 0;
   int hbp            = 0;
   int vfp            = 0;
   int vsp            = 0;
   int vbp            = 0;
   int hmax           = 0;
   int vmax           = 0;
   int pdefault       = 8;
   int pwidth         = 0;
   float roundw     = 0.0f;
   float roundh     = 0.0f;
   float pixel_clock  = 0;
   int ip_flag     = 0;

   /* set core refresh from hz */
   video_monitor_set_refresh_rate(hz);





   snprintf(set_hdmi_timing, sizeof(set_hdmi_timing),
         "hdmi_timings %d 1 %d %d %d %d 1 %d %d %d 0 0 0 %f %d %f 1 ",
         width, hfp, hsp, hbp, height, vfp,vsp, vbp,
         hz, ip_flag, pixel_clock);

   vcos_init();

   vchi_initialise(&vchi_instance);

   vchi_connect(NULL, 0, vchi_instance);

   vc_vchi_gencmd_init(vchi_instance, &vchi_connection, 1);

   vc_gencmd(buffer, sizeof(buffer), set_hdmi_timing);

   vc_gencmd_stop();

   vchi_disconnect(vchi_instance);

   snprintf(output1,  sizeof(output1),
         "tvservice -e \"DMT 87\" > /dev/null");
   system(output1);
   snprintf(output2,  sizeof(output1),
         "fbset -g %d %d %d %d 24 > /dev/null",
         width, height, width, height);
   system(output2);
}
#endif
