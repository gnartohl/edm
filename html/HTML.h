/*****
* HTML.h : XmHTML Widget public header file.
*			Resource defines, enumerations and structures.
*
* This file Version	$Revision$
*
* Creation date:		Tue Apr 15 23:39:26 GMT+0100 1997
* Last modification: 	$Date$
* By:					$Author$
* Current State:		$State$
*
* Author:				newt
*
* Copyright (C) 1994-1997 by Ripley Software Development 
* All Rights Reserved
*
* This file is part of the XmHTML Widget Library
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/
/*****
* $Source$
*****/
/*****
* ChangeLog 
* $Log$
* Revision 1.1  2002/09/20 19:20:04  sinclair
* edm html widgets
*
* Revision 1.5  1997/08/31 17:30:39  newt
* Removed HT_TEXTFLOW
*
* Revision 1.4  1997/08/30 00:22:46  newt
* Alpha channel resources: XmNalphaChannelProcessing and
* XmNimageRGBConversion. Updated comments and reorganized a bunch of things.
*
* Revision 1.3  1997/08/01 12:52:11  newt
* Progressive image loading changes
*
* Revision 1.2  1997/05/28 01:29:28  newt
* XmImage changes: added the XmImageConfig structure and configuration flags.
* Added support for the XmNdecodeGIFProc resource.
*
* Revision 1.1  1997/04/29 14:19:18  newt
* Initial Revision
*
*****/ 

#ifndef _HTML_h_
#define _HTML_h_

/*****
* See the bottom of this file for resource names
*****/

/******************************************************************************
* Enumerations and other constants
******************************************************************************/

/*****
* HTML Elements internal id's
* This list is alphabetically sorted to speed up the searching process.
* DO NOT MODIFY
*****/
typedef enum{
HT_DOCTYPE, HT_A, HT_ADDRESS, HT_APPLET, HT_AREA, HT_B, HT_BASE, HT_BASEFONT,
HT_BIG, HT_BLOCKQUOTE, HT_BODY, HT_BR, HT_CAPTION, HT_CENTER, HT_CITE, HT_CODE,
HT_DD, HT_DFN, HT_DIR, HT_DIV, HT_DL, HT_DT, HT_EM, HT_FONT, HT_FORM, HT_FRAME,
HT_FRAMESET, HT_H1, HT_H2, HT_H3, HT_H4, HT_H5, HT_H6, HT_HEAD, HT_HR, HT_HTML,
HT_I, HT_IMG, HT_INPUT, HT_ISINDEX, HT_KBD, HT_LI, HT_LINK, HT_MAP, HT_MENU,
HT_META, HT_NOFRAMES, HT_OL, HT_OPTION, HT_P, HT_PARAM, HT_PRE, HT_SAMP,
HT_SCRIPT, HT_SELECT, HT_SMALL, HT_STRIKE, HT_STRONG, HT_STYLE, HT_SUB,
HT_SUP, HT_TAB, HT_TABLE, HT_TD, HT_TEXTAREA, HT_TH, HT_TITLE,
HT_TR, HT_TT, HT_U, HT_UL, HT_VAR, HT_ZTEXT
}htmlEnum;

/*****
* Corresponding HTML element name table. Indexing with the above enumeration
* will give the corresponding element name.
*****/
extern String html_tokens[];

/***** 
* XmHTML defines the following callback reasons. This might produce strange
* results once Motif decides to uses enum values above 16383.
* Send us a mail at ripley@xs4all.nl if you get problems that are due to
* these enumeration values.
*****/
enum{
	XmCR_HTML_ANCHORTRACK = 16384,		/* XmNanchorTrackCallback	*/
	XmCR_HTML_DOCUMENT,					/* XmNdocumentCallback		*/
	XmCR_HTML_FORM,						/* XmNformCallback			*/
	XmCR_HTML_FRAME,					/* XmNframeCallback			*/
	XmCR_HTML_FRAMECREATE,				/* XmNframeCallback			*/
	XmCR_HTML_FRAMEDESTROY,				/* XmNframeCallback			*/
	XmCR_HTML_IMAGEMAPACTIVATE,			/* XmNimagemapCallback		*/
	XmCR_HTML_IMAGEMAP,					/* XmNimagemapCallback		*/
	XmCR_HTML_LINK,						/* XmNlinkCallback			*/
	XmCR_HTML_MODIFYING_TEXT_VALUE,		/* XmNmodifyVerifyCallback	*/
	XmCR_HTML_MOTIONTRACK,				/* XmNmotionTrackCallback	*/
	XmCR_HTML_PARSER					/* XmNparserCallback		*/
};

/*****
* URL types XmHTML knows of.
* The port numbers are only shown for demonstration purposes, XmHTML doesn't
* care whether they are present or not.
*****/
typedef enum{
	ANCHOR_UNKNOWN = 0,			/* unknown href								*/
	ANCHOR_NAMED,				/* name="...."								*/
	ANCHOR_JUMP,				/* href="#..."								*/
	ANCHOR_FILE_LOCAL,			/* href="file.html"							*/
	ANCHOR_FILE_REMOTE,			/* href="file://foo.bar/file.html"			*/
	ANCHOR_FTP,					/* href="ftp://foo.bar/file"				*/
	ANCHOR_HTTP,				/* href="http://foo.bar/file.html"			*/
	ANCHOR_GOPHER,				/* href="gopher://foo.bar:70"				*/
	ANCHOR_WAIS,				/* href="wais://foo.bar"					*/
	ANCHOR_NEWS,				/* href="news://foo.bar"					*/
	ANCHOR_TELNET,				/* href="telnet://foo.bar:23"				*/
	ANCHOR_MAILTO,				/* href="mailto:foo@bar"					*/
	ANCHOR_EXEC,				/* href="exec:foo_bar"						*/
	ANCHOR_FORM_IMAGE			/* <input type=image>, only used internally	*/
}URLType;

/*****
* HTML Form component types
*****/
typedef enum {
	FORM_TEXT = 1,					/* textfield						*/
	FORM_PASSWD,					/* password textfield				*/
	FORM_CHECK,						/* checkbox							*/
	FORM_RADIO,						/* radiobox							*/
	FORM_RESET,						/* reset button						*/
	FORM_FILE,						/* filelisting						*/
	FORM_SELECT,					/* select parent					*/
	FORM_OPTION,					/* select child						*/
	FORM_TEXTAREA,					/* multiline edit field				*/
	FORM_IMAGE,						/* drawbutton						*/
	FORM_HIDDEN,					/* hidden input						*/
	FORM_SUBMIT						/* submit button					*/
}componentType;

/*****
* possible error codes for XmNparserCallback
*****/
typedef enum{
	HTML_UNKNOWN_ELEMENT = 1,	/* unknown HTML element						*/
	HTML_BAD,					/* very badly placed element				*/
	HTML_OPEN_BLOCK,			/* block still open while new block started	*/
	HTML_CLOSE_BLOCK,			/* block closed but was never opened		*/
	HTML_OPEN_ELEMENT,			/* unbalanced terminator					*/
	HTML_NESTED,				/* improperly nested element				*/
	HTML_VIOLATION,				/* bad content for current block/element	*/
	HTML_NOTIFY,				/* notification of text insertion/removal	*/
	HTML_INTERNAL				/* internal parser error					*/
}parserError;

/*****
* possible action codes for the action field in the XmHTMLParserCallbackStruct
*****/
enum{
	HTML_REMOVE = 1,			/* remove offending element					*/
	HTML_INSERT,				/* insert missing element					*/
	HTML_SWITCH,				/* switch offending and expected element	*/
	HTML_KEEP,					/* keep offending element					*/
	HTML_IGNORE,				/* ignore, proceed as if nothing happened	*/
	HTML_ALIAS,					/* alias an unknown element to known one	*/
	HTML_TERMINATE				/* terminate parser							*/
};

/*****
* Possible return codes for XmHTMLImageGetType().
*****/
enum{
	IMAGE_ERROR = 0,			/* error on image loading			*/
	IMAGE_UNKNOWN,				/* unknown image					*/
	IMAGE_XPM,					/* X11 pixmap						*/
	IMAGE_XBM,					/* X11 bitmap						*/
	IMAGE_GIF,					/* CompuServe(C) Gif87a or Gif89a	*/
	IMAGE_GIFANIM,				/* animated gif						*/
	IMAGE_GIFANIMLOOP,			/* animated gif with loop extension	*/
	IMAGE_GZF,					/* compatible Gif87a or Gif89a		*/
	IMAGE_GZFANIM,				/* compatible animated gif			*/
	IMAGE_GZFANIMLOOP,			/* compatible animated gif 			*/
	IMAGE_JPEG,					/* JPEG image						*/
	IMAGE_PNG,					/* PNG image						*/
	IMAGE_FLG					/* Fast Loadable Graphic			*/
};

/*****
* Possible return values for a function installed on the
* XmNprogressiveReadProc resource.
*****/
#define STREAM_OK		 1		/* internally used value					*/
#define STREAM_END		 0		/* data stream ended (no more data)			*/
#define STREAM_SUSPEND	-1		/* data stream suspended (not enough data)	*/
#define STREAM_ABORT	-2		/* data stream aborted						*/
#define STREAM_RESIZE	-3		/* resize input buffer						*/

/*****
* Possible return values for the XmNdecodeGIFProc resource and 
* values for the XmHTMLGIFStream state.
*****/
#define GIF_STREAM_OK		 2
#define GIF_STREAM_END		 1
#define GIF_STREAM_ERR		 0
#define GIF_STREAM_INIT		-1
#define GIF_STREAM_FINAL	-2

/*****
* Possible return values from a number of image related routines.
* The actual meaning depends on the routine used.
*****/
typedef enum{
	XmIMAGE_ERROR = 0,		/* unknown error occured */
	XmIMAGE_BAD,			/* bad function call: missing arg or so */
	XmIMAGE_UNKNOWN,		/* provided XmImage/XmImageInfo unknown/unbound */
	XmIMAGE_ALMOST,			/* action completed, further response necessary */
	XmIMAGE_OK				/* action completed. */
}XmImageStatus;

/*****
* Possible values for transparency (value for the "bg" field in both
* XmImage and XmImageInfo structures). Possible values are:
*
* XmIMAGE_NONE
*	indicates the image is not transparent
* XmIMAGE_TRANSPARENCY_BG
*	indicates the image achieves transparency by substituting the current
*	background setting (can be a single color or background image. Internally,
*	such transparency is achieved by using a clipmask).
* XmIMAGE_TRANSPARENCY_ALPHA
*	indicates the image achieves transparency by using an alpha channel.
*	This transparency is currently only used by PNG images with an alpha
*	channel or a tRNS chunk (which is expanded to an alpha channel internally).
*****/
enum{
	XmIMAGE_NONE = 0,
	XmIMAGE_TRANSPARENCY_BG,
	XmIMAGE_TRANSPARENCY_ALPHA
};

/*****
* Possible values for the colorspace value.
*
* XmIMAGE_COLORSPACE_GRAYSCALE
*	image contains only shades of gray. The colorcube is reduced to a 1D
*	representation. All components in a shade have the same value. The
*	pixel values are equal to the value of a single color component.
* XmIMAGE_COLORSPACE_INDEXED
*	image uses a fixed palette. Colorcube is mapped to a 1D lookup-table.
* XmIMAGE_COLORSPACE_RGB
*	image uses a full 3D colorcube. 
*****/
enum{
	/* XmIMAGE_NONE */
	XmIMAGE_COLORSPACE_GRAYSCALE = 1,
	XmIMAGE_COLORSPACE_INDEXED,
	XmIMAGE_COLORSPACE_RGB
};

/*****
* XmImageInfo structure options field bits.
* The ``Set by default'' indicates a bit set when the XmHTMLImageDefaultProc
* is used to read an image. The ``Read Only'' indicates a bit you should
* consider as read-only.
* XmIMAGE_DELAYED
*	Indicates the image is delayed, e.i. it will be provided at a later stage;
* XmIMAGE_DEFERRED_FREE
*	Indicates XmHTML may free this structure when a new document is loaded.
* XmIMAGE_IMMEDIATE_FREE
*	Indicates XmHTML may free this structure when XmHTML no longer needs it;
* XmIMAGE_RGB_SINGLE
*	Indicates that the reds, greens and blues fields are allocated in a single
*	memory area instead of three seperate memory arrays.
* XmIMAGE_ALLOW_SCALE
*	Indicates that scaling an image is allowed.
* XmIMAGE_FRAME_IGNORE
*	Use with animations: set this bit when a frame falls outside the logical
*	screen area. No pixmap is created but the timeout for the frame is kept.
* XmIMAGE_CLIPMASK
*	This bit is set when the returned XmImageInfo structure contains clipmask
*	data. XmHTML uses this info to create a clipping bitmap. Changing this
*	bit from set to unset will lead to a memory leak while changing it from
*	unset to set *without* providing a clipmask yourself *will* cause an error
*	to happen. You can set this bit when you are providing your own clipmask
*	(to provide non-rectangular images for example), PROVIDED you fill the
*	``clip'' field with valid bitmap data (a stream of bytes in XYBitmap format
*	and the same size of the image).
* XmIMAGE_SHARED_DATA
*	This bit is set when images share data. XmHTML sets this bit when the image
*	in question is an internal image, e.i., one for which the image data may
*	never be freed. Be carefull setting this bit yourself, since it prevents
*	XmHTML from freeing the image data present in the XmImageInfo structure.
*	It can easily lead to memory leaks when an image is *not* an internal
*	image.
* XmIMAGE_PROGRESSIVE
*	Setting this bit will enable XmHTML progressive image loading. A function
*	*must* have been installed on the XmNprogressiveReadProc resource *prior*
*	to setting this bit. Installing a function on the XmNprogressiveEndProc
*	is optional. When this bit is set all other bits will be ignored.
* XmIMAGE_DELAYED_CREATION
*	This bit is read-only. It is used internally by XmHTML for images with
*	an alpha channel. Alpha channel processing merges the current background
*	with the original RGB data from the image and uses the result to compose
*	the actual on-screen image (the merged data is stored in the ``data''
*	field of the XmImageInfo structure). XmHTML needs to store the original
*	data somewhere, and when this bit is set it is stored in the ``rgb'' field
*	of the XmImageInfo structure.
*	When this bit is set, the returned XmImageInfo may *NOT BE FREED* as long
*	as the current document is alive. You can discard it as soon as a new
*	document is loaded.
*****/
#define XmIMAGE_DELAYED			(1<<1)
#define XmIMAGE_DEFERRED_FREE	(1<<2)		/* set by default */
#define XmIMAGE_IMMEDIATE_FREE	(1<<3)
#define XmIMAGE_RGB_SINGLE		(1<<4)		/* set by default */
#define XmIMAGE_ALLOW_SCALE		(1<<5)		/* set by default */
#define XmIMAGE_FRAME_IGNORE	(1<<6)
#define XmIMAGE_CLIPMASK		(1<<7)		/* Read Only */
#define XmIMAGE_SHARED_DATA		(1<<8)		/* Read Only */
#define XmIMAGE_PROGRESSIVE		(1<<9)

#define XmIMAGE_DELAYED_CREATION (1<<10)	/* Read Only */

/*****
* XmImageInfo animation disposal values
* A disposal method specifies what should be done before the current frame is
* rendered. Possible values are:
* XmIMAGE_DISPOSE_NONE
*	do nothing, overlays the previous frame with the current frame.
* XmIMAGE_DISPOSE_BY_BACKGROUND
*	Restore to background color. The area used by the previous frame must
*	be restored to the background color/image
* XmIMAGE_DISPOSE_BY_PREVIOUS
*	Restore to previous. The area used by the previous frame must be
*	restored to what was there prior to rendering the previous frame.
*****/
enum{
	/* XmIMAGE_NONE */
	XmIMAGE_DISPOSE_NONE = 1,		/* default behaviour */
	XmIMAGE_DISPOSE_BY_BACKGROUND,
	XmIMAGE_DISPOSE_BY_PREVIOUS
};

/*****
* Primary image cache actions
* (unimplemented)
*****/
#define IMAGE_STORE		0			/* store an image in the cache */
#define IMAGE_GET		1			/* retrieve an image from the cache */
#define IMAGE_DISCARD	2			/* discard an image from the cache */

/*****
* XmNperfectColors/XmNalphaChannelProcessing resource values.
*
* Note: these values are represented by the XmCEnableMode resource class.
*****/
enum{
	/* XmAUTOMATIC */
	XmALWAYS = 1,
	XmNEVER
};

/*****
* Possible XmNimageMapToPalette/XmNimageRGBConversion resource values:
*
* XmQUICK
*	RGBConversion:
*		first checks if the 24bit image contains less than XmNmaxImageColors.
*		If not, XmHTML will dither to a fixed palette. This is fast but has
*		the disadvantage that the background color in an alpha channelled
*		image will not be matched exactly.
*	MapToPalette:
*		Use closest distance algorithm to map colors to the palette. No
*		error correction is performed. Reasonably fast, but quality 
*		heavily depends on the distribution of the colors in the image.
* XmBEST
*	RGBConversion (default):
*		first checks if the 24bit image contains less than XmNmaxImageColors.
*		If it is, the actual image colors are used. If not, a histogram of the
*		image is computed, the most used colors are selected and the resulting
*		image is dithered to this palette.
*		Offers best 24 to 8bit conversion and is probably faster than XmSLOW
*		as only images with more than XmNmaxImageColors will be dithered. 
*	MapToPalette:
*		Ordered dithering using predefined error matrices. Reasonably fast and
*		quite good results;
* XmFAST
*	RGBConversion:
*		Skips the check and dithers to a fixed palette right away. This is the
*		fastest way to do 24 to 8bit conversion but has the disadvantage that
*		every 24bit image is dithered to a fixed palette, regardless of the
*		actual no of colors in the image.
*	MapToPalette:
*		Simple ordered dithering. Fastest but probably the poorest results.
* XmSLOW
*	RGBConversion:
*		Skips the check and does histogram stuff right away.
*	MapToPalette:
*		closest distance algorithm to map image colors to the palette and use
*		dynamic error correction. Slowest but best results;
* XmDISABLED
*	RGBConversion:
*		ignored;
*	MapToPalette (default):
*		Disables palette mapping;
*
* Note: these values are represented by the XmCConversionMode resource class.
*****/
enum{
	XmQUICK = 0,
	XmBEST,
	XmFAST,
	XmSLOW,
	XmDISABLED
};

/*****
* Custom Papersize dimension unit type
* (under construction)
*****/
enum{
	XmHTML_CHAR = 0,
	XmHTML_CENTIMETER,			/* 1cm = 0.39in					*/
	XmHTML_MILLIMETER,			/* 1mm = 0.1cm					*/
	XmHTML_INCH,				/* 1in = 2.54cm					*/
	XmHTML_PICA,				/* 1pc = 12pt					*/
	XmHTML_POINT				/* 1in = 72.27pt, 1cm = 28.45pt	*/
};

/*****
* XmHTMLTextGetFormatted paper size defines
* (under construction)
*****/
enum{
	XmHTMLTEXT_PAPERSIZE_A4 = 0,
	XmHTMLTEXT_PAPERSIZE_LETTER,
	XmHTMLTEXT_PAPERSIZE_CUSTOM
};

/*****
* XmHTMLTextGetFormatted type definitions
* (under construction)
*****/
enum{
	XmHTMLTEXT_PLAIN = 0,		/* generate plain ASCII document		*/
	XmHTMLTEXT_FORMATTED,		/* generate formatted ASCII document	*/
	XmHTMLTEXT_POSTSCRIPT 		/* generate formatted Postscript output	*/
};

/*****
* XmHTMLTextGetFormatted Postscript option bits
* (under construction)
* The MIMIC_FONTS bit instructs XmHTML to use any of the supported postscript
* fonts to approach the fonts used in the document. When set, all other font
* bits are ignored. When not used, the PSFONT bits can be or'd together.
* XmHTML will attempt to do the following mapping:
*	PSFONT_ROMAN/PSFONT_CENTURY -> default text font;
*	PSFONT_HELVETICA/PSFONT_LUCIDA -> fixed width font;
* If only one of the PSFONT bits is set, the entire document will be rendered
* in that font.
*****/
#define XmHTMLTEXT_ADDHEADER				(1<<1)
#define XmHTMLTEXT_ADDFOOTER				(1<<2)
#define XmHTMLTEXT_PSFONT_ROMAN				(1<<3)
#define XmHTMLTEXT_PSFONT_HELVETICA			(1<<4)
#define XmHTMLTEXT_PSFONT_CENTURY			(1<<5)
#define XmHTMLTEXT_PSFONT_LUCIDA			(1<<6)
#define XmHTMLTEXT_MIMIC_FONTS				(1<<7)

/*****
* XmHTMLGetHeadAttributes mask bits
*****/
#define HeadClear		((unsigned char)0)	/* clear everything		*/
#define HeadDocType		(1<<0)				/* fill doctype member	*/
#define HeadTitle		(1<<1)				/* fill title member	*/
#define HeadIsIndex		(1<<2)				/* fill isIndex member	*/
#define HeadBase		(1<<3)				/* fill Base member		*/
#define HeadMeta		(1<<4)				/* fill meta members	*/
#define HeadLink		(1<<5)				/* fill link members	*/
#define HeadScript		(1<<6)				/* fill script members	*/
#define HeadStyle		(1<<7)				/* fill Style members	*/
#define HeadAll			((unsigned char)~0)	/* fill all members		*/

/*****
* XmImage frame selection flags.
* any positive number will return the requested frame. If larger than
* total framecount, last frame is returned.
*****/
#define AllFrames		-1 			/* do all frames				*/
#define FirstFrame		-2			/* only use first frame			*/
#define LastFrame		-3			/* only do last frame			*/

/*****
* XmImage configuration flags
*****/
#define ImageFSDither		(1L<<1)	/* Floyd-Steinberg on quantized images	*/
#define ImageCreateGC		(1L<<2)	/* create gc for image					*/
#define ImageWorkSpace		(1L<<3)	/* create animation workspace			*/
#define ImageClipmask		(1L<<4)	/* create clipmask						*/
#define ImageBackground		(1L<<5)	/* substitute background pixel			*/
#define ImageQuantize		(1L<<6)	/* quantize image						*/
#define ImageMaxColors		(1L<<7)	/* sets maximum colors					*/
#define ImageGifDecodeProc	(1L<<8)	/* gif lzw decoder function				*/
#define ImageGifzCmd		(1L<<9)	/* gif lzw uncompress command			*/
#define ImageFrameSelect	(1L<<10)/* frame selection						*/
#define ImageScreenGamma	(1L<<11)/* gamma correction. JPEG and PNG only	*/

/******************************************************************************
* Commonly used structures
******************************************************************************/
/*****
* XmHTMLTextBlock
*****/
typedef struct{
	String ptr;					/* pointer to text to remove/insert	*/
	int len;					/* length of this text				*/
}XmHTMLTextBlockRec, *XmHTMLTextBlock;

/*****
* A parser aliasing table
******/
typedef struct{
	String element;				/* element name				*/
	htmlEnum alias;				/* alias for this element	*/
}XmHTMLAlias, *XmHTMLAliasTable;

/*****
* Representation of parsed HTML elements
*****/
typedef struct _XmHTMLObject{
	htmlEnum id;				/* internal ID for this element				*/
	String element;				/* element text								*/
	String attributes;			/* element attributes (if any)				*/
	Boolean is_end;				/* true if this is a closing element		*/
	Boolean terminated;			/* true if element has closing counterpart	*/
	Cardinal line;				/* line number in input for this element	*/
	struct _XmHTMLObject *next;
	struct _XmHTMLObject *prev;
}XmHTMLObject;

/*****
* Custom papersize definition
*****/
typedef struct _XmHTMLPaperSize{
	unsigned char unit_type;	/* unit in which dimensions are specified	*/
	unsigned char paper_type;	/* type of paper: A4, US or custom			*/
	Cardinal width;				/* total paper width						*/
	Cardinal height;			/* total paper height						*/
	Cardinal left_margin;		/* left text margin							*/
	Cardinal right_margin;		/* right text margin						*/
	Cardinal top_margin;		/* top text margin							*/
	Cardinal bottom_margin;		/* bottom text margin						*/
}XmHTMLPaperSize;

/***** 
* The following structure is returned by the XmHTMLImageDefaultProc convenience
* function. When a procedure for the XmNimageProc resource is installed,
* it *must* return this structure.
*****/
typedef struct _XmImageInfo
{
	/* regular image fields */
	String url;					/* original location of image				*/
	unsigned char *data;		/* raw image data. ZPixmap format			*/
	unsigned char *clip;		/* raw clipmask data. XYBitmap format		*/
	Dimension width;			/* used image width, in pixels				*/
	Dimension height;			/* used image height, in pixels				*/
	Dimension *reds;			/* red image pixels							*/
	Dimension *greens;			/* green image pixels						*/
	Dimension *blues;			/* blue image pixels						*/
	int bg;						/* transparent pixel index/type				*/
	unsigned int ncolors;		/* Number of colors in the image			*/
	unsigned int options;		/* image option bits						*/

	/* image classification fields and original data */
	unsigned char type;			/* image type, see the IMAGE_ enum above	*/
	unsigned char depth;		/* bits per pixel for this image			*/
	unsigned char colorspace;	/* colorspace for this image				*/
	unsigned char transparency;	/* transparency type for this image			*/
	Dimension swidth;			/* image width as read from image			*/
	Dimension sheight;			/* image height as read from image			*/
	unsigned int scolors;		/* Original number of colors in the image	*/

	/* Special fields for images with an alpha channel */
	unsigned char *alpha;		/* alpha channel data 						*/
	float fg_gamma;				/* image gamma								*/

	/* Additional animation data */
	int x;						/* logical screen x-position for this frame	*/
	int y;						/* logical screen y-position for this frame	*/
	int loop_count;				/* animation loop count						*/
	unsigned char dispose;		/* image disposal method					*/
	int timeout;				/* frame refreshment in milliseconds		*/
	int nframes;				/* no of animation frames remaining			*/
	struct _XmImageInfo *frame;	/* ptr to next animation frame				*/

	XtPointer user_data;		/* any data to be stored with this image	*/
}XmImageInfo, *XmImageInfoStruct;

/* XmHTML method to load images */
typedef XmImageInfo* (*XmImageProc)(Widget, String);

/****
* The next two structures constitute the XmImage definition which are used by
* the XmImageCreate and XmImageDestroy routines. Please note that the *only*
* safe way to destroy an XmImage is to use the XmImageDestroy function.
* XmHTML does not use the XmImage structure, it is provided for your
* convenience.
****/
/****
* Animation frame data.
****/
typedef struct{
	int x;					/* x position in logical screen		*/
	int y;					/* y position in logical screen		*/
	int w;					/* width of this particular frame	*/
	int h;					/* height of this particular frame	*/
	int timeout;			/* timeout for the next frame		*/
	unsigned char dispose;	/* previous frame disposal method	*/
	Pixmap pixmap;			/* actual image						*/
	Pixmap clip;			/* image clipmask					*/
	Pixmap prev_state;		/* previous screen state			*/
}XmImageFrame;

/*****
* Actual image definition.
* If you want access to the xcc member, include the XCC.h header file.
*****/
typedef struct{
	String file;				/* originating file							*/
	unsigned char type;			/* image type, see the IMAGE_ enum below	*/
	Pixmap pixmap;				/* actual image								*/
	Pixmap clip;				/* for transparant pixmaps					*/
	unsigned int options;		/* image option bits						*/
	int width;					/* current image width, in pixels			*/
	int height;					/* current image height, in pixels			*/
	int ncolors;				/* no of colors in this image				*/
	int scolors;				/* specified no of colors 					*/
	int swidth;					/* image width as read from image			*/
	int sheight;				/* image height as read from image			*/
	int depth;					/* depth of this image						*/
	int npixels;				/* no of really allocated pixels			*/
	GC gc;						/* graphics context for rendering			*/

	/* animation data */
	XmImageFrame *frames;		/* array of animation frames				*/
	int nframes;				/* no of frames following					*/
	int current_frame;			/* current frame count						*/
	int current_loop;			/* current loop count						*/
	int loop_count;				/* maximum loop count						*/
	XtIntervalId proc_id;		/* timer id for animations					*/
	Widget w;					/* image owner								*/
	XtAppContext context;		/* Application context for animations		*/

	/* private data */
  	struct _XColorContext *xcc;	/* a lot of visual info						*/
}XmImage;

/*****
* Link member information.
*****/
typedef struct
{
	String url;				/* value of URL tag		*/
	String rel;				/* value of REL tag		*/
	String rev;				/* value of REV tag		*/
	String title;			/* value of TITLE tag	*/
}XmHTMLLinkDataRec, *XmHTMLLinkDataPtr;

/*****
* Meta member information.
*****/
typedef struct
{
	String http_equiv;		/* value of HTTP-EQUIV tag	*/
	String name;			/* value of NAME tag		*/
	String content;			/* value of CONTENT tag		*/
}XmHTMLMetaDataRec, *XmHTMLMetaDataPtr;

/*****
* XmHTMLHeadAttributes definition
*****/
typedef struct{
	String doctype;				/* doctype data								*/
	String title;				/* document title							*/
	Boolean is_index;			/* true when the <isindex> element exists	*/
	String base;				/* value of the <base> element				*/
	int num_meta;				/* number of META info to process			*/
	XmHTMLMetaDataPtr meta;		/* array of META info to process			*/
	int num_link;				/* number of LINK info to process			*/
	XmHTMLLinkDataPtr link;		/* array of LINK info to process			*/
	String style_type;			/* value of the style type element tag		*/
	String style;				/* <style></style> contents					*/
	String script_lang;			/* value of the language element tag		*/
	String script;				/* <script></script> contents				*/
}XmHTMLHeadAttributes;

/*****
* forward declaration of XmHTMLAnchorCallback structure
*****/
typedef struct _XmHTMLAnchorCallbackStruct *XmHTMLAnchorPtr;

/*****
* XmHTMLXYToInfo return value
* This structure and any of it members may *never* be freed by the caller.
*****/
typedef struct
{
	Cardinal line;				/* line number at selected position			*/
	Boolean is_map;				/* true when clicked image is an imagemap	*/
	int x,y;					/* position relative to image corner		*/
	XmImageInfo *image;			/* image data								*/
	XmHTMLAnchorPtr anchor;		/* possible anchor data						*/
}XmHTMLInfoStructure, *XmHTMLInfoPtr;

/*****
* XmHTML progressive object loading
* (PLC stands for Progressive Loader Context)
*****/
typedef struct _XmHTMLPLCStream{
	Cardinal total_in;			/* no of bytes received so far				*/
	Cardinal min_out;			/* minimum number of bytes requested		*/
	Cardinal max_out;			/* maximum number of bytes requested		*/
	XtPointer user_data;		/* any data registered on this PLC			*/
	unsigned char pad[24];		/* reserved for future use					*/
}XmHTMLPLCStream;

/*****
* External GIF decoder stream object. This is the only argument to any
* procedure installed on the XmNdecodeGIFProc resource.
*
* The first block is kept up to date by XmHTML and is read-only. When state
* is GIF_STREAM_INIT, the decoder should initialize it's private data and store
* it in the external_state field so that it can be used for successive calls
* to the decoder. When state is GIF_STREAM_FINAL, the decoder should wrapup
* and flush all pending data. It can also choose to destruct it's internal
* data structures here (another call with state set to GIF_STREAM_END will be
* made when the internal loader is destroying it's internal objects as
* well).
*
* All following fields are the ``public'' fields and must be updated by the 
* external decoder. The msg field can be set to an error message if the
* external decoder fails for some reason. XmHTML will then display this 
* error message and abort this image.
*****/
typedef struct _XmHTMLGIFStream{
	/* read-only fields */
	int state;					/* decoder state						*/
	int codesize;				/* initial LZW codesize					*/
	Boolean is_progressive;		/* when used by a progressive loader	*/
	unsigned char *next_in;		/* next input byte						*/
	Cardinal avail_in;			/* number of bytes available at next_in	*/
	Cardinal total_in;			/* total nb of input bytes read so far	*/

	/* fields to be updated by caller */
	unsigned char *next_out;	/* next output byte should be put there	*/
	Cardinal avail_out;			/* remaining free space at next_out		*/
	Cardinal total_out;			/* total nb of bytes output so far		*/

	String msg;					/* last error message, or NULL			*/
	XtPointer external_state;	/* room for decoder-specific data		*/
}XmHTMLGIFStream;

/* and the proto for the XmNdecodeGIFProc resource */
typedef int (*XmImageGifProc)(XmHTMLGIFStream*);

/*****
* Progressive Loading function prototypes.
* XmHTMLGetDataProc: proto for function installed on the
*                    XmNprogressiveReadProc resource;
* XmHTMLEndDataProc: proto for function installed on the
*                    XmNprogressiveEndProc resource;
*****/
typedef int  (*XmHTMLGetDataProc)(XmHTMLPLCStream*, XtPointer);
typedef void (*XmHTMLEndDataProc)(XmHTMLPLCStream*, XtPointer, int, Boolean);

/*****
* possible values for the third argument on the EndDataProc
*****/
enum{
	/* XmNONE = 0 */	/* PLCObject referenced by all objects */
	XmPLC_IMAGE,		/* PLCObject for an image */
	XmPLC_DOCUMENT,		/* PLCObject for a document */
	XmPLC_FINISHED		/* indicates all plc's have been processed */
};

/*****
* XmImage configuration
*****/
typedef struct{
	unsigned long flags;		/* XmImage configuration flags, see above	*/
	int ncolors;				/* desired number of colors					*/
	int which_frames;			/* animation frames selection flag			*/
	int bg_color;				/* background pixel on transparent images	*/
	String z_cmd;				/* gif uncompress command					*/
	XmImageGifProc gif_proc;	/* external gif decoder						*/
	float gamma;				/* gamma correction. JPEG and PNG only		*/
}XmImageConfig;

/******************************************************************************
* Callback structures
* Unless explicitly mentioned, *none* of these structures (or any of its
* members) may be freed.
******************************************************************************/

/*****
* XmNactivateCallback and XmNanchorTrackCallback callback structure.
*****/
typedef struct _XmHTMLAnchorCallbackStruct{
	int reason;				/* the reason the callback was called		*/
	XEvent *event;			/* event structure that triggered callback	*/
	URLType url_type;		/* type of url referenced					*/
	Cardinal line;			/* line number of the selected anchor		*/
	String href;			/* pointer to the anchor value				*/
	String target;			/* pointer to target value					*/
	String rel;				/* pointer to rel value						*/
	String rev;				/* pointer to rev value						*/
	String title;			/* pointer to title value					*/
	Boolean is_frame;		/* true when inside a frame					*/
	Boolean doit;			/* local anchor vetoing flag				*/
	Boolean visited;		/* local anchor visited flag				*/
}XmHTMLAnchorCallbackStruct;

/*****
* XmNdocumentCallback callback structure.
*****/
typedef struct
{
	int reason;					/* the reason the callback was called		*/
	XEvent *event;				/* always NULL for XmNdocumentCallback		*/
	Boolean html32;				/* True if document was HTML 3.2 conforming	*/
	Boolean verified;			/* True if document has been verified		*/
	Boolean balanced;			/* True if parser tree is balanced			*/
	Boolean terminated;			/* True if parser terminated prematurely	*/
	int pass_level;				/* current parser level count. Starts at 1	*/
	Boolean redo;				/* perform another pass?					*/
}XmHTMLDocumentCallbackStruct, *XmHTMLDocumentPtr;

/*****
* XmNformCallback callback structure.
*****/
/*****
* Form Component data
*****/
typedef struct
{
	componentType type;			/* Form component type	*/
	String name;				/* component name		*/
	String value;				/* component value		*/
}XmHTMLFormDataRec, *XmHTMLFormDataPtr;

/*****
* Actual XmNformCallback callback structure.
*****/
typedef struct
{
	int reason;				/* the reason the callback was called		*/
	XEvent *event;			/* event structure that triggered callback	*/
	String action;			/* URL or cgi-bin destination				*/
	String enctype;			/* form encoding							*/
	int method;				/* Form Method, GET (0) or POST (1)			*/
	int ncomponents;		/* no of components in this form			*/
	XmHTMLFormDataPtr components;
}XmHTMLFormCallbackStruct, *XmHTMLFormPtr;

/*****
* XmNframeCallback callback structure.
* This callback is activated when one of the following events occurs:
* 1. XmHTML wants to create a frame, reason = XmCR_HTML_FRAMECREATE
*    can be veto'd by setting doit to False and supplying a HTML widget
*    id yourself;
* 2. XmHTML wants to destroy a frame, reason = XmCR_HTML_FRAMEDESTROY
*    can be veto'd by setting doit to False (widget reuse).
* 3. XmHTML has finished creating a frame, reason = XmCR_HTML_FRAME.
*    This is the time to attach callbacks and set additional resources on the
*    newly created XmHTMLWidget.
*****/
typedef struct
{
	int reason;			/* the reason the callback was called		*/
	XEvent *event;		/* event structure that triggered callback	*/
	String src;			/* requested document						*/
	String name;		/* frame name								*/
	Widget html;		/* XmHTML widget id							*/
	Boolean doit;		/* destroy/create vetoing flag				*/
}XmHTMLFrameCallbackStruct, *XmHTMLFramePtr;

/*****
* XmNimagemapCallback callback structure.
* callback reasons can be one of the following: 
* XmCR_HTML_IMAGEMAP_ACTIVATE
*	user clicked on an image. Valid fields are x, y and image_name. x and y
*	are relative to the upper-left corner of the image. Only invoked when
*	an image has it's ismap attribute set and no usemap is present for this
*	image.
* XmCR_HTML_IMAGEMAP
*	an image requires an external imagemap. The only valid field is map_name
*	which contains the location of the imagemap to fetch. If the contents
*	of this imagemap is set in the map_contents field, it will be loaded
*	by the widget. Alternatively, one could also use the XmHTMLAddImagemap
*	convenience routine to set an imagemap into the widget.
*****/
typedef struct
{
	int reason;			/* the reason the callback was called				*/
	XEvent *event;		/* event structure that triggered callback			*/
	int x,y;			/* position relative to the upper-left image corner	*/
	String image_name;	/* name of referenced image, value of src attribute	*/
	String map_name;	/* name of imagemap to fetch						*/
	String map_contents;/* contents of fetched imagemap						*/
	XmImageInfo *image;	/* image data										*/
}XmHTMLImagemapCallbackStruct, *XmHTMLImagemapPtr;

/*****
* XmNlinkCallback callback structure.
*****/
typedef struct{
	int reason;					/* the reason the callback was called		*/
	XEvent *event;				/* event structure that triggered callback	*/
	int num_link;				/* number of LINK info to process			*/
	XmHTMLLinkDataPtr link;		/* array of LINK info to process			*/
}XmHTMLLinkCallbackStruct, *XmHTMLLinkPtr;

/*****
* XmNmodifyVerify callback structure.
*****/
typedef struct{
	int reason;				/* the reason the callback was called		*/
	XEvent *event;			/* always NULL for XmNmodifyVerifyCallback	*/
	Boolean doit;			/* unused									*/
	int action;				/* type of modification, HTML_REMOVE/INSERT	*/
	Cardinal line;			/* current line number in input text		*/
	int start_pos;			/* start of text to change					*/
	int end_pos;			/* end of text to change					*/
	XmHTMLTextBlock text;	/* describes text to be removed or inserted	*/
}XmHTMLVerifyCallbackStruct, *XmHTMLVerifyPtr;

/*****
* XmNparserCallback callback structure.
*****/
typedef struct{
	int reason;					/* the reason the callback was called	*/
	XEvent *event;				/* always NULL for XmNparserCallback	*/
	int errnum;					/* total error count uptil now			*/
	Cardinal line;				/* line number where error was detected	*/
	int start_pos;				/* absolute index where error starts	*/
	int end_pos;				/* absolute index where error ends		*/
	parserError error;			/* type of error						*/
	unsigned char action;		/* suggested correction action			*/
	String err_msg;				/* error message						*/
	XmHTMLTextBlock repair;		/* proposed element to insert			*/
	XmHTMLTextBlock current;	/* current parser state 				*/
	XmHTMLTextBlock offender;	/* offending element					*/
}XmHTMLParserCallbackStruct, *XmHTMLParserPtr;

/******************************************************************************
* Resource string definitions.
* The debug resources are only effective when libXmHTML was compiled with
* DEBUG defines.
******************************************************************************/

/*****
* Automatically generated file.
*  ***DO NOT EDIT THIS FILE***
*****/
/*****
* mkStrings Version 1.00, Build Date: Oct 10 1997 03:26:23
* File created at: Fri Oct 10 03:28:45 1997
*****/

#ifndef __XmHTML_Strings_h__
#define __XmHTML_Strings_h__

_XFUNCPROTOBEGIN

/*****
* Don't define XmHTML_STRINGDEFINES if you want to save space
*****/

#ifndef XmHTML_STRINGDEFINES
# ifndef _XmConst
#  define _XmConst
# endif
extern _XmConst char _XmHTMLStrings[];
#endif


#ifdef XmHTML_STRINGDEFINES
# ifndef XmN
#  define XmN ""
# endif
#else
# ifndef XmN
#  define XmN ((char *)&_XmHTMLStrings[0])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCAnchorUnderlineType
#  define XmCAnchorUnderlineType "AnchorUnderlineType"
# endif
# ifndef XmRAnchorUnderlineType
#  define XmRAnchorUnderlineType "AnchorUnderlineType"
# endif
#else
# ifndef XmCAnchorUnderlineType
#  define XmCAnchorUnderlineType ((char *)&_XmHTMLStrings[1])
# endif
# ifndef XmRAnchorUnderlineType
#  define XmRAnchorUnderlineType ((char *)&_XmHTMLStrings[1])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCAnchorVisitedProc
#  define XmCAnchorVisitedProc "AnchorVisitedProc"
# endif
# ifndef XmRAnchorVisitedProc
#  define XmRAnchorVisitedProc "AnchorVisitedProc"
# endif
#else
# ifndef XmCAnchorVisitedProc
#  define XmCAnchorVisitedProc ((char *)&_XmHTMLStrings[21])
# endif
# ifndef XmRAnchorVisitedProc
#  define XmRAnchorVisitedProc ((char *)&_XmHTMLStrings[21])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCConversionMode
#  define XmCConversionMode "ConversionMode"
# endif
# ifndef XmRConversionMode
#  define XmRConversionMode "ConversionMode"
# endif
#else
# ifndef XmCConversionMode
#  define XmCConversionMode ((char *)&_XmHTMLStrings[39])
# endif
# ifndef XmRConversionMode
#  define XmRConversionMode ((char *)&_XmHTMLStrings[39])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCDecodeGIFProc
#  define XmCDecodeGIFProc "DecodeGIFProc"
# endif
# ifndef XmRDecodeGIFProc
#  define XmRDecodeGIFProc "DecodeGIFProc"
# endif
#else
# ifndef XmCDecodeGIFProc
#  define XmCDecodeGIFProc ((char *)&_XmHTMLStrings[54])
# endif
# ifndef XmRDecodeGIFProc
#  define XmRDecodeGIFProc ((char *)&_XmHTMLStrings[54])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCEnableMode
#  define XmCEnableMode "EnableMode"
# endif
# ifndef XmREnableMode
#  define XmREnableMode "EnableMode"
# endif
#else
# ifndef XmCEnableMode
#  define XmCEnableMode ((char *)&_XmHTMLStrings[68])
# endif
# ifndef XmREnableMode
#  define XmREnableMode ((char *)&_XmHTMLStrings[68])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCImageProc
#  define XmCImageProc "ImageProc"
# endif
# ifndef XmRImageProc
#  define XmRImageProc "ImageProc"
# endif
#else
# ifndef XmCImageProc
#  define XmCImageProc ((char *)&_XmHTMLStrings[79])
# endif
# ifndef XmRImageProc
#  define XmRImageProc ((char *)&_XmHTMLStrings[79])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCMaxImageColors
#  define XmCMaxImageColors "MaxImageColors"
# endif
# ifndef XmRMaxImageColors
#  define XmRMaxImageColors "MaxImageColors"
# endif
#else
# ifndef XmCMaxImageColors
#  define XmCMaxImageColors ((char *)&_XmHTMLStrings[89])
# endif
# ifndef XmRMaxImageColors
#  define XmRMaxImageColors ((char *)&_XmHTMLStrings[89])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCPerfectColors
#  define XmCPerfectColors "PerfectColors"
# endif
# ifndef XmRPerfectColors
#  define XmRPerfectColors "PerfectColors"
# endif
#else
# ifndef XmCPerfectColors
#  define XmCPerfectColors ((char *)&_XmHTMLStrings[104])
# endif
# ifndef XmRPerfectColors
#  define XmRPerfectColors ((char *)&_XmHTMLStrings[104])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCProgressiveEndProc
#  define XmCProgressiveEndProc "ProgressiveEndProc"
# endif
# ifndef XmRProgressiveEndProc
#  define XmRProgressiveEndProc "ProgressiveEndProc"
# endif
#else
# ifndef XmCProgressiveEndProc
#  define XmCProgressiveEndProc ((char *)&_XmHTMLStrings[118])
# endif
# ifndef XmRProgressiveEndProc
#  define XmRProgressiveEndProc ((char *)&_XmHTMLStrings[118])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCProgressiveInitialDelay
#  define XmCProgressiveInitialDelay "ProgressiveInitialDelay"
# endif
# ifndef XmRProgressiveInitialDelay
#  define XmRProgressiveInitialDelay "ProgressiveInitialDelay"
# endif
#else
# ifndef XmCProgressiveInitialDelay
#  define XmCProgressiveInitialDelay ((char *)&_XmHTMLStrings[137])
# endif
# ifndef XmRProgressiveInitialDelay
#  define XmRProgressiveInitialDelay ((char *)&_XmHTMLStrings[137])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCProgressiveMaximumDelay
#  define XmCProgressiveMaximumDelay "ProgressiveMaximumDelay"
# endif
# ifndef XmRProgressiveMaximumDelay
#  define XmRProgressiveMaximumDelay "ProgressiveMaximumDelay"
# endif
#else
# ifndef XmCProgressiveMaximumDelay
#  define XmCProgressiveMaximumDelay ((char *)&_XmHTMLStrings[161])
# endif
# ifndef XmRProgressiveMaximumDelay
#  define XmRProgressiveMaximumDelay ((char *)&_XmHTMLStrings[161])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCProgressiveMinimumDelay
#  define XmCProgressiveMinimumDelay "ProgressiveMinimumDelay"
# endif
# ifndef XmRProgressiveMinimumDelay
#  define XmRProgressiveMinimumDelay "ProgressiveMinimumDelay"
# endif
#else
# ifndef XmCProgressiveMinimumDelay
#  define XmCProgressiveMinimumDelay ((char *)&_XmHTMLStrings[185])
# endif
# ifndef XmRProgressiveMinimumDelay
#  define XmRProgressiveMinimumDelay ((char *)&_XmHTMLStrings[185])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCProgressiveReadProc
#  define XmCProgressiveReadProc "ProgressiveReadProc"
# endif
# ifndef XmRProgressiveReadProc
#  define XmRProgressiveReadProc "ProgressiveReadProc"
# endif
#else
# ifndef XmCProgressiveReadProc
#  define XmCProgressiveReadProc ((char *)&_XmHTMLStrings[209])
# endif
# ifndef XmRProgressiveReadProc
#  define XmRProgressiveReadProc ((char *)&_XmHTMLStrings[209])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCScreenGamma
#  define XmCScreenGamma "ScreenGamma"
# endif
# ifndef XmRScreenGamma
#  define XmRScreenGamma "ScreenGamma"
# endif
#else
# ifndef XmCScreenGamma
#  define XmCScreenGamma ((char *)&_XmHTMLStrings[229])
# endif
# ifndef XmRScreenGamma
#  define XmRScreenGamma ((char *)&_XmHTMLStrings[229])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmCTopLine
#  define XmCTopLine "TopLine"
# endif
# ifndef XmRTopLine
#  define XmRTopLine "TopLine"
# endif
#else
# ifndef XmCTopLine
#  define XmCTopLine ((char *)&_XmHTMLStrings[241])
# endif
# ifndef XmRTopLine
#  define XmRTopLine ((char *)&_XmHTMLStrings[241])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNalphaChannelProcessing
#  define XmNalphaChannelProcessing "alphaChannelProcessing"
# endif
#else
# ifndef XmNalphaChannelProcessing
#  define XmNalphaChannelProcessing ((char *)&_XmHTMLStrings[249])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorActivatedBackground
#  define XmNanchorActivatedBackground "anchorActivatedBackground"
# endif
#else
# ifndef XmNanchorActivatedBackground
#  define XmNanchorActivatedBackground ((char *)&_XmHTMLStrings[272])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorActivatedForeground
#  define XmNanchorActivatedForeground "anchorActivatedForeground"
# endif
#else
# ifndef XmNanchorActivatedForeground
#  define XmNanchorActivatedForeground ((char *)&_XmHTMLStrings[298])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorButtons
#  define XmNanchorButtons "anchorButtons"
# endif
#else
# ifndef XmNanchorButtons
#  define XmNanchorButtons ((char *)&_XmHTMLStrings[324])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorCursor
#  define XmNanchorCursor "anchorCursor"
# endif
#else
# ifndef XmNanchorCursor
#  define XmNanchorCursor ((char *)&_XmHTMLStrings[338])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorDisplayCursor
#  define XmNanchorDisplayCursor "anchorDisplayCursor"
# endif
#else
# ifndef XmNanchorDisplayCursor
#  define XmNanchorDisplayCursor ((char *)&_XmHTMLStrings[351])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorForeground
#  define XmNanchorForeground "anchorForeground"
# endif
#else
# ifndef XmNanchorForeground
#  define XmNanchorForeground ((char *)&_XmHTMLStrings[371])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorTargetForeground
#  define XmNanchorTargetForeground "anchorTargetForeground"
# endif
#else
# ifndef XmNanchorTargetForeground
#  define XmNanchorTargetForeground ((char *)&_XmHTMLStrings[388])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorTargetUnderlineType
#  define XmNanchorTargetUnderlineType "anchorTargetUnderlineType"
# endif
#else
# ifndef XmNanchorTargetUnderlineType
#  define XmNanchorTargetUnderlineType ((char *)&_XmHTMLStrings[411])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorTrackCallback
#  define XmNanchorTrackCallback "anchorTrackCallback"
# endif
#else
# ifndef XmNanchorTrackCallback
#  define XmNanchorTrackCallback ((char *)&_XmHTMLStrings[437])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorUnderlineType
#  define XmNanchorUnderlineType "anchorUnderlineType"
# endif
#else
# ifndef XmNanchorUnderlineType
#  define XmNanchorUnderlineType ((char *)&_XmHTMLStrings[457])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorVisitedForeground
#  define XmNanchorVisitedForeground "anchorVisitedForeground"
# endif
#else
# ifndef XmNanchorVisitedForeground
#  define XmNanchorVisitedForeground ((char *)&_XmHTMLStrings[477])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorVisitedProc
#  define XmNanchorVisitedProc "anchorVisitedProc"
# endif
#else
# ifndef XmNanchorVisitedProc
#  define XmNanchorVisitedProc ((char *)&_XmHTMLStrings[501])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNanchorVisitedUnderlineType
#  define XmNanchorVisitedUnderlineType "anchorVisitedUnderlineType"
# endif
#else
# ifndef XmNanchorVisitedUnderlineType
#  define XmNanchorVisitedUnderlineType ((char *)&_XmHTMLStrings[519])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNbodyImage
#  define XmNbodyImage "bodyImage"
# endif
#else
# ifndef XmNbodyImage
#  define XmNbodyImage ((char *)&_XmHTMLStrings[546])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNcharset
#  define XmNcharset "charset"
# endif
#else
# ifndef XmNcharset
#  define XmNcharset ((char *)&_XmHTMLStrings[556])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdebugDisableWarnings
#  define XmNdebugDisableWarnings "debugDisableWarnings"
# endif
#else
# ifndef XmNdebugDisableWarnings
#  define XmNdebugDisableWarnings ((char *)&_XmHTMLStrings[564])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdebugEnableFullOutput
#  define XmNdebugEnableFullOutput "debugEnableFullOutput"
# endif
#else
# ifndef XmNdebugEnableFullOutput
#  define XmNdebugEnableFullOutput ((char *)&_XmHTMLStrings[585])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdebugFilePrefix
#  define XmNdebugFilePrefix "debugFilePrefix"
# endif
#else
# ifndef XmNdebugFilePrefix
#  define XmNdebugFilePrefix ((char *)&_XmHTMLStrings[607])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdebugLevels
#  define XmNdebugLevels "debugLevels"
# endif
#else
# ifndef XmNdebugLevels
#  define XmNdebugLevels ((char *)&_XmHTMLStrings[623])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdebugNoAnimationLoopCount
#  define XmNdebugNoAnimationLoopCount "debugNoAnimationLoopCount"
# endif
#else
# ifndef XmNdebugNoAnimationLoopCount
#  define XmNdebugNoAnimationLoopCount ((char *)&_XmHTMLStrings[635])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdebugSaveClipmasks
#  define XmNdebugSaveClipmasks "debugSaveClipmasks"
# endif
#else
# ifndef XmNdebugSaveClipmasks
#  define XmNdebugSaveClipmasks ((char *)&_XmHTMLStrings[661])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdecodeGIFProc
#  define XmNdecodeGIFProc "decodeGIFProc"
# endif
#else
# ifndef XmNdecodeGIFProc
#  define XmNdecodeGIFProc ((char *)&_XmHTMLStrings[680])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNdocumentCallback
#  define XmNdocumentCallback "documentCallback"
# endif
#else
# ifndef XmNdocumentCallback
#  define XmNdocumentCallback ((char *)&_XmHTMLStrings[694])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNenableBadHTMLWarnings
#  define XmNenableBadHTMLWarnings "enableBadHTMLWarnings"
# endif
#else
# ifndef XmNenableBadHTMLWarnings
#  define XmNenableBadHTMLWarnings ((char *)&_XmHTMLStrings[711])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNenableBodyColors
#  define XmNenableBodyColors "enableBodyColors"
# endif
#else
# ifndef XmNenableBodyColors
#  define XmNenableBodyColors ((char *)&_XmHTMLStrings[733])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNenableBodyImages
#  define XmNenableBodyImages "enableBodyImages"
# endif
#else
# ifndef XmNenableBodyImages
#  define XmNenableBodyImages ((char *)&_XmHTMLStrings[750])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNenableDocumentColors
#  define XmNenableDocumentColors "enableDocumentColors"
# endif
#else
# ifndef XmNenableDocumentColors
#  define XmNenableDocumentColors ((char *)&_XmHTMLStrings[767])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNenableDocumentFonts
#  define XmNenableDocumentFonts "enableDocumentFonts"
# endif
#else
# ifndef XmNenableDocumentFonts
#  define XmNenableDocumentFonts ((char *)&_XmHTMLStrings[788])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNenableOutlining
#  define XmNenableOutlining "enableOutlining"
# endif
#else
# ifndef XmNenableOutlining
#  define XmNenableOutlining ((char *)&_XmHTMLStrings[808])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNfontFamily
#  define XmNfontFamily "fontFamily"
# endif
#else
# ifndef XmNfontFamily
#  define XmNfontFamily ((char *)&_XmHTMLStrings[824])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNfontFamilyFixed
#  define XmNfontFamilyFixed "fontFamilyFixed"
# endif
#else
# ifndef XmNfontFamilyFixed
#  define XmNfontFamilyFixed ((char *)&_XmHTMLStrings[835])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNfontSizeFixedList
#  define XmNfontSizeFixedList "fontSizeFixedList"
# endif
#else
# ifndef XmNfontSizeFixedList
#  define XmNfontSizeFixedList ((char *)&_XmHTMLStrings[851])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNfontSizeList
#  define XmNfontSizeList "fontSizeList"
# endif
#else
# ifndef XmNfontSizeList
#  define XmNfontSizeList ((char *)&_XmHTMLStrings[869])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNformCallback
#  define XmNformCallback "formCallback"
# endif
#else
# ifndef XmNformCallback
#  define XmNformCallback ((char *)&_XmHTMLStrings[882])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNframeCallback
#  define XmNframeCallback "frameCallback"
# endif
#else
# ifndef XmNframeCallback
#  define XmNframeCallback ((char *)&_XmHTMLStrings[895])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNfreezeAnimations
#  define XmNfreezeAnimations "freezeAnimations"
# endif
#else
# ifndef XmNfreezeAnimations
#  define XmNfreezeAnimations ((char *)&_XmHTMLStrings[909])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimageEnable
#  define XmNimageEnable "imageEnable"
# endif
#else
# ifndef XmNimageEnable
#  define XmNimageEnable ((char *)&_XmHTMLStrings[926])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimageMapToPalette
#  define XmNimageMapToPalette "imageMapToPalette"
# endif
#else
# ifndef XmNimageMapToPalette
#  define XmNimageMapToPalette ((char *)&_XmHTMLStrings[938])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimagePalette
#  define XmNimagePalette "imagePalette"
# endif
#else
# ifndef XmNimagePalette
#  define XmNimagePalette ((char *)&_XmHTMLStrings[956])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimageProc
#  define XmNimageProc "imageProc"
# endif
#else
# ifndef XmNimageProc
#  define XmNimageProc ((char *)&_XmHTMLStrings[969])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimageRGBConversion
#  define XmNimageRGBConversion "imageRGBConversion"
# endif
#else
# ifndef XmNimageRGBConversion
#  define XmNimageRGBConversion ((char *)&_XmHTMLStrings[979])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimagemapBoundingBoxForeground
#  define XmNimagemapBoundingBoxForeground "imagemapBoundingBoxForeground"
# endif
#else
# ifndef XmNimagemapBoundingBoxForeground
#  define XmNimagemapBoundingBoxForeground ((char *)&_XmHTMLStrings[998])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimagemapCallback
#  define XmNimagemapCallback "imagemapCallback"
# endif
#else
# ifndef XmNimagemapCallback
#  define XmNimagemapCallback ((char *)&_XmHTMLStrings[1028])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNimagemapDrawBoundingBoxes
#  define XmNimagemapDrawBoundingBoxes "imagemapDrawBoundingBoxes"
# endif
#else
# ifndef XmNimagemapDrawBoundingBoxes
#  define XmNimagemapDrawBoundingBoxes ((char *)&_XmHTMLStrings[1045])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNlinkCallback
#  define XmNlinkCallback "linkCallback"
# endif
#else
# ifndef XmNlinkCallback
#  define XmNlinkCallback ((char *)&_XmHTMLStrings[1071])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNmaxImageColors
#  define XmNmaxImageColors "maxImageColors"
# endif
#else
# ifndef XmNmaxImageColors
#  define XmNmaxImageColors ((char *)&_XmHTMLStrings[1084])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNmimeType
#  define XmNmimeType "mimeType"
# endif
#else
# ifndef XmNmimeType
#  define XmNmimeType ((char *)&_XmHTMLStrings[1099])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNmotionTrackCallback
#  define XmNmotionTrackCallback "motionTrackCallback"
# endif
#else
# ifndef XmNmotionTrackCallback
#  define XmNmotionTrackCallback ((char *)&_XmHTMLStrings[1108])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNparserCallback
#  define XmNparserCallback "parserCallback"
# endif
#else
# ifndef XmNparserCallback
#  define XmNparserCallback ((char *)&_XmHTMLStrings[1128])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNparserIsProgressive
#  define XmNparserIsProgressive "parserIsProgressive"
# endif
#else
# ifndef XmNparserIsProgressive
#  define XmNparserIsProgressive ((char *)&_XmHTMLStrings[1143])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNperfectColors
#  define XmNperfectColors "perfectColors"
# endif
#else
# ifndef XmNperfectColors
#  define XmNperfectColors ((char *)&_XmHTMLStrings[1163])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNprogressiveEndProc
#  define XmNprogressiveEndProc "progressiveEndProc"
# endif
#else
# ifndef XmNprogressiveEndProc
#  define XmNprogressiveEndProc ((char *)&_XmHTMLStrings[1177])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNprogressiveInitialDelay
#  define XmNprogressiveInitialDelay "progressiveInitialDelay"
# endif
#else
# ifndef XmNprogressiveInitialDelay
#  define XmNprogressiveInitialDelay ((char *)&_XmHTMLStrings[1196])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNprogressiveMaximumDelay
#  define XmNprogressiveMaximumDelay "progressiveMaximumDelay"
# endif
#else
# ifndef XmNprogressiveMaximumDelay
#  define XmNprogressiveMaximumDelay ((char *)&_XmHTMLStrings[1220])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNprogressiveMinimumDelay
#  define XmNprogressiveMinimumDelay "progressiveMinimumDelay"
# endif
#else
# ifndef XmNprogressiveMinimumDelay
#  define XmNprogressiveMinimumDelay ((char *)&_XmHTMLStrings[1244])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNprogressiveReadProc
#  define XmNprogressiveReadProc "progressiveReadProc"
# endif
#else
# ifndef XmNprogressiveReadProc
#  define XmNprogressiveReadProc ((char *)&_XmHTMLStrings[1268])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNretainSource
#  define XmNretainSource "retainSource"
# endif
#else
# ifndef XmNretainSource
#  define XmNretainSource ((char *)&_XmHTMLStrings[1288])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNscreenGamma
#  define XmNscreenGamma "screenGamma"
# endif
#else
# ifndef XmNscreenGamma
#  define XmNscreenGamma ((char *)&_XmHTMLStrings[1301])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNstrictHTMLChecking
#  define XmNstrictHTMLChecking "strictHTMLChecking"
# endif
#else
# ifndef XmNstrictHTMLChecking
#  define XmNstrictHTMLChecking ((char *)&_XmHTMLStrings[1313])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNtopLine
#  define XmNtopLine "topLine"
# endif
#else
# ifndef XmNtopLine
#  define XmNtopLine ((char *)&_XmHTMLStrings[1332])
# endif
#endif

#ifdef XmHTML_STRINGDEFINES
# ifndef XmNuncompressCommand
#  define XmNuncompressCommand "uncompressCommand"
# endif
#else
# ifndef XmNuncompressCommand
#  define XmNuncompressCommand ((char *)&_XmHTMLStrings[1340])
# endif
#endif

_XFUNCPROTOEND

/* Don't add anything after this endif! */
#endif /* __XmHTML_Strings_h__ */

/* Don't add anything after this endif! */
#endif /* _HTML_h_ */
