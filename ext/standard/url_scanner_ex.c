/* Generated by re2c 0.5 on Wed Sep 20 10:04:05 2000 */
#line 1 "/home/sas/src/php4/ext/standard/url_scanner_ex.re"
/*
  +----------------------------------------------------------------------+
  | PHP version 4.0                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Sascha Schumann <sascha@schumann.cx>                        |
  +----------------------------------------------------------------------+
*/

#include "php.h"

#ifdef TRANS_SID

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "php_globals.h"
#define STATE_TAG SOME_OTHER_STATE_TAG
#include "basic_functions.h"
#undef STATE_TAG

#define url_adapt_ext url_adapt_ext_ex
#define url_scanner url_scanner_ex

static inline void smart_str_append(smart_str *dest, smart_str *src)
{
	size_t newlen;

	if (!dest->c)
		dest->len = dest->a = 0;
	
	newlen = dest->len + src->len;
	if (newlen >= dest->a) {
		dest->c = realloc(dest->c, newlen + 129);
		dest->a = newlen + 128;
	}
	memcpy(dest->c + dest->len, src->c, src->len);
	dest->c[dest->len = newlen] = '\0';
}

static inline void smart_str_appendc(smart_str *dest, char c)
{
	size_t newlen;

	newlen = dest->len + 1;
	if (newlen >= dest->a) {
		dest->c = realloc(dest->c, newlen + 129);
		dest->a = newlen + 128;
	}
	dest->c[dest->len++] = c;
	dest->c[dest->len] = '\0';
}

static inline void smart_str_free(smart_str *s)
{
	if (s->c) {
		free(s->c);
		s->c = NULL;
	}
	s->a = s->len = 0;
}

static inline void smart_str_copyl(smart_str *dest, const char *src, size_t len)
{
	dest->c = realloc(dest->c, len + 1);
	memcpy(dest->c, src, len);
	dest->c[len] = '\0';
	dest->a = dest->len = len;
}

static inline void smart_str_appendl(smart_str *dest, const char *src, size_t len)
{
	smart_str s;

	s.c = (char *) src;
	s.len = len;

	smart_str_append(dest, &s);
}

static inline void smart_str_setl(smart_str *dest, const char *src, size_t len)
{
	dest->len = len;
	dest->a = len + 1;
	dest->c = (char *) src;
}

#define smart_str_appends(dest, src) smart_str_appendl(dest, src, sizeof(src)-1)

static inline void smart_str_sets(smart_str *dest, const char *src)
{
	smart_str_setl(dest, src, strlen(src));
}

static inline void append_modified_url(smart_str *url, smart_str *dest, smart_str *name, smart_str *val, const char *separator)
{
	register const char *p, *q;
	const char *bash = NULL;
	char sep = "?";
	
	q = url->c + url->len;
	
	for (p = url->c; p < q; p++) {
		switch(*p) {
			case ':':
				smart_str_append(dest, url);
				return;
			case '?':
				sep = *separator;
				break;
			case '#':
				bash = p;
				break;
		}
	}

	if (bash) 
		smart_str_appendl(dest, url->c, bash - url->c);
	else
		smart_str_append(dest, url);

	smart_str_appendc(dest, sep);
	smart_str_append(dest, name);
	smart_str_appendc(dest, '=');
	smart_str_append(dest, val);

	if (bash)
		smart_str_appendl(dest, bash, q - bash);
}

struct php_tag_arg {
	char *tag;
	int taglen;
	char *arg;
	int arglen;
};

#define TAG_ARG_ENTRY(a,b) {#a,sizeof(#a)-1,#b,sizeof(#b)-1},

static struct php_tag_arg check_tag_arg[] = {
	TAG_ARG_ENTRY(a, href)
	TAG_ARG_ENTRY(area, href)
	TAG_ARG_ENTRY(frame, src)
	TAG_ARG_ENTRY(img, src)
	TAG_ARG_ENTRY(input, src)
	TAG_ARG_ENTRY(form, fake_entry_for_passing_on_form_tag)
	{0}
};

static inline void tag_arg(url_adapt_state_ex_t *ctx PLS_DC)
{
	char f = 0;
	int i;

	for (i = 0; check_tag_arg[i].tag; i++) {
		if (check_tag_arg[i].arglen == ctx->arg.len
				&& check_tag_arg[i].taglen == ctx->tag.len
				&& strncasecmp(ctx->tag.c, check_tag_arg[i].tag, ctx->tag.len) == 0
				&& strncasecmp(ctx->arg.c, check_tag_arg[i].arg, ctx->arg.len) == 0) {
			f = 1;
			break;
		}
	}

	smart_str_appends(&ctx->result, "\"");
	if (f) {
		append_modified_url(&ctx->val, &ctx->result, &ctx->q_name, &ctx->q_value, PG(arg_separator));
	} else {
		smart_str_append(&ctx->result, &ctx->val);
	}
	smart_str_appends(&ctx->result, "\"");
}

enum {
	STATE_PLAIN,
	STATE_TAG,
	STATE_NEXT_ARG,
	STATE_ARG,
	STATE_BEFORE_VAL,
	STATE_VAL
};

#define YYFILL(n) goto stop
#define YYCTYPE char
#define YYCURSOR xp
#define YYLIMIT end
#define YYMARKER q
#define STATE ctx->state

#define PASSTHRU() {\
	smart_str_appendl(&ctx->result, start, YYCURSOR - start); \
}

#define HANDLE_FORM() {\
	if (ctx->tag.len == 4 && strncasecmp(ctx->tag.c, "form", 4) == 0) {\
		smart_str_appends(&ctx->result, "<INPUT TYPE=HIDDEN NAME=\""); \
		smart_str_append(&ctx->result, &ctx->q_name); \
		smart_str_appends(&ctx->result, "\" VALUE=\""); \
		smart_str_append(&ctx->result, &ctx->q_value); \
		smart_str_appends(&ctx->result, "\">"); \
	} \
}

/*
 *  HANDLE_TAG copies the HTML Tag and checks whether we 
 *  have that tag in our table. If we might modify it,
 *  we continue to scan the tag, otherwise we simply copy the complete
 *  HTML stuff to the result buffer.
 */

#define HANDLE_TAG() {\
	int ok = 0; \
	int i; \
	smart_str_setl(&ctx->tag, start, YYCURSOR - start); \
	for (i = 0; check_tag_arg[i].tag; i++) { \
		if (ctx->tag.len == check_tag_arg[i].taglen \
				&& strncasecmp(ctx->tag.c, check_tag_arg[i].tag, ctx->tag.len) == 0) { \
			ok = 1; \
			break; \
		} \
	} \
	STATE = ok ? STATE_NEXT_ARG : STATE_PLAIN; \
}

#define HANDLE_ARG() {\
	smart_str_setl(&ctx->arg, start, YYCURSOR - start); \
}
#define HANDLE_VAL(quotes) {\
	smart_str_setl(&ctx->val, start + quotes, YYCURSOR - start - quotes * 2); \
	tag_arg(ctx PLS_CC); \
}

/*
 * Since arg/tag are read-only during the mainloop, we do not need
 * to copy them. We need those variables across multiple calls 
 * to url_adapt() though, but they point to a private buffer. So we
 * copy them before leaving the mainloop() and restore them at
 * the beginning.
 */

#define MOVE_TO_CTX(X) \
	if (ctx->X.c) \
		smart_str_copyl(&ctx->c_##X, ctx->X.c, ctx->X.len); \
	else \
		smart_str_free(&ctx->c_##X)

#define FETCH_FROM_CTX(X) \
	smart_str_setl(&ctx->X, ctx->c_##X.c, ctx->c_##X.len)
	
static inline void mainloop(url_adapt_state_ex_t *ctx, const char *newdata, size_t newlen)
{
	char *end, *q;
	char *xp;
	char *start;
	int rest;
	PLS_FETCH();

	FETCH_FROM_CTX(arg);
	FETCH_FROM_CTX(tag);

	smart_str_appendl(&ctx->buf, newdata, newlen);
	
	YYCURSOR = ctx->buf.c;
	YYLIMIT = ctx->buf.c + ctx->buf.len;

#line 283

	
	while(1) {
		start = YYCURSOR;
#ifdef SCANNER_DEBUG
		printf("state %d at %s\n", STATE, YYCURSOR);
#endif
	switch(STATE) {
		
		case STATE_PLAIN:
{
	YYCTYPE yych;
	unsigned int yyaccept;
	goto yy0;
yy1:	++YYCURSOR;
yy0:
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych != '<')	goto yy4;
yy2:	yych = *++YYCURSOR;
yy3:
#line 294
	{ PASSTHRU(); STATE = STATE_TAG; continue; }
yy4:	yych = *++YYCURSOR;
yy5:
#line 295
	{ PASSTHRU(); continue; }
}
#line 296

			break;
			
		case STATE_TAG:
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128,   0,   0,   0,   0,   0, 
	  0, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	};
	goto yy6;
yy7:	++YYCURSOR;
yy6:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych <= '@')	goto yy10;
	if(yych <= 'Z')	goto yy8;
	if(yych <= '`')	goto yy10;
	if(yych >= '{')	goto yy10;
yy8:	yych = *++YYCURSOR;
	goto yy13;
yy9:
#line 301
	{ HANDLE_TAG() /* Sets STATE */; PASSTHRU(); continue; }
yy10:	yych = *++YYCURSOR;
yy11:
#line 302
	{ PASSTHRU(); continue; }
yy12:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy13:	if(yybm[0+yych] & 128)	goto yy12;
	goto yy9;
}
#line 303

  			break;
			
		case STATE_NEXT_ARG:
{
	YYCTYPE yych;
	unsigned int yyaccept;
	goto yy14;
yy15:	++YYCURSOR;
yy14:
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych <= '='){
		if(yych <= '\n'){
			if(yych <= '\t')	goto yy22;
			goto yy18;
		} else {
			if(yych == ' ')	goto yy18;
			goto yy22;
		}
	} else {
		if(yych <= 'Z'){
			if(yych <= '>')	goto yy16;
			if(yych <= '@')	goto yy22;
			goto yy20;
		} else {
			if(yych <= '`')	goto yy22;
			if(yych <= 'z')	goto yy20;
			goto yy22;
		}
	}
yy16:	yych = *++YYCURSOR;
yy17:
#line 308
	{ PASSTHRU(); HANDLE_FORM(); STATE = STATE_PLAIN; continue; }
yy18:	yych = *++YYCURSOR;
yy19:
#line 309
	{ PASSTHRU(); continue; }
yy20:	yych = *++YYCURSOR;
yy21:
#line 310
	{ YYCURSOR--; STATE = STATE_ARG; continue; }
yy22:	yych = *++YYCURSOR;
yy23:
#line 311
	{ PASSTHRU(); continue; }
}
#line 312

 	 		break;

		case STATE_ARG:
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128,   0,   0,   0,   0,   0, 
	  0, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	};
	goto yy24;
yy25:	++YYCURSOR;
yy24:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych <= '@')	goto yy28;
	if(yych <= 'Z')	goto yy26;
	if(yych <= '`')	goto yy28;
	if(yych >= '{')	goto yy28;
yy26:	yych = *++YYCURSOR;
	goto yy31;
yy27:
#line 317
	{ PASSTHRU(); HANDLE_ARG(); STATE = STATE_BEFORE_VAL; continue; }
yy28:	yych = *++YYCURSOR;
yy29:
#line 318
	{ PASSTHRU(); STATE = STATE_NEXT_ARG; continue; }
yy30:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy31:	if(yybm[0+yych] & 128)	goto yy30;
	goto yy27;
}
#line 319


		case STATE_BEFORE_VAL:
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	128,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	};
	goto yy32;
yy33:	++YYCURSOR;
yy32:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych == ' ')	goto yy34;
	if(yych == '=')	goto yy36;
	goto yy38;
yy34:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ' ')	goto yy41;
	if(yych == '=')	goto yy39;
yy35:
#line 324
	{ YYCURSOR--; STATE = STATE_NEXT_ARG; continue; }
yy36:	yych = *++YYCURSOR;
	goto yy40;
yy37:
#line 323
	{ PASSTHRU(); STATE = STATE_VAL; continue; }
yy38:	yych = *++YYCURSOR;
	goto yy35;
yy39:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy40:	if(yybm[0+yych] & 128)	goto yy39;
	goto yy37;
yy41:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy42:	if(yych == ' ')	goto yy41;
	if(yych == '=')	goto yy39;
yy43:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy35;
	}
}
#line 325

			break;

		case STATE_VAL:
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 128, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	128, 192,   0, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192,   0, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	};
	goto yy44;
yy45:	++YYCURSOR;
yy44:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych <= ' '){
		if(yych == '\n')	goto yy50;
		if(yych <= '\037')	goto yy48;
		goto yy50;
	} else {
		if(yych <= '"'){
			if(yych <= '!')	goto yy48;
		} else {
			if(yych == '>')	goto yy50;
			goto yy48;
		}
	}
yy46:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych != '>')	goto yy54;
yy47:
#line 332
	{ PASSTHRU(); STATE = STATE_NEXT_ARG; continue; }
yy48:	yych = *++YYCURSOR;
	goto yy52;
yy49:
#line 331
	{ HANDLE_VAL(0); STATE = STATE_NEXT_ARG; continue; }
yy50:	yych = *++YYCURSOR;
	goto yy47;
yy51:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy52:	if(yybm[0+yych] & 64)	goto yy51;
	goto yy49;
yy53:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy54:	if(yybm[0+yych] & 128)	goto yy53;
	if(yych <= '=')	goto yy56;
yy55:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy47;
	}
yy56:	yych = *++YYCURSOR;
yy57:
#line 330
	{ HANDLE_VAL(1); STATE = STATE_NEXT_ARG; continue; }
}
#line 333

			break;
	}
	}

stop:
#ifdef SCANNER_DEBUG
	printf("stopped in state %d at pos %d (%d:%c)\n", STATE, YYCURSOR - ctx->buf.c, *YYCURSOR, *YYCURSOR);
#endif

	MOVE_TO_CTX(tag);
	MOVE_TO_CTX(arg);

	rest = YYLIMIT - start;
		
	memmove(ctx->buf.c, start, rest);
	ctx->buf.c[rest] = '\0';
	ctx->buf.len = rest;
}


char *url_adapt_ext(const char *src, size_t srclen, const char *name, const char *value, size_t *newlen)
{
	char *ret;
	url_adapt_state_ex_t *ctx;
	BLS_FETCH();

	ctx = &BG(url_adapt_state_ex);

	smart_str_sets(&ctx->q_name, name);
	smart_str_sets(&ctx->q_value, value);
	mainloop(ctx, src, srclen);

	*newlen = ctx->result.len;

	if (ctx->result.len == 0) {
		return strdup("");
	}
	ret = ctx->result.c;
	ctx->result.c = NULL;
	ctx->result.len = ctx->result.a = 0;
	return ret;
}

PHP_RINIT_FUNCTION(url_scanner)
{
	url_adapt_state_ex_t *ctx;
	BLS_FETCH();
	
	ctx = &BG(url_adapt_state_ex);

	memset(ctx, 0, sizeof(*ctx));

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(url_scanner)
{
	url_adapt_state_ex_t *ctx;
	BLS_FETCH();
	
	ctx = &BG(url_adapt_state_ex);

	smart_str_free(&ctx->result);
	smart_str_free(&ctx->buf);
	smart_str_free(&ctx->c_tag);
	smart_str_free(&ctx->c_arg);

	return SUCCESS;
}

#endif
