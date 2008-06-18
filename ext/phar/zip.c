/*
  +----------------------------------------------------------------------+
  | ZIP archive support for Phar                                         |
  +----------------------------------------------------------------------+
  | Copyright (c) 2007-2008 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Gregory Beaver <cellog@php.net>                             |
  +----------------------------------------------------------------------+
*/

#include "phar_internal.h"

#ifdef WORDS_BIGENDIAN
# define PHAR_GET_32(buffer) (((((unsigned char*)(buffer))[3]) << 24) \
		| ((((unsigned char*)(buffer))[2]) << 16) \
		| ((((unsigned char*)(buffer))[1]) <<  8) \
		| (((unsigned char*)(buffer))[0]))
# define PHAR_GET_16(buffer) (((((unsigned char*)(buffer))[1]) <<  8) \
		| (((unsigned char*)(buffer))[0]))
# define PHAR_SET_32(buffer) PHAR_GET_32(buffer)
# define PHAR_SET_16(buffer) PHAR_GET_16(buffer)
#else
# define PHAR_GET_32(buffer) (buffer)
# define PHAR_GET_16(buffer) (buffer)
# define PHAR_SET_32(buffer) (buffer)
# define PHAR_SET_16(buffer) (buffer)
#endif

static int phar_zip_process_extra(php_stream *fp, phar_entry_info *entry, php_uint16 len TSRMLS_DC)
{
	union {
		phar_zip_extra_field_header header;
		phar_zip_unix3 unix3;
	} h;
	int read;

	do {
		if (sizeof(h.header) != php_stream_read(fp, (char *) &h.header, sizeof(h.header))) {
			return FAILURE;
		}
		if (h.header.tag[0] != 'n' || h.header.tag[1] != 'u') {
			/* skip to next header */
			php_stream_seek(fp, PHAR_GET_16(h.header.size), SEEK_CUR);
			len -= PHAR_GET_16(h.header.size) + 4;
			continue;
		}
		/* unix3 header found */
		read = php_stream_read(fp, (char *) &(h.unix3.crc32), sizeof(h.unix3) - sizeof(h.header));
		len -= read + 4;
		if (sizeof(h.unix3) - sizeof(h.header) != read) {
			return FAILURE;
		}
		if (PHAR_GET_16(h.unix3.size) > sizeof(h.unix3) - 4) {
			/* skip symlink filename - we may add this support in later */
			php_stream_seek(fp, h.unix3.size - sizeof(h.unix3.size), SEEK_CUR);
		}
		/* set permissions */
		entry->flags &= PHAR_ENT_COMPRESSION_MASK;
		if (entry->is_dir) {
			entry->flags |= PHAR_GET_16(h.unix3.perms) & PHAR_ENT_PERM_MASK;
		} else {
			entry->flags |= PHAR_GET_16(h.unix3.perms) & PHAR_ENT_PERM_MASK;
		}
	} while (len);
	return SUCCESS;
}

/*
  extracted from libzip
  zip_dirent.c -- read directory entry (local or central), clean dirent
  Copyright (C) 1999, 2003, 2004, 2005 Dieter Baron and Thomas Klausner

  This function is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
static time_t phar_zip_d2u_time(int dtime, int ddate)
{
    struct tm *tm, tmbuf;
    time_t now;

    now = time(NULL);
    tm = php_localtime_r(&now, &tmbuf);
    
    tm->tm_year = ((ddate>>9)&127) + 1980 - 1900;
    tm->tm_mon = ((ddate>>5)&15) - 1;
    tm->tm_mday = ddate&31;

    tm->tm_hour = (dtime>>11)&31;
    tm->tm_min = (dtime>>5)&63;
    tm->tm_sec = (dtime<<1)&62;

    return mktime(tm);
}

static void phar_zip_u2d_time(time_t time, php_uint16 *dtime, php_uint16 *ddate)
{
    struct tm *tm, tmbuf;

    tm = php_localtime_r(&time, &tmbuf);
    *ddate = ((tm->tm_year+1900-1980)<<9) + ((tm->tm_mon+1)<<5) + tm->tm_mday;
    *dtime = ((tm->tm_hour)<<11) + ((tm->tm_min)<<5) + ((tm->tm_sec)>>1);
}

/**
 * Does not check for a previously opened phar in the cache.
 *
 * Parse a new one and add it to the cache, returning either SUCCESS or 
 * FAILURE, and setting pphar to the pointer to the manifest entry
 * 
 * This is used by phar_open_from_fp to process a zip-based phar, but can be called
 * directly.
 */
int phar_parse_zipfile(php_stream *fp, char *fname, int fname_len, char *alias, int alias_len, phar_archive_data** pphar, char **error TSRMLS_DC) /* {{{ */
{
	phar_zip_dir_end locator;
	char buf[sizeof(locator) + 65536];
	long size;
	size_t read;
	php_uint16 i;
	phar_archive_data *mydata = NULL;
	phar_entry_info entry = {0};
	char *p = buf, *ext, *actual_alias = NULL;

	size = php_stream_tell(fp);
	if (size > sizeof(locator) + 65536) {
		/* seek to max comment length + end of central directory record */
		size = sizeof(locator) + 65536;
		if (FAILURE == php_stream_seek(fp, -size, SEEK_END)) {
			php_stream_close(fp);
			if (error) {
				spprintf(error, 4096, "phar error: unable to search for end of central directory in zip-based phar \"%s\"", fname);
			}
			return FAILURE;
		}
	} else {
		php_stream_seek(fp, 0, SEEK_SET);
	}
	if (!(read = php_stream_read(fp, buf, size))) {
		php_stream_close(fp);
		if (error) {
			spprintf(error, 4096, "phar error: unable to read in data to search for end of central directory in zip-based phar \"%s\"", fname);
		}
		return FAILURE;
	}
	while ((p=(char *) memchr(p + 1, 'P', (size_t) (size - (p + 1 - buf)))) != NULL) {
		if (!memcmp(p + 1, "K\5\6", 3)) {
			memcpy((void *)&locator, (void *) p, sizeof(locator));
			if (locator.centraldisk != 0 || locator.disknumber != 0) {
				/* split archives not handled */
				php_stream_close(fp);
				if (error) {
					spprintf(error, 4096, "phar error: split archives spanning multiple zips cannot be processed in zip-based phar \"%s\"", fname);
				}
				return FAILURE;
			}
			if (locator.counthere != locator.count) {
				if (error) {
					spprintf(error, 4096, "phar error: corrupt zip archive, conflicting file count in end of central directory record in zip-based phar \"%s\"", fname);
				}
				php_stream_close(fp);
				return FAILURE;
			}
			mydata = pecalloc(sizeof(phar_archive_data), 1, PHAR_G(persist));
			mydata->is_persistent = PHAR_G(persist);

			/* read in archive comment, if any */
			if (locator.comment_len) {
				char *metadata;

				metadata = p + sizeof(locator);
				if (PHAR_GET_16(locator.comment_len) != size - (metadata - buf)) {
					if (error) {
						spprintf(error, 4096, "phar error: corrupt zip archive, zip file comment truncated in zip-based phar \"%s\"", fname);
					}
					php_stream_close(fp);
					pefree(mydata, mydata->is_persistent);
					return FAILURE;
				}
				mydata->metadata_len = PHAR_GET_16(locator.comment_len);
				if (phar_parse_metadata(&metadata, &mydata->metadata, PHAR_GET_16(locator.comment_len) TSRMLS_CC) == FAILURE) {
					mydata->metadata_len = 0;
					/* if not valid serialized data, it is a regular string */
					if (entry.is_persistent) {
						ALLOC_PERMANENT_ZVAL(mydata->metadata);
					} else {
						ALLOC_ZVAL(mydata->metadata);
					}
					INIT_ZVAL(*mydata->metadata);
					metadata = pestrndup(metadata, PHAR_GET_16(locator.comment_len), mydata->is_persistent);
					ZVAL_STRINGL(mydata->metadata, metadata, PHAR_GET_16(locator.comment_len), 0);
				}
			} else {
				mydata->metadata = NULL;
			}
			goto foundit;
		}
	}
	php_stream_close(fp);
	if (error) {
		spprintf(error, 4096, "phar error: end of central directory not found in zip-based phar \"%s\"", fname);
	}
	return FAILURE;
foundit:
	mydata->fname = pestrndup(fname, fname_len, mydata->is_persistent);
#ifdef PHP_WIN32
	phar_unixify_path_separators(mydata->fname, fname_len);
#endif
	mydata->is_zip = 1;
	mydata->fname_len = fname_len;
	ext = strrchr(mydata->fname, '/');
	if (ext) {
		mydata->ext = memchr(ext, '.', (mydata->fname + fname_len) - ext);
		if (mydata->ext == ext) {
			mydata->ext = memchr(ext + 1, '.', (mydata->fname + fname_len) - ext - 1);
		}
		if (mydata->ext) {
			mydata->ext_len = (mydata->fname + fname_len) - mydata->ext;
		}
	}
	/* clean up on big-endian systems */
	/* seek to central directory */
	php_stream_seek(fp, PHAR_GET_32(locator.cdir_offset), SEEK_SET);
	/* read in central directory */
	zend_hash_init(&mydata->manifest, PHAR_GET_16(locator.count),
		zend_get_hash_value, destroy_phar_manifest_entry, (zend_bool)mydata->is_persistent);
	zend_hash_init(&mydata->mounted_dirs, 5,
		zend_get_hash_value, NULL, (zend_bool)mydata->is_persistent);
	zend_hash_init(&mydata->virtual_dirs, PHAR_GET_16(locator.count) * 2,
		zend_get_hash_value, NULL, (zend_bool)mydata->is_persistent);
	entry.phar = mydata;
	entry.is_zip = 1;
	entry.fp_type = PHAR_FP;
	entry.is_persistent = mydata->is_persistent;
#define PHAR_ZIP_FAIL(errmsg) \
			zend_hash_destroy(&mydata->manifest); \
			mydata->manifest.arBuckets = 0; \
			zend_hash_destroy(&mydata->mounted_dirs); \
			mydata->mounted_dirs.arBuckets = 0; \
			zend_hash_destroy(&mydata->virtual_dirs); \
			mydata->virtual_dirs.arBuckets = 0; \
			php_stream_close(fp); \
			if (mydata->metadata) { \
				zval_dtor(mydata->metadata); \
			} \
			if (error) { \
				spprintf(error, 4096, "phar error: %s in zip-based phar \"%s\"", errmsg, mydata->fname); \
			} \
			pefree(mydata->fname, mydata->is_persistent); \
			if (mydata->alias) { \
				pefree(mydata->alias, mydata->is_persistent); \
			} \
			pefree(mydata, mydata->is_persistent); \
			return FAILURE;

	/* add each central directory item to the manifest */
	for (i = 0; i < PHAR_GET_16(locator.count); ++i) {
		phar_zip_central_dir_file zipentry;

		if (sizeof(zipentry) != php_stream_read(fp, (char *) &zipentry, sizeof(zipentry))) {
			PHAR_ZIP_FAIL("unable to read central directory entry, truncated");
		}
		/* clean up for bigendian systems */
		if (memcmp("PK\1\2", zipentry.signature, 4)) {
			/* corrupted entry */
			PHAR_ZIP_FAIL("corrupted central directory entry, no magic signature");
		}
		if (entry.is_persistent) entry.manifest_pos = i;
		entry.compressed_filesize = PHAR_GET_32(zipentry.compsize);
		entry.uncompressed_filesize = PHAR_GET_32(zipentry.uncompsize);
		entry.crc32 = PHAR_GET_32(zipentry.crc32);
		/* do not PHAR_GET_16 either on the next line */
		entry.timestamp = phar_zip_d2u_time(zipentry.timestamp, zipentry.datestamp);
		entry.flags = PHAR_ENT_PERM_DEF_FILE;
		entry.header_offset = PHAR_GET_32(zipentry.offset);
		entry.offset = entry.offset_abs = PHAR_GET_32(zipentry.offset) + sizeof(phar_zip_file_header) + PHAR_GET_16(zipentry.filename_len) +
			PHAR_GET_16(zipentry.extra_len);
		if (PHAR_GET_16(zipentry.flags) & PHAR_ZIP_FLAG_ENCRYPTED) {
			PHAR_ZIP_FAIL("Cannot process encrypted zip files");
		}
		if (!PHAR_GET_16(zipentry.filename_len)) {
			PHAR_ZIP_FAIL("Cannot process zips created from stdin (zero-length filename)");
		}
		entry.filename_len = PHAR_GET_16(zipentry.filename_len);
		entry.filename = (char *) pemalloc(entry.filename_len + 1, entry.is_persistent);
		if (entry.filename_len != php_stream_read(fp, entry.filename, entry.filename_len)) {
			pefree(entry.filename, entry.is_persistent);
			PHAR_ZIP_FAIL("unable to read in filename from central directory, truncated");
		}
		entry.filename[entry.filename_len] = '\0';
		if (entry.filename[entry.filename_len - 1] == '/') {
			entry.is_dir = 1;
			entry.filename_len--;
			entry.flags |= PHAR_ENT_PERM_DEF_DIR;
		} else {
			entry.is_dir = 0;
		}
		phar_add_virtual_dirs(mydata, entry.filename, entry.filename_len TSRMLS_CC);
		if (PHAR_GET_16(zipentry.extra_len)) {
			off_t loc = php_stream_tell(fp);
			if (FAILURE == phar_zip_process_extra(fp, &entry, PHAR_GET_16(zipentry.extra_len) TSRMLS_CC)) {
				efree(entry.filename);
				PHAR_ZIP_FAIL("Unable to process extra field header for file in central directory");
			}
			php_stream_seek(fp, loc + PHAR_GET_16(zipentry.extra_len), SEEK_SET);
		}
		switch (zipentry.compressed) {
			case PHAR_ZIP_COMP_NONE :
				/* compression flag already set */
				break;
			case PHAR_ZIP_COMP_DEFLATE :
				entry.flags |= PHAR_ENT_COMPRESSED_GZ;
				if (!PHAR_G(has_zlib)) {
					efree(entry.filename);
					PHAR_ZIP_FAIL("zlib extension is required");
				}
				break;
			case PHAR_ZIP_COMP_BZIP2 :
				entry.flags |= PHAR_ENT_COMPRESSED_BZ2;
				if (!PHAR_G(has_bz2)) {
					efree(entry.filename);
					PHAR_ZIP_FAIL("bzip2 extension is required");
				}
				break;
			case 1 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (Shrunk) used in this zip");
			case 2 :
			case 3 :
			case 4 :
			case 5 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (Reduce) used in this zip");
			case 6 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (Implode) used in this zip");
			case 7 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (Tokenize) used in this zip");
			case 9 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (Deflate64) used in this zip");
			case 10 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (PKWare Implode/old IBM TERSE) used in this zip");
			case 14 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (LZMA) used in this zip");
			case 18 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (IBM TERSE) used in this zip");
			case 19 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (IBM LZ77) used in this zip");
			case 97 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (WavPack) used in this zip");
			case 98 :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (PPMd) used in this zip");
			default :
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unsupported compression method (unknown) used in this zip");
		}
		/* get file metadata */
		if (zipentry.comment_len) {
			if (PHAR_GET_16(zipentry.comment_len) != php_stream_read(fp, buf, PHAR_GET_16(zipentry.comment_len))) {
				pefree(entry.filename, entry.is_persistent);
				PHAR_ZIP_FAIL("unable to read in file comment, truncated");
			}
			p = buf;
			entry.metadata_len = zipentry.comment_len;
			if (phar_parse_metadata(&p, &(entry.metadata), PHAR_GET_16(zipentry.comment_len) TSRMLS_CC) == FAILURE) {
				entry.metadata_len = 0;
				/* if not valid serialized data, it is a regular string */
				if (entry.is_persistent) {
					ALLOC_PERMANENT_ZVAL(entry.metadata);
				} else {
					ALLOC_ZVAL(entry.metadata);
				}
				INIT_ZVAL(*entry.metadata);
				ZVAL_STRINGL(entry.metadata, pestrndup(buf, PHAR_GET_16(zipentry.comment_len), entry.is_persistent), PHAR_GET_16(zipentry.comment_len), 0);
			}
		} else {
			entry.metadata = NULL;
		}
		if (!actual_alias && entry.filename_len == sizeof(".phar/alias.txt")-1 && !strncmp(entry.filename, ".phar/alias.txt", sizeof(".phar/alias.txt")-1)) {
			php_stream_filter *filter;
			off_t saveloc;

			/* archive alias found, seek to file contents, do not validate local header.  Potentially risky, but
			   not very. */
			saveloc = php_stream_tell(fp);
			php_stream_seek(fp, PHAR_GET_32(zipentry.offset) + sizeof(phar_zip_file_header) + entry.filename_len + PHAR_GET_16(zipentry.extra_len), SEEK_SET);
			mydata->alias_len = entry.uncompressed_filesize;
			if (entry.flags & PHAR_ENT_COMPRESSED_GZ) {
				filter = php_stream_filter_create("zlib.inflate", NULL, php_stream_is_persistent(fp) TSRMLS_CC);
				if (!filter) {
					pefree(entry.filename, entry.is_persistent);
					PHAR_ZIP_FAIL("unable to decompress alias, zlib filter creation failed");
				}
				php_stream_filter_append(&fp->readfilters, filter);
				if (!(entry.uncompressed_filesize = php_stream_copy_to_mem(fp, &actual_alias, entry.uncompressed_filesize, 0)) || !actual_alias) {
					pefree(entry.filename, entry.is_persistent);
					PHAR_ZIP_FAIL("unable to read in alias, truncated");
				}
				php_stream_filter_flush(filter, 1);
				php_stream_filter_remove(filter, 1 TSRMLS_CC);
			} else if (entry.flags & PHAR_ENT_COMPRESSED_BZ2) {
				php_stream_filter *filter;
				filter = php_stream_filter_create("bzip2.decompress", NULL, php_stream_is_persistent(fp) TSRMLS_CC);
				if (!filter) {
					pefree(entry.filename, entry.is_persistent);
					PHAR_ZIP_FAIL("unable to read in alias, bzip2 filter creation failed");
				}
				php_stream_filter_append(&fp->readfilters, filter);
				php_stream_filter_append(&fp->readfilters, filter);
				if (!(entry.uncompressed_filesize = php_stream_copy_to_mem(fp, &actual_alias, entry.uncompressed_filesize, 0)) || !actual_alias) {
					pefree(entry.filename, entry.is_persistent);
					PHAR_ZIP_FAIL("unable to read in alias, truncated");
				}
				php_stream_filter_flush(filter, 1);
				php_stream_filter_remove(filter, 1 TSRMLS_CC);
			} else {
				if (!(entry.uncompressed_filesize = php_stream_copy_to_mem(fp, &actual_alias, entry.uncompressed_filesize, 0)) || !actual_alias) {
					pefree(entry.filename, entry.is_persistent);
					PHAR_ZIP_FAIL("unable to read in alias, truncated");
				}
			}

			/* return to central directory parsing */
			php_stream_seek(fp, saveloc, SEEK_SET);
		}
		phar_set_inode(&entry TSRMLS_CC);
		zend_hash_add(&mydata->manifest, entry.filename, entry.filename_len, (void *)&entry,sizeof(phar_entry_info), NULL);
	}
	mydata->fp = fp;
	if (zend_hash_exists(&(mydata->manifest), ".phar/stub.php", sizeof(".phar/stub.php")-1)) {
		mydata->is_data = 0;
	} else {
		mydata->is_data = 1;
	}
	zend_hash_add(&(PHAR_GLOBALS->phar_fname_map), mydata->fname, fname_len, (void*)&mydata, sizeof(phar_archive_data*), NULL);
	if (actual_alias) {
		phar_archive_data **fd_ptr;

		if (!phar_validate_alias(actual_alias, mydata->alias_len)) {
			if (error) {
				spprintf(error, 4096, "phar error: invalid alias \"%s\" in zip-based phar \"%s\"", actual_alias, fname);
			}
			efree(actual_alias);
			zend_hash_del(&(PHAR_GLOBALS->phar_fname_map), mydata->fname, fname_len);
			return FAILURE;
		}
		mydata->is_temporary_alias = 0;
		if (SUCCESS == zend_hash_find(&(PHAR_GLOBALS->phar_alias_map), actual_alias, mydata->alias_len, (void **)&fd_ptr)) {
			if (SUCCESS != phar_free_alias(*fd_ptr, actual_alias, mydata->alias_len TSRMLS_CC)) {
				if (error) {
					spprintf(error, 4096, "phar error: Unable to add zip-based phar \"%s\" with implicit alias, alias is already in use", fname);
				}
				efree(actual_alias);
				zend_hash_del(&(PHAR_GLOBALS->phar_fname_map), mydata->fname, fname_len);
				return FAILURE;
			}
		}
		mydata->alias = entry.is_persistent ? pestrndup(actual_alias, mydata->alias_len, 1) : actual_alias;
		if (entry.is_persistent) {
			efree(actual_alias);
		}
		zend_hash_add(&(PHAR_GLOBALS->phar_alias_map), actual_alias, mydata->alias_len, (void*)&mydata, sizeof(phar_archive_data*), NULL);
	} else {
		phar_archive_data **fd_ptr;

		if (alias_len) {
			if (SUCCESS == zend_hash_find(&(PHAR_GLOBALS->phar_alias_map), alias, alias_len, (void **)&fd_ptr)) {
				if (SUCCESS != phar_free_alias(*fd_ptr, alias, alias_len TSRMLS_CC)) {
					if (error) {
						spprintf(error, 4096, "phar error: Unable to add zip-based phar \"%s\" with explicit alias, alias is already in use", fname);
					}
					zend_hash_del(&(PHAR_GLOBALS->phar_fname_map), mydata->fname, fname_len);
					return FAILURE;
				}
			}
			zend_hash_add(&(PHAR_GLOBALS->phar_alias_map), actual_alias, mydata->alias_len, (void*)&mydata, sizeof(phar_archive_data*), NULL);
			mydata->alias = pestrndup(alias, alias_len, mydata->is_persistent);
			mydata->alias_len = alias_len;
		} else {
			mydata->alias = pestrndup(mydata->fname, fname_len, mydata->is_persistent);
			mydata->alias_len = fname_len;
		}
		mydata->is_temporary_alias = 1;
	}

	if (pphar) {
		*pphar = mydata;
	}
	return SUCCESS;
}
/* }}} */

/**
 * Create or open a zip-based phar for writing
 */
int phar_open_or_create_zip(char *fname, int fname_len, char *alias, int alias_len, int is_data, int options, phar_archive_data** pphar, char **error TSRMLS_DC) /* {{{ */
{
	phar_archive_data *phar;
	int ret = phar_create_or_parse_filename(fname, fname_len, alias, alias_len, is_data, options, &phar, error TSRMLS_CC);

	if (FAILURE == ret) {
		return FAILURE;
	}

	if (pphar) {
		*pphar = phar;
	}

	phar->is_data = is_data;

	if (phar->is_zip) {
		return ret;
	}

	if (phar->is_brandnew) {
		phar->internal_file_start = 0;
		phar->is_zip = 1;
		phar->is_tar = 0;
		return SUCCESS;
	}

	/* we've reached here - the phar exists and is a regular phar */
	if (error) {
		spprintf(error, 4096, "phar zip error: phar \"%s\" already exists as a regular phar and must be deleted from disk prior to creating as a zip-based phar", fname);
	}
	return FAILURE;
}
/* }}} */

struct _phar_zip_pass {
	php_stream *filefp;
	php_stream *centralfp;
	php_stream *old;
	int free_fp;
	int free_ufp;
	char **error;
};
/* perform final modification of zip contents for each file in the manifest before saving */
static int phar_zip_changed_apply(void *data, void *arg TSRMLS_DC) /* {{{ */
{
	phar_entry_info *entry;
	phar_zip_file_header local;
	phar_zip_unix3 perms;
	phar_zip_central_dir_file central;
	struct _phar_zip_pass *p;
	php_uint32 newcrc32;
	off_t offset;

	entry = (phar_entry_info *)data;
	p = (struct _phar_zip_pass*) arg;
	if (entry->is_mounted) {
		return ZEND_HASH_APPLY_KEEP;
	}
	if (entry->is_deleted) {
		if (entry->fp_refcount <= 0) {
			return ZEND_HASH_APPLY_REMOVE;
		} else {
			/* we can't delete this in-memory until it is closed */
			return ZEND_HASH_APPLY_KEEP;
		}
	}
	memset(&local, 0, sizeof(local));
	memset(&central, 0, sizeof(central));
	memset(&perms, 0, sizeof(perms));
	strncpy(local.signature, "PK\3\4", 4);
	strncpy(central.signature, "PK\1\2", 4);
	central.extra_len = local.extra_len = PHAR_SET_16(sizeof(perms));
	perms.tag[0] = 'n';
	perms.tag[1] = 'u';
	perms.size = PHAR_SET_16(sizeof(perms) - 4);
	perms.perms = PHAR_SET_16(entry->flags & PHAR_ENT_PERM_MASK);
	perms.crc32 = ~0;
	CRC32(perms.crc32, (char)perms.perms & 0xFF);
	CRC32(perms.crc32, (char)perms.perms & 0xFF00 >> 8);
	perms.crc32 = PHAR_SET_32(~(perms.crc32));
	if (entry->flags & PHAR_ENT_COMPRESSED_GZ) {
		local.compressed = central.compressed = PHAR_SET_16(PHAR_ZIP_COMP_DEFLATE);
	}
	if (entry->flags & PHAR_ENT_COMPRESSED_BZ2) {
		local.compressed = central.compressed = PHAR_SET_16(PHAR_ZIP_COMP_BZIP2);
	}
	/* do not use PHAR_SET_16 on either field of the next line */
	phar_zip_u2d_time(entry->timestamp, &local.timestamp, &local.datestamp);
	central.timestamp = local.timestamp;
	central.datestamp = local.datestamp;
	central.filename_len = local.filename_len = PHAR_SET_16(entry->filename_len + (entry->is_dir ? 1 : 0));
	central.offset = PHAR_SET_32(php_stream_tell(p->filefp));
	/* do extra field for perms later */
	if (entry->is_modified) {
		php_uint32 loc;
		php_stream_filter *filter;
		php_stream *efp;

		if (entry->is_dir) {
			entry->is_modified = 0;
			if (entry->fp_type == PHAR_MOD && entry->fp != entry->phar->fp && entry->fp != entry->phar->ufp) {
				php_stream_close(entry->fp);
				entry->fp = NULL;
				entry->fp_type = PHAR_FP;
			}
			goto continue_dir;
		}
		if (FAILURE == phar_open_entry_fp(entry, p->error, 0 TSRMLS_CC)) {
			spprintf(p->error, 0, "unable to open file contents of file \"%s\" in zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		if (-1 == phar_seek_efp(entry, 0, SEEK_SET, 0, 0 TSRMLS_CC)) {
			spprintf(p->error, 0, "unable to seek to start of file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		efp = phar_get_efp(entry, 0 TSRMLS_CC);

		newcrc32 = ~0;
		for (loc = 0;loc < entry->uncompressed_filesize; ++loc) {
			CRC32(newcrc32, php_stream_getc(efp));
		}
		entry->crc32 = ~newcrc32;
		central.uncompsize = local.uncompsize = PHAR_SET_32(entry->uncompressed_filesize);
		if (!(entry->flags & PHAR_ENT_COMPRESSION_MASK)) {
			/* not compressed */
			entry->compressed_filesize = entry->uncompressed_filesize;
			central.compsize = local.compsize = PHAR_SET_32(entry->compressed_filesize);
			goto not_compressed;
		}
		filter = php_stream_filter_create(phar_compress_filter(entry, 0), NULL, 0 TSRMLS_CC);
		if (!filter) {
			if (entry->flags & PHAR_ENT_COMPRESSED_GZ) {
				spprintf(p->error, 0, "unable to gzip compress file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			} else {
				spprintf(p->error, 0, "unable to bzip2 compress file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			}
			return ZEND_HASH_APPLY_STOP;
		}

		/* create new file that holds the compressed version */
		/* work around inability to specify freedom in write and strictness
		in read count */
		entry->cfp = php_stream_fopen_tmpfile();
		if (!entry->cfp) {
			spprintf(p->error, 0, "unable to create temporary file for file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		php_stream_flush(efp);
		if (-1 == phar_seek_efp(entry, 0, SEEK_SET, 0, 0 TSRMLS_CC)) {
			spprintf(p->error, 0, "unable to seek to start of file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		php_stream_filter_append((&entry->cfp->writefilters), filter);
		if (entry->uncompressed_filesize != php_stream_copy_to_stream(efp, entry->cfp, entry->uncompressed_filesize)) {
			spprintf(p->error, 0, "unable to copy compressed file contents of file \"%s\" while creating new phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		php_stream_filter_flush(filter, 1);
		php_stream_flush(entry->cfp);
		php_stream_filter_remove(filter, 1 TSRMLS_CC);
		php_stream_seek(entry->cfp, 0, SEEK_END);
		entry->compressed_filesize = (php_uint32) php_stream_tell(entry->cfp);
		central.compsize = local.compsize = PHAR_SET_32(entry->compressed_filesize);
		/* generate crc on compressed file */
		php_stream_rewind(entry->cfp);
		entry->old_flags = entry->flags;
		entry->is_modified = 1;
	} else {
		central.uncompsize = local.uncompsize = PHAR_SET_32(entry->uncompressed_filesize);
		central.compsize = local.compsize = PHAR_SET_32(entry->compressed_filesize);
		if (-1 == php_stream_seek(p->old, entry->offset_abs, SEEK_SET)) {
			spprintf(p->error, 0, "unable to seek to start of file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
	}
not_compressed:
	central.crc32 = local.crc32 = PHAR_SET_32(entry->crc32);
continue_dir:
	/* set file metadata */
	if (entry->metadata) {
		php_serialize_data_t metadata_hash;

		if (entry->metadata_str.c) {
			smart_str_free(&entry->metadata_str);
		}
		entry->metadata_str.c = 0;
		entry->metadata_str.len = 0;
		PHP_VAR_SERIALIZE_INIT(metadata_hash);
		php_var_serialize(&entry->metadata_str, &entry->metadata, &metadata_hash TSRMLS_CC);
		PHP_VAR_SERIALIZE_DESTROY(metadata_hash);
		central.comment_len = PHAR_SET_16(entry->metadata_str.len);
	}
	entry->header_offset = php_stream_tell(p->filefp);
	offset = entry->header_offset + sizeof(local) + entry->filename_len + (entry->is_dir ? 1 : 0) + sizeof(perms);
	if (sizeof(local) != php_stream_write(p->filefp, (char *)&local, sizeof(local))) {
		spprintf(p->error, 0, "unable to write local file header of file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
		return ZEND_HASH_APPLY_STOP;
	}
	if (sizeof(central) != php_stream_write(p->centralfp, (char *)&central, sizeof(central))) {
		spprintf(p->error, 0, "unable to write central directory entry for file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
		return ZEND_HASH_APPLY_STOP;
	}
	if (entry->is_dir) {
		if (entry->filename_len != php_stream_write(p->filefp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to local directory entry for directory \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		if (1 != php_stream_write(p->filefp, "/", 1)) {
			spprintf(p->error, 0, "unable to write filename to local directory entry for directory \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		if (entry->filename_len != php_stream_write(p->centralfp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to central directory entry for directory \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		if (1 != php_stream_write(p->centralfp, "/", 1)) {
			spprintf(p->error, 0, "unable to write filename to central directory entry for directory \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
	} else {
		if (entry->filename_len != php_stream_write(p->filefp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to local directory entry for file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
		if (entry->filename_len != php_stream_write(p->centralfp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to central directory entry for file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
	}
	if (sizeof(perms) != php_stream_write(p->filefp, (char *)&perms, sizeof(perms))) {
		spprintf(p->error, 0, "unable to write local extra permissions file header of file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
		return ZEND_HASH_APPLY_STOP;
	}
	if (sizeof(perms) != php_stream_write(p->centralfp, (char *)&perms, sizeof(perms))) {
		spprintf(p->error, 0, "unable to write central extra permissions file header of file \"%s\" to zip-based phar \"%s\"", entry->filename, entry->phar->fname);
		return ZEND_HASH_APPLY_STOP;
	}
	if (entry->is_modified) {
		if (entry->cfp) {
			if (entry->compressed_filesize != php_stream_copy_to_stream(entry->cfp, p->filefp, entry->compressed_filesize)) {
				spprintf(p->error, 0, "unable to write compressed contents of file \"%s\" in zip-based phar \"%s\"", entry->filename, entry->phar->fname);
				return ZEND_HASH_APPLY_STOP;
			}
			php_stream_close(entry->cfp);
			entry->cfp = NULL;
		} else {
			if (FAILURE == phar_open_entry_fp(entry, p->error, 0 TSRMLS_CC)) {
				return ZEND_HASH_APPLY_STOP;
			}
			phar_seek_efp(entry, 0, SEEK_SET, 0, 0 TSRMLS_CC);
			if (entry->uncompressed_filesize != php_stream_copy_to_stream(phar_get_efp(entry, 0 TSRMLS_CC), p->filefp, entry->uncompressed_filesize)) {
				spprintf(p->error, 0, "unable to write contents of file \"%s\" in zip-based phar \"%s\"", entry->filename, entry->phar->fname);
				return ZEND_HASH_APPLY_STOP;
			}
		}
		if (entry->fp_type == PHAR_MOD && entry->fp != entry->phar->fp && entry->fp != entry->phar->ufp && entry->fp_refcount == 0) {
			php_stream_close(entry->fp);
		}
		entry->is_modified = 0;
	} else {
		if (entry->fp_refcount) {
			/* open file pointers refer to this fp, do not free the stream */
			switch (entry->fp_type) {
				case PHAR_FP:
					p->free_fp = 0;
					break;
				case PHAR_UFP:
					p->free_ufp = 0;
				default:
					break;
			}
		}
		if (!entry->is_dir && entry->compressed_filesize && entry->compressed_filesize != php_stream_copy_to_stream(p->old, p->filefp, entry->compressed_filesize)) {
			spprintf(p->error, 0, "unable to copy contents of file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			return ZEND_HASH_APPLY_STOP;
		}
	}
	entry->fp = NULL;
	entry->offset = entry->offset_abs = offset;
	entry->fp_type = PHAR_FP;
	if (entry->metadata_str.c) {
		if (entry->metadata_str.len != php_stream_write(p->centralfp, entry->metadata_str.c, entry->metadata_str.len)) {
			spprintf(p->error, 0, "unable to write metadata as file comment for file \"%s\" while creating zip-based phar \"%s\"", entry->filename, entry->phar->fname);
			smart_str_free(&entry->metadata_str);
			return ZEND_HASH_APPLY_STOP;
		}
		smart_str_free(&entry->metadata_str);
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

int phar_zip_flush(phar_archive_data *phar, char *user_stub, long len, int defaultstub, char **error TSRMLS_DC) /* {{{ */
{
	char *pos;
	smart_str main_metadata_str = {0};
	static const char newstub[] = "<?php // zip-based phar archive stub file\n__HALT_COMPILER();";
	php_stream *stubfile, *oldfile;
	php_serialize_data_t metadata_hash;
	int free_user_stub, closeoldfile = 0;
	phar_entry_info entry = {0};
	char *temperr = NULL;
	struct _phar_zip_pass pass;
	phar_zip_dir_end eocd;

	pass.error = &temperr;
	entry.flags = PHAR_ENT_PERM_DEF_FILE;
	entry.timestamp = time(NULL);
	entry.is_modified = 1;
	entry.is_zip = 1;
	entry.phar = phar;
	entry.fp_type = PHAR_MOD;

	if (phar->is_persistent) {
		if (error) {
			spprintf(error, 0, "internal error: attempt to flush cached zip-based phar \"%s\"", phar->fname);
		}
		return EOF;
	}
	if (phar->is_data) {
		goto nostub;
	}

	/* set alias */
	if (!phar->is_temporary_alias && phar->alias_len) {
		entry.fp = php_stream_fopen_tmpfile();
		if (phar->alias_len != (int)php_stream_write(entry.fp, phar->alias, phar->alias_len)) {
			if (error) {
				spprintf(error, 0, "unable to set alias in zip-based phar \"%s\"", phar->fname);
			}
			return EOF;
		}
		entry.uncompressed_filesize = entry.compressed_filesize = phar->alias_len;
		entry.filename = estrndup(".phar/alias.txt", sizeof(".phar/alias.txt")-1);
		entry.filename_len = sizeof(".phar/alias.txt")-1;
		if (SUCCESS != zend_hash_update(&phar->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(phar_entry_info), NULL)) {
			if (error) {
				spprintf(error, 0, "unable to set alias in zip-based phar \"%s\"", phar->fname);
			}
			return EOF;
		}
	} else {
		zend_hash_del(&phar->manifest, ".phar/alias.txt", sizeof(".phar/alias.txt")-1);
	}
	/* register alias */
	if (phar->alias_len) {
		if (FAILURE == phar_get_archive(&phar, phar->fname, phar->fname_len, phar->alias, phar->alias_len, error TSRMLS_CC)) {
			return EOF;
		}
	}

	/* set stub */
	if (user_stub && !defaultstub) {
		if (len < 0) {
			/* resource passed in */
			if (!(php_stream_from_zval_no_verify(stubfile, (zval **)user_stub))) {
				if (error) {
					spprintf(error, 0, "unable to access resource to copy stub to new zip-based phar \"%s\"", phar->fname);
				}
				return EOF;
			}
			if (len == -1) {
				len = PHP_STREAM_COPY_ALL;
			} else {
				len = -len;
			}
			user_stub = 0;
			if (!(len = php_stream_copy_to_mem(stubfile, &user_stub, len, 0)) || !user_stub) {
				if (error) {
					spprintf(error, 0, "unable to read resource to copy stub to new zip-based phar \"%s\"", phar->fname);
				}
				return EOF;
			}
			free_user_stub = 1;
		} else {
			free_user_stub = 0;
		}
		if ((pos = strstr(user_stub, "__HALT_COMPILER();")) == NULL)
		{
			if (error) {
				spprintf(error, 0, "illegal stub for zip-based phar \"%s\"", phar->fname);
			}
			if (free_user_stub) {
				efree(user_stub);
			}
			return EOF;
		}
		len = pos - user_stub + 18;
		entry.fp = php_stream_fopen_tmpfile();
		entry.uncompressed_filesize = len + 5;

		if ((size_t)len != php_stream_write(entry.fp, user_stub, len)
		||            5 != php_stream_write(entry.fp, " ?>\r\n", 5)) {
			if (error) {
				spprintf(error, 0, "unable to create stub from string in new zip-based phar \"%s\"", phar->fname);
			}
			if (free_user_stub) {
				efree(user_stub);
			}
			php_stream_close(entry.fp);
			return EOF;
		}
		entry.filename = estrndup(".phar/stub.php", sizeof(".phar/stub.php")-1);
		entry.filename_len = sizeof(".phar/stub.php")-1;
		if (SUCCESS != zend_hash_update(&phar->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(phar_entry_info), NULL)) {
			if (free_user_stub) {
				efree(user_stub);
			}
			if (error) {
				spprintf(error, 0, "unable to set stub in zip-based phar \"%s\"", phar->fname);
			}
			return EOF;
		}
		if (free_user_stub) {
			efree(user_stub);
		}
	} else {
		/* Either this is a brand new phar (add the stub), or the default stub is required (overwrite the stub) */
		entry.fp = php_stream_fopen_tmpfile();

		if (sizeof(newstub)-1 != php_stream_write(entry.fp, newstub, sizeof(newstub)-1)) {
			php_stream_close(entry.fp);
			if (error) {
				spprintf(error, 0, "unable to %s stub in%szip-based phar \"%s\", failed", user_stub ? "overwrite" : "create", user_stub ? " " : " new ", phar->fname);
			}
			return EOF;
		}

		entry.uncompressed_filesize = entry.compressed_filesize = sizeof(newstub) - 1;
		entry.filename = estrndup(".phar/stub.php", sizeof(".phar/stub.php")-1);
		entry.filename_len = sizeof(".phar/stub.php")-1;

		if (!defaultstub) {
			if (!zend_hash_exists(&phar->manifest, ".phar/stub.php", sizeof(".phar/stub.php")-1)) {
				if (SUCCESS != zend_hash_add(&phar->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(phar_entry_info), NULL)) {
					php_stream_close(entry.fp);
					efree(entry.filename);
					if (error) {
						spprintf(error, 0, "unable to create stub in zip-based phar \"%s\"", phar->fname);
					}
					return EOF;
				}
			} else {
				php_stream_close(entry.fp);
				efree(entry.filename);
			}
		} else {
			if (SUCCESS != zend_hash_update(&phar->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(phar_entry_info), NULL)) {
				php_stream_close(entry.fp);
				efree(entry.filename);
				if (error) {
					spprintf(error, 0, "unable to overwrite stub in zip-based phar \"%s\"", phar->fname);
				}
				return EOF;
			}
		}
	}

nostub:

	if (phar->fp && !phar->is_brandnew) {
		oldfile = phar->fp;
		closeoldfile = 0;
		php_stream_rewind(oldfile);
	} else {
		oldfile = php_stream_open_wrapper(phar->fname, "rb", 0, NULL);
		closeoldfile = oldfile != NULL;
	}

	/* save modified files to the zip */
	pass.old = oldfile;
	pass.filefp = php_stream_fopen_tmpfile();
	if (!pass.filefp) {
		if (closeoldfile) {
			php_stream_close(oldfile);
		}
		if (error) {
			spprintf(error, 4096, "phar zip flush of \"%s\" failed: unable to open temporary file", phar->fname);
		}
		return EOF;
	}
	pass.centralfp = php_stream_fopen_tmpfile();
	if (!pass.centralfp) {
		if (closeoldfile) {
			php_stream_close(oldfile);
		}
		if (error) {
			spprintf(error, 4096, "phar zip flush of \"%s\" failed: unable to open temporary file", phar->fname);
		}
		return EOF;
	}
	pass.free_fp = pass.free_ufp = 1;
	memset(&eocd, 0, sizeof(eocd));

	strncpy(eocd.signature, "PK\5\6", 4);
	eocd.counthere = eocd.count = zend_hash_num_elements(&phar->manifest);
	zend_hash_apply_with_argument(&phar->manifest, phar_zip_changed_apply, (void *) &pass TSRMLS_CC);
	if (temperr) {
		php_stream_close(pass.filefp);
		php_stream_close(pass.centralfp);
		if (closeoldfile) {
			php_stream_close(oldfile);
		}
		if (error) {
			spprintf(error, 4096, "phar zip flush of \"%s\" failed: %s", phar->fname, temperr);
		}
		efree(temperr);
		return EOF;
	}

	/* save zip */
	eocd.cdir_size = php_stream_tell(pass.centralfp);
	eocd.cdir_offset = php_stream_tell(pass.filefp);
	php_stream_seek(pass.centralfp, 0, SEEK_SET);
	if (eocd.cdir_size != php_stream_copy_to_stream(pass.centralfp, pass.filefp, PHP_STREAM_COPY_ALL)) {
		php_stream_close(pass.filefp);
		php_stream_close(pass.centralfp);
		if (error) {
			spprintf(error, 4096, "phar zip flush of \"%s\" failed: unable to write central-directory", phar->fname);
		}
		if (closeoldfile) {
			php_stream_close(oldfile);
		}
		return EOF;
	}
	php_stream_close(pass.centralfp);
	if (phar->metadata) {
		/* set phar metadata */
		PHP_VAR_SERIALIZE_INIT(metadata_hash);
		php_var_serialize(&main_metadata_str, &phar->metadata, &metadata_hash TSRMLS_CC);
		PHP_VAR_SERIALIZE_DESTROY(metadata_hash);
		eocd.comment_len = PHAR_SET_16(main_metadata_str.len);
		if (sizeof(eocd) != php_stream_write(pass.filefp, (char *)&eocd, sizeof(eocd))) {
			php_stream_close(pass.filefp);
			if (error) {
				spprintf(error, 4096, "phar zip flush of \"%s\" failed: unable to write end of central-directory", phar->fname);
			}
			if (closeoldfile) {
				php_stream_close(oldfile);
			}
			smart_str_free(&main_metadata_str);
			return EOF;
		}
		if (main_metadata_str.len != php_stream_write(pass.filefp, main_metadata_str.c, main_metadata_str.len)) {
			php_stream_close(pass.filefp);
			if (error) {
				spprintf(error, 4096, "phar zip flush of \"%s\" failed: unable to write metadata to zip comment", phar->fname);
			}
			if (closeoldfile) {
				php_stream_close(oldfile);
			}
			smart_str_free(&main_metadata_str);
			return EOF;
		}
		smart_str_free(&main_metadata_str);
	} else {
		if (sizeof(eocd) != php_stream_write(pass.filefp, (char *)&eocd, sizeof(eocd))) {
			php_stream_close(pass.filefp);
			if (error) {
				spprintf(error, 4096, "phar zip flush of \"%s\" failed: unable to write end of central-directory", phar->fname);
			}
			if (closeoldfile) {
				php_stream_close(oldfile);
			}
			return EOF;
		}
	}
	if (phar->fp && pass.free_fp) {
		php_stream_close(phar->fp);
	}
	if (phar->ufp) {
		if (pass.free_ufp) {
			php_stream_close(phar->ufp);
		}
		phar->ufp = NULL;
	}
	/* re-open */
	phar->is_brandnew = 0;
	if (phar->donotflush) {
		/* deferred flush */
		phar->fp = pass.filefp;
	} else {
		phar->fp = php_stream_open_wrapper(phar->fname, "w+b", IGNORE_URL|STREAM_MUST_SEEK|REPORT_ERRORS, NULL);
		if (!phar->fp) {
			if (closeoldfile) {
				php_stream_close(oldfile);
			}
			phar->fp = pass.filefp;
			if (error) {
				spprintf(error, 4096, "unable to open new phar \"%s\" for writing", phar->fname);
			}
			return EOF;
		}
		php_stream_rewind(pass.filefp);
		php_stream_copy_to_stream(pass.filefp, phar->fp, PHP_STREAM_COPY_ALL);
		/* we could also reopen the file in "rb" mode but there is no need for that */
		php_stream_close(pass.filefp);
	}

	if (closeoldfile) {
		php_stream_close(oldfile);
	}
	return EOF;
}
/* }}} */
