#ifndef PHP_COMPAT_H
#define PHP_COMPAT_H

#ifdef PHP_WIN32
#include "config.w32.h"
#else
#include "php_config.h"
#endif

#if defined(HAVE_BUNDLED_PCRE) || !defined(PHP_VERSION)
#define pcre_compile 			php_pcre_compile
#define pcre_copy_substring		php_pcre_copy_substring
#define pcre_exec				php_pcre_exec
#define pcre_get_substring		php_pcre_substring
#define pcre_get_substring_list	php_pcre_get_substring_list
#define pcre_info				php_pcre_info
#define pcre_maketables			php_pcre_maketables
#define pcre_study				php_pcre_study
#define pcre_version			php_pcre_version
#endif

#define lookup				php_lookup
#define hashTableInit		php_hashTableInit
#define hashTableDestroy	php_hashTableDestroy
#define hashTableIterInit	php_hashTableIterInit
#define hashTableIterNext	php_hashTableIterNext
#define XML_DefaultCurrent php_XML_DefaultCurrent
#define XML_ErrorString php_XML_ErrorString
#define XML_ExternalEntityParserCreate php_XML_ExternalEntityParserCreate
#define XML_GetBase php_XML_GetBase
#define XML_GetBuffer php_XML_GetBuffer
#define XML_GetCurrentByteCount php_XML_GetCurrentByteCount
#define XML_GetCurrentByteIndex php_XML_GetCurrentByteIndex
#define XML_GetCurrentColumnNumber php_XML_GetCurrentColumnNumber
#define XML_GetCurrentLineNumber php_XML_GetCurrentLineNumber
#define XML_GetErrorCode php_XML_GetErrorCode
#define XML_GetSpecifiedAttributeCount php_XML_GetSpecifiedAttributeCount
#define XML_Parse php_XML_Parse
#define XML_ParseBuffer php_XML_ParseBuffer
#define XML_ParserCreate php_XML_ParserCreate
#define XML_ParserCreateNS php_XML_ParserCreateNS
#define XML_ParserFree php_XML_ParserFree
#define XML_SetBase php_XML_SetBase
#define XML_SetCdataSectionHandler php_XML_SetCdataSectionHandler
#define XML_SetCharacterDataHandler php_XML_SetCharacterDataHandler
#define XML_SetCommentHandler php_XML_SetCommentHandler
#define XML_SetDefaultHandler php_XML_SetDefaultHandler
#define XML_SetDefaultHandlerExpand php_XML_SetDefaultHandlerExpand
#define XML_SetElementHandler php_XML_SetElementHandler
#define XML_SetEncoding php_XML_SetEncoding
#define XML_SetExternalEntityRefHandler php_XML_SetExternalEntityRefHandler
#define XML_SetExternalEntityRefHandlerArg php_XML_SetExternalEntityRefHandlerArg
#define XML_SetNamespaceDeclHandler php_XML_SetNamespaceDeclHandler
#define XML_SetNotStandaloneHandler php_XML_SetNotStandaloneHandler
#define XML_SetNotationDeclHandler php_XML_SetNotationDeclHandler
#define XML_SetProcessingInstructionHandler php_XML_SetProcessingInstructionHandler
#define XML_SetUnknownEncodingHandler php_XML_SetUnknownEncodingHandler
#define XML_SetUnparsedEntityDeclHandler php_XML_SetUnparsedEntityDeclHandler
#define XML_SetUserData php_XML_SetUserData
#define XML_UseParserAsHandlerArg php_XML_UseParserAsHandlerArg
#define XmlGetUtf16InternalEncoding php_XmlGetUtf16InternalEncoding
#define XmlGetUtf8InternalEncoding php_XmlGetUtf8InternalEncoding
#define XmlInitEncoding php_XmlInitEncoding
#define XmlInitUnknownEncoding php_XmlInitUnknownEncoding
#define XmlParseXmlDecl php_XmlParseXmlDecl
#define XmlSizeOfUnknownEncoding php_XmlSizeOfUnknownEncoding
#define XmlUtf16Encode php_XmlUtf16Encode
#define XmlUtf8Encode php_XmlUtf8Encode
#define XmlPrologStateInit php_XmlPrologStateInit

#endif
