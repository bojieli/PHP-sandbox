/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_01.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Sterling Hughes <sterling@php.net>                           |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef _PHP_SWF_H
#define _PHP_SWF_H

#ifdef COMPILE_DL_SWF
#undef HAVE_SWF
#define HAVE_SWF 1
#endif

#if HAVE_SWF

extern zend_module_entry swf_module_entry;
#define swf_module_ptr &swf_module_entry

PHP_MINIT_FUNCTION(swf);
PHP_MINFO_FUNCTION(swf);
extern void php_swf_define(INTERNAL_FUNCTION_PARAMETERS, int opt);
extern void php_swf_fill_bitmap(INTERNAL_FUNCTION_PARAMETERS, int opt);
extern void php_swf_geo_same(INTERNAL_FUNCTION_PARAMETERS, int opt);
PHP_FUNCTION(swf_openfile);
PHP_FUNCTION(swf_closefile);
PHP_FUNCTION(swf_labelframe);
PHP_FUNCTION(swf_showframe);
PHP_FUNCTION(swf_setframe);
PHP_FUNCTION(swf_getframe);
PHP_FUNCTION(swf_mulcolor);
PHP_FUNCTION(swf_addcolor);
PHP_FUNCTION(swf_placeobject);
PHP_FUNCTION(swf_modifyobject);
PHP_FUNCTION(swf_removeobject);
PHP_FUNCTION(swf_nextid);
PHP_FUNCTION(swf_startdoaction);
PHP_FUNCTION(swf_enddoaction);
PHP_FUNCTION(swf_actiongotoframe);
PHP_FUNCTION(swf_actiongeturl);
PHP_FUNCTION(swf_actionnextframe);
PHP_FUNCTION(swf_actionprevframe);
PHP_FUNCTION(swf_actionplay);
PHP_FUNCTION(swf_actionstop);
PHP_FUNCTION(swf_actiontogglequality);
PHP_FUNCTION(swf_actionwaitforframe);
PHP_FUNCTION(swf_actionsettarget);
PHP_FUNCTION(swf_actiongotolabel);
PHP_FUNCTION(swf_defineline);
PHP_FUNCTION(swf_definerect);
PHP_FUNCTION(swf_startshape);
PHP_FUNCTION(swf_shapelinesolid);
PHP_FUNCTION(swf_shapefilloff);
PHP_FUNCTION(swf_shapefillsolid);
PHP_FUNCTION(swf_shapefillbitmapclip);
PHP_FUNCTION(swf_shapefillbitmaptile);
PHP_FUNCTION(swf_shapemoveto);
PHP_FUNCTION(swf_shapelineto);
PHP_FUNCTION(swf_shapecurveto);
PHP_FUNCTION(swf_shapecurveto3);
PHP_FUNCTION(swf_shapearc);
PHP_FUNCTION(swf_endshape);
PHP_FUNCTION(swf_definefont);
PHP_FUNCTION(swf_setfont);
PHP_FUNCTION(swf_fontsize);
PHP_FUNCTION(swf_fontslant);
PHP_FUNCTION(swf_fonttracking);
PHP_FUNCTION(swf_getfontinfo);
PHP_FUNCTION(swf_definetext);
PHP_FUNCTION(swf_textwidth);
PHP_FUNCTION(swf_definebitmap);
PHP_FUNCTION(swf_getbitmapinfo);
PHP_FUNCTION(swf_startsymbol);
PHP_FUNCTION(swf_endsymbol);
PHP_FUNCTION(swf_startbutton);
PHP_FUNCTION(swf_addbuttonrecord);
PHP_FUNCTION(swf_oncondition);
PHP_FUNCTION(swf_endbutton);
PHP_FUNCTION(swf_viewport);
PHP_FUNCTION(swf_ortho2);
PHP_FUNCTION(swf_perspective);
PHP_FUNCTION(swf_polarview);
PHP_FUNCTION(swf_lookat);
PHP_FUNCTION(swf_pushmatrix);
PHP_FUNCTION(swf_popmatrix);
PHP_FUNCTION(swf_scale);
PHP_FUNCTION(swf_translate);
PHP_FUNCTION(swf_rotate);
PHP_FUNCTION(swf_posround);

#else
#define swf_module_ptr NULL
#endif /* HAVE_FLASH */
#define phpext_swf_ptr swf_module_ptr
#endif  /* _PHP_FLASH_H */
