/****************************************************************************
 * Copyright (C) 2015 Intel Corporation.   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 ***************************************************************************/

#include "pipe/p_screen.h"
#include "pipe/p_defines.h"
#include "util/u_memory.h"
#include "util/u_format.h"
#include "util/u_inlines.h"
#include "util/u_cpu_detect.h"

#include "state_tracker/sw_winsys.h"

extern "C" {
#include "gallivm/lp_bld_limits.h"
}

#include "swr_public.h"
#include "swr_screen.h"
#include "swr_context.h"
#include "swr_resource.h"
#include "swr_fence.h"
#include "gen_knobs.h"

#include "jit_api.h"

#include <stdio.h>

static const char *
swr_get_name(struct pipe_screen *screen)
{
   return "SWR";
}

static const char *
swr_get_vendor(struct pipe_screen *screen)
{
   return "Intel Corporation";
}

static boolean
swr_is_format_supported(struct pipe_screen *screen,
                        enum pipe_format format,
                        enum pipe_texture_target target,
                        unsigned sample_count,
                        unsigned bind)
{
   struct sw_winsys *winsys = swr_screen(screen)->winsys;
   const struct util_format_description *format_desc;

   assert(target == PIPE_BUFFER || target == PIPE_TEXTURE_1D
          || target == PIPE_TEXTURE_1D_ARRAY
          || target == PIPE_TEXTURE_2D
          || target == PIPE_TEXTURE_2D_ARRAY
          || target == PIPE_TEXTURE_RECT
          || target == PIPE_TEXTURE_3D
          || target == PIPE_TEXTURE_CUBE
          || target == PIPE_TEXTURE_CUBE_ARRAY);

   format_desc = util_format_description(format);
   if (!format_desc)
      return FALSE;

   if (sample_count > 1)
      return FALSE;

   if (bind
       & (PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT | PIPE_BIND_SHARED)) {
      if (!winsys->is_displaytarget_format_supported(winsys, bind, format))
         return FALSE;
   }

   if (bind & PIPE_BIND_RENDER_TARGET) {
      if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS)
         return FALSE;

      if (mesa_to_swr_format(format) == (SWR_FORMAT)-1)
         return FALSE;

      /*
       * Although possible, it is unnatural to render into compressed or YUV
       * surfaces. So disable these here to avoid going into weird paths
       * inside the state trackers.
       */
      if (format_desc->block.width != 1 || format_desc->block.height != 1)
         return FALSE;
   }

   /* We're going to lie and say we support all depth/stencil formats.
    * SWR actually needs separate bindings, and only does F32 depth.
    */
   if (bind & PIPE_BIND_DEPTH_STENCIL) {
      if (format_desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS)
         return FALSE;
   }

   return TRUE;
}

static int
swr_get_param(struct pipe_screen *screen, enum pipe_cap param)
{
   switch (param) {
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
      return 1;
   case PIPE_CAP_TWO_SIDED_STENCIL:
      return 1;
   case PIPE_CAP_SM3:
      return 1;
   case PIPE_CAP_ANISOTROPIC_FILTER:
      return 0;
   case PIPE_CAP_POINT_SPRITE:
      return 1;
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return PIPE_MAX_COLOR_BUFS;
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      return 1;
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
      return 1;
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP:
      return 1;
   case PIPE_CAP_TEXTURE_SHADOW_MAP:
      return 1;
   case PIPE_CAP_TEXTURE_SWIZZLE:
      return 1;
   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK:
      return 0;
   case PIPE_CAP_MAX_TEXTURE_2D_LEVELS:
      return 13; // xxx This increases rendertarget max size to 4k x 4k.  No
                 // way to separate widht/height.
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return 12; // xxx
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return 12; // xxx
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
      return 1;
   case PIPE_CAP_INDEP_BLEND_ENABLE:
      return 1;
   case PIPE_CAP_INDEP_BLEND_FUNC:
      return 1;
   case PIPE_CAP_TGSI_FS_COORD_ORIGIN_LOWER_LEFT:
      return 0; // Don't support lower left frag coord.
   case PIPE_CAP_TGSI_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_TGSI_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
   case PIPE_CAP_TGSI_FS_COORD_PIXEL_CENTER_INTEGER:
      return 1;
   case PIPE_CAP_DEPTH_CLIP_DISABLE:
      return 1;
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return MAX_SO_STREAMS;
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return MAX_ATTRIBUTES;
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return 1024;
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return 1;
   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;
   case PIPE_CAP_PRIMITIVE_RESTART:
      return 1;
   case PIPE_CAP_SHADER_STENCIL_EXPORT:
      return 1;
   case PIPE_CAP_TGSI_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_START_INSTANCE:
      return 1;
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
      return 1;
   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return 256; /* for GL3 */
   case PIPE_CAP_MIN_TEXEL_OFFSET:
      return -8;
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return 7;
   case PIPE_CAP_CONDITIONAL_RENDER:
      return 1;
   case PIPE_CAP_TEXTURE_BARRIER:
      return 0;
   case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED: /* draw module */
   case PIPE_CAP_VERTEX_COLOR_CLAMPED: /* draw module */
      return 1;
   case PIPE_CAP_MIXED_COLORBUFFER_FORMATS:
      return 0;
   case PIPE_CAP_GLSL_FEATURE_LEVEL:
      return 330;
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
      return 0;
   case PIPE_CAP_COMPUTE:
      return 0;
   case PIPE_CAP_USER_VERTEX_BUFFERS:
   case PIPE_CAP_USER_INDEX_BUFFERS:
   case PIPE_CAP_USER_CONSTANT_BUFFERS:
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_TGSI_VS_LAYER_VIEWPORT:
      return 1;
   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 16;
   case PIPE_CAP_TGSI_CAN_COMPACT_CONSTANTS:
   case PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
      return 0;
   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return 64;
   case PIPE_CAP_QUERY_TIMESTAMP:
      return 1;
   case PIPE_CAP_CUBE_MAP_ARRAY:
      return 0;
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
      return 1;
   case PIPE_CAP_MAX_TEXTURE_BUFFER_SIZE:
      return 65536;
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      return 0;
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_PREFER_BLIT_BASED_TEXTURE_TRANSFER:
      return 0;
   case PIPE_CAP_MAX_VIEWPORTS:
      return 1;
   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_NATIVE;
   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
   case PIPE_CAP_TEXTURE_GATHER_SM5:
      return 0;
   case PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT:
      return 1;
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_TEXTURE_GATHER_OFFSETS:
   case PIPE_CAP_TGSI_VS_WINDOW_SPACE_POSITION:
   case PIPE_CAP_TGSI_FS_FINE_DERIVATIVE:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
      return 0;
   case PIPE_CAP_FAKE_SW_MSAA:
      return 1;
   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
      return 0;
   case PIPE_CAP_DRAW_INDIRECT:
      return 1;

   case PIPE_CAP_VENDOR_ID:
      return 0xFFFFFFFF;
   case PIPE_CAP_DEVICE_ID:
      return 0xFFFFFFFF;
   case PIPE_CAP_ACCELERATED:
      return 0;
   case PIPE_CAP_VIDEO_MEMORY: {
      /* XXX: Do we want to return the full amount of system memory ? */
      uint64_t system_memory;

      if (!os_get_total_physical_memory(&system_memory))
         return 0;

      return (int)(system_memory >> 20);
   }
   case PIPE_CAP_UMA:
      return 1;
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
      return 1;
   case PIPE_CAP_CLIP_HALFZ:
      return 1;
   case PIPE_CAP_VERTEXID_NOBASE:
      return 0;
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
      return 1;
   case PIPE_CAP_MULTISAMPLE_Z_RESOLVE:
      return 0;
   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
      return 0; // xxx
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
      return 0;
   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      return 0;
   case PIPE_CAP_DEPTH_BOUNDS_TEST:
      return 0; // xxx
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
      return 1;
   }

   /* should only get here on unhandled cases */
   debug_printf("Unexpected PIPE_CAP %d query\n", param);
   return 0;
}

static int
swr_get_shader_param(struct pipe_screen *screen,
                     unsigned shader,
                     enum pipe_shader_cap param)
{
   if (shader == PIPE_SHADER_VERTEX || shader == PIPE_SHADER_FRAGMENT)
      return gallivm_get_shader_param(param);

   // Todo: geometry, tesselation, compute
   return 0;
}


static float
swr_get_paramf(struct pipe_screen *screen, enum pipe_capf param)
{
   switch (param) {
   case PIPE_CAPF_MAX_LINE_WIDTH:
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
   case PIPE_CAPF_MAX_POINT_WIDTH:
      return 255.0; /* arbitrary */
   case PIPE_CAPF_MAX_POINT_WIDTH_AA:
      return 0.0;
   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 0.0;
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 0.0;
   case PIPE_CAPF_GUARD_BAND_LEFT:
   case PIPE_CAPF_GUARD_BAND_TOP:
   case PIPE_CAPF_GUARD_BAND_RIGHT:
   case PIPE_CAPF_GUARD_BAND_BOTTOM:
      return 0.0;
   }
   /* should only get here on unhandled cases */
   debug_printf("Unexpected PIPE_CAPF %d query\n", param);
   return 0.0;
}

SWR_FORMAT
mesa_to_swr_format(enum pipe_format format)
{
   const struct util_format_description *format_desc =
      util_format_description(format);
   if (!format_desc)
      return (SWR_FORMAT)-1;

   // more robust check would be comparing all attributes of the formats
   // luckily format names are mostly standardized
   for (int i = 0; i < NUM_SWR_FORMATS; i++) {
      const SWR_FORMAT_INFO &swr_desc = GetFormatInfo((SWR_FORMAT)i);

      if (!strcasecmp(format_desc->short_name, swr_desc.name))
         return (SWR_FORMAT)i;
   }

   // ... with some exceptions
   switch (format) {
   case PIPE_FORMAT_R8G8B8A8_SRGB:
      return R8G8B8A8_UNORM_SRGB;
   case PIPE_FORMAT_B8G8R8A8_SRGB:
      return B8G8R8A8_UNORM_SRGB;
   case PIPE_FORMAT_I8_UNORM:
      return R8_UNORM;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      return R24_UNORM_X8_TYPELESS;
   case PIPE_FORMAT_L8A8_UNORM:
      return R8G8_UNORM;
   default:
      break;
   }

   debug_printf("asked to convert unsupported format %s\n",
                format_desc->name);
   return (SWR_FORMAT)-1;
}

static boolean
swr_displaytarget_layout(struct swr_screen *screen, struct swr_resource *res)
{
   struct sw_winsys *winsys = screen->winsys;

   UINT stride;
   res->display_target = winsys->displaytarget_create(winsys,
                                                      res->base.bind,
                                                      res->base.format,
                                                      res->alignedWidth,
                                                      res->alignedHeight,
                                                      64,
                                                      &stride);

   if (res->display_target == NULL)
      return FALSE;

   /* Clear the display target surface */
   void *map = winsys->displaytarget_map(
      winsys, res->display_target, PIPE_TRANSFER_WRITE);

   if (map)
      memset(map, 0, res->alignedHeight * stride);

   winsys->displaytarget_unmap(winsys, res->display_target);

   return TRUE;
}

static struct pipe_resource *
swr_resource_create(struct pipe_screen *_screen,
                    const struct pipe_resource *templat)
{
   struct swr_screen *screen = swr_screen(_screen);
   struct swr_resource *res = CALLOC_STRUCT(swr_resource);
   if (!res)
      return NULL;

   res->base = *templat;
   pipe_reference_init(&res->base.reference, 1);
   res->base.screen = &screen->base;

   const struct util_format_description *desc =
      util_format_description(templat->format);
   res->has_depth = util_format_has_depth(desc);
   res->has_stencil = util_format_has_stencil(desc);

   pipe_format fmt = templat->format;
   if (res->has_depth)
      fmt = PIPE_FORMAT_Z24_UNORM_S8_UINT;
   if (res->has_stencil && !res->has_depth)
      fmt = PIPE_FORMAT_R8_UINT;

   res->swr.width = templat->width0;
   res->swr.height = templat->height0;
   res->swr.depth = templat->depth0;
   res->swr.type = SURFACE_2D;
   res->swr.tileMode = SWR_TILE_NONE;
   res->swr.format = mesa_to_swr_format(fmt);
   res->swr.numSamples = (1 << templat->nr_samples);

   SWR_FORMAT_INFO finfo = GetFormatInfo(res->swr.format);

   unsigned total_size = 0;
   unsigned width = templat->width0;
   unsigned height = templat->height0;
   unsigned depth = templat->depth0;
   unsigned layers = templat->array_size;

   for (int level = 0; level <= templat->last_level; level++) {
      unsigned alignedWidth, alignedHeight;
      unsigned num_slices;

      if (templat->bind & (PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_RENDER_TARGET
                           | PIPE_BIND_DISPLAY_TARGET)) {
         alignedWidth = (width + (KNOB_MACROTILE_X_DIM - 1))
            & ~(KNOB_MACROTILE_X_DIM - 1);
         alignedHeight = (height + (KNOB_MACROTILE_Y_DIM - 1))
            & ~(KNOB_MACROTILE_Y_DIM - 1);
      } else {
         alignedWidth = width;
         alignedHeight = height;
      }

      if (level == 0) {
         res->alignedWidth = alignedWidth;
         res->alignedHeight = alignedHeight;
      }

      res->row_stride[level] = alignedWidth * finfo.Bpp;
      res->img_stride[level] = res->row_stride[level] * alignedHeight;
      res->mip_offsets[level] = total_size;

      if (templat->target == PIPE_TEXTURE_3D)
         num_slices = depth;
      else if (templat->target == PIPE_TEXTURE_1D_ARRAY
               || templat->target == PIPE_TEXTURE_2D_ARRAY
               || templat->target == PIPE_TEXTURE_CUBE
               || templat->target == PIPE_TEXTURE_CUBE_ARRAY)
         num_slices = layers;
      else
         num_slices = 1;

      total_size += res->img_stride[level] * num_slices;

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      depth = u_minify(depth, 1);
   }

   res->swr.halign = res->alignedWidth;
   res->swr.valign = res->alignedHeight;
   res->swr.pitch = res->row_stride[0];
   res->swr.pBaseAddress = (BYTE *)_aligned_malloc(total_size, 64);

   if (res->has_depth && res->has_stencil) {
      res->secondary.width = templat->width0;
      res->secondary.height = templat->height0;
      res->secondary.depth = templat->depth0;
      res->secondary.type = SURFACE_2D;
      res->secondary.tileMode = SWR_TILE_NONE;
      res->secondary.format = R8_UINT;
      res->secondary.numSamples = (1 << templat->nr_samples);

      SWR_FORMAT_INFO finfo = GetFormatInfo(res->secondary.format);
      res->secondary.pitch = res->alignedWidth * finfo.Bpp;
      res->secondary.pBaseAddress = (BYTE *)_aligned_malloc(
         res->alignedHeight * res->secondary.pitch, 64);
   }

   if (swr_resource_is_texture(&res->base)) {
      if (res->base.bind & (PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT
                            | PIPE_BIND_SHARED)) {
         /* displayable surface */
         if (!swr_displaytarget_layout(screen, res))
            goto fail;
      }
   }

   return &res->base;

fail:
   FREE(res);
   return NULL;
}

static void
swr_resource_destroy(struct pipe_screen *p_screen, struct pipe_resource *pt)
{
   struct swr_screen *screen = swr_screen(p_screen);
   struct swr_resource *res = swr_resource(pt);

   /*
    * If this resource is attached to a context it may still be in use, check
    * dependencies before freeing
    * XXX TODO: don't use SwrWaitForIdle, use fences and come up with a real
    * resource manager.
    * XXX It's happened that we get a swr_destroy prior to freeing the
    * framebuffer resource.  Don't wait on it.
    */
   if (res->bound_to_context && !res->display_target) {
      struct swr_context *ctx =
         swr_context((pipe_context *)res->bound_to_context);
      SwrWaitForIdle(
         ctx->swrContext); // BMCDEBUG, don't SwrWaitForIdle!!! Use a fence.
   }

   if (res->display_target) {
      /* display target */
      struct sw_winsys *winsys = screen->winsys;
      winsys->displaytarget_destroy(winsys, res->display_target);
   }

   _aligned_free(res->swr.pBaseAddress);
   _aligned_free(res->secondary.pBaseAddress);

   FREE(res);
}


static void
swr_flush_frontbuffer(struct pipe_screen *p_screen,
                      struct pipe_resource *resource,
                      unsigned level,
                      unsigned layer,
                      void *context_private,
                      struct pipe_box *sub_box)
{
   SWR_SURFACE_STATE &colorBuffer = swr_resource(resource)->swr;

   struct swr_screen *screen = swr_screen(p_screen);
   struct sw_winsys *winsys = screen->winsys;
   struct swr_resource *res = swr_resource(resource);

   /* Ensure fence set at flush is finished, before reading frame buffer */
   swr_fence_finish(p_screen, screen->flush_fence, 0);

   void *map = winsys->displaytarget_map(
      winsys, res->display_target, PIPE_TRANSFER_WRITE);
   memcpy(
      map, colorBuffer.pBaseAddress, colorBuffer.pitch * colorBuffer.height);
   winsys->displaytarget_unmap(winsys, res->display_target);

   assert(res->display_target);
   if (res->display_target)
      winsys->displaytarget_display(
         winsys, res->display_target, context_private, sub_box);
}


static void
swr_destroy_screen(struct pipe_screen *p_screen)
{
   struct swr_screen *screen = swr_screen(p_screen);
   struct sw_winsys *winsys = screen->winsys;

   fprintf(stderr, "SWR destroy screen!\n");

   swr_fence_finish(p_screen, screen->flush_fence, 0);
   swr_fence_reference(p_screen, &screen->flush_fence, NULL);

   JitDestroyContext(screen->hJitMgr);

   if (winsys->destroy)
      winsys->destroy(winsys);

   FREE(screen);
}


struct pipe_screen *
swr_create_screen(struct sw_winsys *winsys)
{
   struct swr_screen *screen = CALLOC_STRUCT(swr_screen);

   if (!screen)
      return NULL;

   fprintf(stderr, "SWR create screen!\n");
   util_cpu_detect();
   if (util_cpu_caps.has_avx2)
      fprintf(stderr, "This processor supports AVX2.\n");
   else if (util_cpu_caps.has_avx)
      fprintf(stderr, "This processor supports AVX.\n");
   /* Exit gracefully if there is no AVX support */
   else {
      fprintf(stderr, " !!! This processor does not support AVX or AVX2.  "
                      "OpenSWR requires AVX.\n");
      exit(-1);
   }

   if (!getenv("KNOB_MAX_PRIMS_PER_DRAW")) {
      g_GlobalKnobs.MAX_PRIMS_PER_DRAW.Value(49152);
   }

   screen->winsys = winsys;
   screen->base.get_name = swr_get_name;
   screen->base.get_vendor = swr_get_vendor;
   screen->base.is_format_supported = swr_is_format_supported;
   screen->base.context_create = swr_create_context;

   screen->base.destroy = swr_destroy_screen;
   screen->base.get_param = swr_get_param;
   screen->base.get_shader_param = swr_get_shader_param;
   screen->base.get_paramf = swr_get_paramf;

   screen->base.resource_create = swr_resource_create;
   screen->base.resource_destroy = swr_resource_destroy;

   screen->base.flush_frontbuffer = swr_flush_frontbuffer;

   screen->hJitMgr = JitCreateContext(KNOB_SIMD_WIDTH, KNOB_ARCH_STR);

   swr_fence_init(&screen->base);

   return &screen->base;
}
