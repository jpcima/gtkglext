/* GdkGLExt - OpenGL Extension to GDK
 * Copyright (C) 2002-2004  Naofumi Yasufuku
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

#include <string.h>

#define PANGO_ENABLE_ENGINE 1
#include <pango/pangoxft.h>

#include "gdkglx.h"
#include "gdkglprivate-x11.h"
#include "gdkglfont.h"

#ifdef GDKGLEXT_MULTIHEAD_SUPPORT
#include <gdk/gdkdisplay.h>
#endif /* GDKGLEXT_MULTIHEAD_SUPPORT */

/*
 * This code is ripped from the FOX toolkit.
 */
static void glXUseXftFont(XftFont* font,int first,int count,int listBase){
  GLint swapbytes,lsbfirst,rowlength,skiprows,skippixels,alignment,list;
  GLfloat x0,y0,dx,dy;
  FT_Face face;
  FT_Error err;
  int i,size,x,y;
  unsigned char *glyph;

  // Save the current packing mode for bitmaps
  glGetIntegerv(GL_UNPACK_SWAP_BYTES,&swapbytes);
  glGetIntegerv(GL_UNPACK_LSB_FIRST,&lsbfirst);
  glGetIntegerv(GL_UNPACK_ROW_LENGTH,&rowlength);
  glGetIntegerv(GL_UNPACK_SKIP_ROWS,&skiprows);
  glGetIntegerv(GL_UNPACK_SKIP_PIXELS,&skippixels);
  glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);

  // Set desired packing modes
  glPixelStorei(GL_UNPACK_SWAP_BYTES,GL_FALSE);
  glPixelStorei(GL_UNPACK_LSB_FIRST,GL_FALSE);
  glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  // Get face info
  face=XftLockFace(font);

  // Render font glyphs; use FreeType to render to bitmap
  for(i=first; i<count; i++){
    list=listBase+i;

    // Load glyph
    err=FT_Load_Glyph(face,FT_Get_Char_Index(face,i),FT_LOAD_DEFAULT);
    if(err){ g_warning("glXUseXftFont: unable to load glyph"); return; }

    // Render glyph
    err=FT_Render_Glyph(face->glyph,FT_RENDER_MODE_MONO);
    if(err){ g_warning("glXUseXftFont: unable to render glyph"); return; }

    // Pitch may be negative, its the stride between rows
    size=ABS(face->glyph->bitmap.pitch) * face->glyph->bitmap.rows;

    // Glyph coordinates; note info in freetype is 6-bit fixed point
    x0=-(face->glyph->metrics.horiBearingX>>6);
    y0=(face->glyph->metrics.height-face->glyph->metrics.horiBearingY)>>6;
    dx=face->glyph->metrics.horiAdvance>>6;
    dy=0;

    // Allocate glyph data
    glyph=g_malloc(size);

    // Copy into OpenGL bitmap format; note OpenGL upside down
    for(y=0; y<face->glyph->bitmap.rows; y++){
      for(x=0; x<face->glyph->bitmap.pitch; x++){
        glyph[y*face->glyph->bitmap.pitch+x]=face->glyph->bitmap.buffer[(face->glyph->bitmap.rows-y-1)*face->glyph->bitmap.pitch+x];
        }
      }

    // Put bitmap into display list
    glNewList(list,GL_COMPILE);
    glBitmap(ABS(face->glyph->bitmap.pitch)<<3,face->glyph->bitmap.rows,x0,y0,dx,dy,glyph);
    glEndList();

    // Free glyph data
    g_free(glyph);
    }

  // Restore packing modes
  glPixelStorei(GL_UNPACK_SWAP_BYTES,swapbytes);
  glPixelStorei(GL_UNPACK_LSB_FIRST,lsbfirst);
  glPixelStorei(GL_UNPACK_ROW_LENGTH,rowlength);
  glPixelStorei(GL_UNPACK_SKIP_ROWS,skiprows);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS,skippixels);
  glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);

  // Unlock face
  XftUnlockFace(font);
  }

static PangoFont *
gdk_gl_font_use_pango_font_common (PangoFontMap               *font_map,
                                   const PangoFontDescription *font_desc,
                                   int                         first,
                                   int                         count,
                                   int                         list_base)
{
  PangoFont *font = NULL;
  XftFont *xfont = NULL;

  GDK_GL_NOTE_FUNC_PRIVATE ();

  font = pango_font_map_load_font (font_map, NULL, font_desc);
  if (font == NULL)
    {
      g_warning ("cannot load PangoFont");
      goto FAIL;
    }


  xfont = pango_xft_font_get_font(font);
  if (xfont == NULL)
    {
      g_warning ("cannot load XftFont");
      goto FAIL;
    }

  glXUseXftFont (xfont, first, count, list_base);

 FAIL:

  return font;
}

/**
 * gdk_gl_font_use_pango_font:
 * @font_desc: a #PangoFontDescription describing the font to use.
 * @first: the index of the first glyph to be taken.
 * @count: the number of glyphs to be taken.
 * @list_base: the index of the first display list to be generated.
 *
 * Creates bitmap display lists from a #PangoFont.
 *
 * Return value: the #PangoFont used, or NULL if no font matched.
 **/
PangoFont *
gdk_gl_font_use_pango_font (const PangoFontDescription *font_desc,
                            int                         first,
                            int                         count,
                            int                         list_base)
{
  PangoFontMap *font_map;

  g_return_val_if_fail (font_desc != NULL, NULL);

  GDK_GL_NOTE_FUNC ();

  font_map = pango_xft_get_font_map (gdk_x11_get_default_xdisplay (),
                                     gdk_x11_get_default_screen ());

  return gdk_gl_font_use_pango_font_common (font_map, font_desc,
                                            first, count, list_base);
}

#ifdef GDKGLEXT_MULTIHEAD_SUPPORT

/**
 * gdk_gl_font_use_pango_font_for_display:
 * @display: a #GdkDisplay.
 * @font_desc: a #PangoFontDescription describing the font to use.
 * @first: the index of the first glyph to be taken.
 * @count: the number of glyphs to be taken.
 * @list_base: the index of the first display list to be generated.
 *
 * Creates bitmap display lists from a #PangoFont.
 *
 * Return value: the #PangoFont used, or NULL if no font matched.
 **/
PangoFont *
gdk_gl_font_use_pango_font_for_display (GdkDisplay                 *display,
                                        const PangoFontDescription *font_desc,
                                        int                         first,
                                        int                         count,
                                        int                         list_base)
{
  PangoFontMap *font_map;

  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
  g_return_val_if_fail (font_desc != NULL, NULL);

  GDK_GL_NOTE_FUNC ();

  font_map = pango_xft_get_font_map (gdk_x11_display_get_xdisplay (display),
                                     gdk_x11_screen_get_screen_number (gdk_display_get_default_screen (display)));

  return gdk_gl_font_use_pango_font_common (font_map, font_desc,
                                            first, count, list_base);
}

#endif /* GDKGLEXT_MULTIHEAD_SUPPORT */
