/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2002 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Uwe Steinmann <steinm@php.net>                               |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/* TODO
 * - Support Notation Nodes
 * */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_domxml.h"

#if HAVE_DOMXML
#include "ext/standard/info.h"
#define PHP_XPATH 1
#define PHP_XPTR 2

/* General macros used by domxml */
#define DOMXML_DOMOBJ_NEW(zval, obj, ret)			if (NULL == (zval = php_domobject_new(obj, ret TSRMLS_CC))) { \
														php_error(E_WARNING, "%s(): cannot create required DOM object", \
																  get_active_function_name(TSRMLS_C)); \
														RETURN_FALSE; \
													}

#define DOMXML_RET_ZVAL(zval)						SEPARATE_ZVAL(&zval); \
													*return_value = *zval; \
													FREE_ZVAL(zval);

#define DOMXML_RET_OBJ(zval, obj, ret)				DOMXML_DOMOBJ_NEW(zval, obj, ret); \
													DOMXML_RET_ZVAL(zval);

#define DOMXML_GET_THIS(zval)						if (NULL == (zval = getThis())) { \
														php_error(E_WARNING, "%s(): underlying object missing", \
																  get_active_function_name(TSRMLS_C)); \
														RETURN_FALSE; \
													}

#define DOMXML_GET_OBJ(ret, zval, le)				if (NULL == (ret = php_dom_get_object(zval, le, 0 TSRMLS_CC))) { \
														php_error(E_WARNING, "%s(): cannot fetch DOM object", \
																  get_active_function_name(TSRMLS_C)); \
														RETURN_FALSE; \
													}

#define DOMXML_GET_THIS_OBJ(ret, zval, le)			DOMXML_GET_THIS(zval); \
													DOMXML_GET_OBJ(ret, zval, le);

#define DOMXML_NO_ARGS()							if (ZEND_NUM_ARGS() != 0) { \
														php_error(E_WARNING, "%s() expects exactly 0 parameters, %d given", \
																  get_active_function_name(TSRMLS_C), ZEND_NUM_ARGS()); \
														return; \
													}

#define DOMXML_NOT_IMPLEMENTED()					php_error(E_WARNING, "%s() not yet implemented", \
															  get_active_function_name(TSRMLS_C)); \
													return;

/* WARNING: The number of parameters is actually the
 * number of passed variables to zend_parse_parameters(),
 * *NOT* the number of parameters expected by the PHP function. */
#define DOMXML_PARAM_NONE(ret, zval, le)			if (NULL == (zval = getThis())) { \
														if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zval) == FAILURE) { \
															return; \
														} \
													} \
													DOMXML_GET_OBJ(ret, zval, le);

#define DOMXML_PARAM_ONE(ret, zval, le, s, p1)	if (NULL == (zval = getThis())) { \
														if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o"s, &zval, p1) == FAILURE) { \
															return; \
														} \
													} else { \
														if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, s, p1) == FAILURE) { \
															return; \
														} \
													} \
													DOMXML_GET_OBJ(ret, zval, le);

#define DOMXML_PARAM_TWO(ret, zval, le, s, p1, p2)	if (NULL == (zval = getThis())) { \
														if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o"s, &zval, p1, p2) == FAILURE) { \
															return; \
														} \
													} else { \
														if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, s, p1, p2) == FAILURE) { \
															return; \
														} \
													} \
													DOMXML_GET_OBJ(ret, zval, le);

#define DOMXML_PARAM_THREE(ret, zval, le, s, p1, p2, p3)		if (NULL == (zval = getThis())) { \
																	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o"s, &zval, p1, p2, p3) == FAILURE) { \
																		return; \
																	} \
																} else { \
																	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, s, p1, p2, p3) == FAILURE) { \
																		return; \
																	} \
																} \
																DOMXML_GET_OBJ(ret, zval, le);


#define DOMXML_PARAM_FOUR(ret, zval, le, s, p1, p2, p3, p4)		if (NULL == (zval = getThis())) { \
																	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o"s, &zval, p1, p2, p3, p4) == FAILURE) { \
																		return; \
																	} \
																} else { \
																	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, s, p1, p2, p3, p4) == FAILURE) { \
																		return; \
																	} \
																} \
																DOMXML_GET_OBJ(ret, zval, le);


static int le_domxmldocp;
static int le_domxmldoctypep;
static int le_domxmldtdp;
static int le_domxmlnodep;
static int le_domxmlelementp;
static int le_domxmlattrp;
static int le_domxmlcdatap;
static int le_domxmltextp;
static int le_domxmlpip;
static int le_domxmlcommentp;
static int le_domxmlnotationp;
static int le_domxmlparserp;
/*static int le_domxmlentityp;*/
static int le_domxmlentityrefp;
/*static int le_domxmlnsp;*/


#if HAVE_DOMXSLT
static int le_domxsltstylesheetp;
#endif

#if defined(LIBXML_XPATH_ENABLED)
static int le_xpathctxp;
static int le_xpathobjectp;
#endif

zend_class_entry *domxmldoc_class_entry;
zend_class_entry *domxmldoctype_class_entry;
zend_class_entry *domxmlelement_class_entry;
zend_class_entry *domxmldtd_class_entry;
zend_class_entry *domxmlnode_class_entry;
zend_class_entry *domxmlattr_class_entry;
zend_class_entry *domxmlcdata_class_entry;
zend_class_entry *domxmltext_class_entry;
zend_class_entry *domxmlpi_class_entry;
zend_class_entry *domxmlcomment_class_entry;
zend_class_entry *domxmlnotation_class_entry;
zend_class_entry *domxmlentity_class_entry;
zend_class_entry *domxmlentityref_class_entry;
zend_class_entry *domxmlns_class_entry;
zend_class_entry *domxmlparser_class_entry;
#if defined(LIBXML_XPATH_ENABLED)
zend_class_entry *xpathctx_class_entry;
zend_class_entry *xpathobject_class_entry;
#endif
#if HAVE_DOMXSLT
zend_class_entry *domxsltstylesheet_class_entry;
#endif


static int node_attributes(zval **attributes, xmlNode *nodep TSRMLS_DC);
static int node_children(zval **children, xmlNode *nodep TSRMLS_DC);

static zend_function_entry domxml_functions[] = {
	PHP_FE(domxml_version,												NULL)
	PHP_FE(xmldoc,														NULL)
	PHP_FALIAS(domxml_open_mem,				xmldoc,	NULL)
	PHP_FE(xmldocfile,													NULL)
	PHP_FALIAS(domxml_open_file,				xmldocfile,	NULL)
#if defined(LIBXML_HTML_ENABLED)
	PHP_FE(html_doc,													NULL)
	PHP_FE(html_doc_file,												NULL)
#endif
	PHP_FE(domxml_xmltree,														NULL)
	PHP_FALIAS(xmltree,				domxml_xmltree,	NULL)
	PHP_FE(domxml_substitute_entities_default,														NULL)
	PHP_FE(domxml_add_root,												NULL)
	PHP_FE(domxml_dump_mem,												NULL)
	PHP_FE(domxml_dump_mem_file,										NULL)
	PHP_FE(domxml_dump_node,											NULL)
#if defined(LIBXML_HTML_ENABLED)
	PHP_FE(domxml_html_dump_mem,										NULL)
#endif
	PHP_FE(domxml_node_attributes,										NULL)
	PHP_FE(domxml_elem_get_attribute,									NULL)
	PHP_FE(domxml_elem_set_attribute,									NULL)
	PHP_FE(domxml_node_children,										NULL)
	PHP_FE(domxml_node_has_attributes,									NULL)
	PHP_FE(domxml_node_new_child,										NULL)
	PHP_FE(domxml_node,													NULL)
	PHP_FE(domxml_node_unlink_node,										NULL)
	PHP_FE(domxml_node_set_content,										NULL)
	PHP_FE(domxml_node_get_content,										NULL)
	PHP_FE(domxml_new_xmldoc,											NULL)
	PHP_FALIAS(domxml_new_doc,				domxml_new_xmldoc,	NULL)
	PHP_FE(domxml_parser,												NULL)
	PHP_FE(domxml_parser_add_chunk,										NULL)
	PHP_FE(domxml_parser_end,											NULL)
#if defined(LIBXML_XPATH_ENABLED)
	PHP_FE(xpath_new_context,											NULL)
	PHP_FE(xpath_eval,													NULL)
	PHP_FE(xpath_eval_expression,										NULL)
	PHP_FE(xpath_register_ns,											NULL)
	PHP_FE(domxml_doc_get_elements_by_tagname,							NULL)
	PHP_FE(domxml_doc_get_element_by_id,								NULL)
#endif
#if defined(LIBXML_XPTR_ENABLED)
	PHP_FE(xptr_new_context,											NULL)
	PHP_FE(xptr_eval,													NULL)
#endif
#if HAVE_DOMXSLT
	PHP_FE(domxml_xslt_version,											NULL)
	PHP_FE(domxml_xslt_stylesheet,										NULL)
	PHP_FE(domxml_xslt_stylesheet_doc,									NULL)
	PHP_FE(domxml_xslt_stylesheet_file,									NULL)
	PHP_FE(domxml_xslt_process,											NULL)
#endif

	PHP_FALIAS(domxml_root,				domxml_doc_document_element,	NULL)
	PHP_FALIAS(domxml_attributes,		domxml_node_attributes,			NULL)
	PHP_FALIAS(domxml_get_attribute,	domxml_elem_get_attribute,		NULL)
	PHP_FALIAS(domxml_getattr,			domxml_elem_get_attribute,		NULL)
	PHP_FALIAS(domxml_set_attribute,	domxml_elem_set_attribute,		NULL)
	PHP_FALIAS(domxml_setattr,			domxml_elem_set_attribute,		NULL)
	PHP_FALIAS(domxml_children,			domxml_node_children,			NULL)
	PHP_FALIAS(domxml_new_child,		domxml_node_new_child,			NULL)
	PHP_FALIAS(domxml_unlink_node,		domxml_node_unlink_node,		NULL)
	PHP_FALIAS(set_content,				domxml_node_set_content,		NULL)
	PHP_FALIAS(new_xmldoc,				domxml_new_xmldoc,				NULL)
	PHP_FALIAS(domxml_dumpmem,			domxml_dump_mem,				NULL)

	{NULL, NULL, NULL}
};


static function_entry php_domxmldoc_class_functions[] = {
	PHP_FALIAS(doctype, 				domxml_doc_doctype, 			NULL)
	PHP_FALIAS(implementation,			domxml_doc_implementation,		NULL)
	PHP_FALIAS(root,					domxml_doc_document_element,	NULL)	/* not DOM */
	PHP_FALIAS(document_element,		domxml_doc_document_element,	NULL)
	PHP_FALIAS(create_element,			domxml_doc_create_element,		NULL)
	PHP_FALIAS(create_text_node,		domxml_doc_create_text_node,	NULL)
	PHP_FALIAS(create_comment,			domxml_doc_create_comment,		NULL)
	PHP_FALIAS(create_attribute,		domxml_doc_create_attribute,	NULL)
	PHP_FALIAS(create_cdata_section,	domxml_doc_create_cdata_section,	NULL)
	PHP_FALIAS(create_entity_reference,	domxml_doc_create_entity_reference,	NULL)
	PHP_FALIAS(create_processing_instruction,	domxml_doc_create_processing_instruction,	NULL)
	PHP_FALIAS(get_elements_by_tagname,	domxml_doc_get_elements_by_tagname,	NULL)
	PHP_FALIAS(get_element_by_id,		domxml_doc_get_element_by_id,	NULL)
	/* children is deprecated because it is inherited from DomNode */
/*	PHP_FALIAS(children,				domxml_node_children,			NULL) */
	PHP_FALIAS(add_root,				domxml_add_root,				NULL)
	PHP_FALIAS(imported_node,			domxml_doc_imported_node,		NULL)
	PHP_FALIAS(dtd,						domxml_intdtd,					NULL)
	PHP_FALIAS(dumpmem,					domxml_dump_mem,				NULL)
	PHP_FALIAS(dump_mem,				domxml_dump_mem,				NULL)
	PHP_FALIAS(dump_mem_file,			domxml_dump_mem_file,			NULL)
	PHP_FALIAS(dump_file,			domxml_dump_mem_file,			NULL)
#if defined(LIBXML_HTML_ENABLED)
	PHP_FALIAS(html_dump_mem,			domxml_html_dump_mem,			NULL)
#endif
#if defined(LIBXML_XPATH_ENABLED)
	PHP_FALIAS(xpath_init,				xpath_init,						NULL)
	PHP_FALIAS(xpath_new_context,		xpath_new_context,				NULL)
	PHP_FALIAS(xptr_new_context,		xptr_new_context,				NULL)
#endif
	{NULL, NULL, NULL}
};

static function_entry php_domxmlparser_class_functions[] = {
	PHP_FALIAS(add_chunk,				domxml_parser_add_chunk,		NULL)
	PHP_FALIAS(end,						domxml_parser_end,				NULL)
	{NULL, NULL, NULL}
};

static function_entry php_domxmldoctype_class_functions[] = {
	PHP_FALIAS(name,					domxml_doctype_name,			NULL)
	PHP_FALIAS(entities,				domxml_doctype_entities,		NULL)
	PHP_FALIAS(notations,				domxml_doctype_notations,		NULL)
	PHP_FALIAS(system_id,				domxml_doctype_system_id,		NULL)
	PHP_FALIAS(public_id,				domxml_doctype_public_id,		NULL)
/*
	PHP_FALIAS(internal_subset,			domxml_doctype_internal_subset,	NULL)
*/
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmldtd_class_functions[] = {
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlnode_class_functions[] = {
	PHP_FALIAS(domnode,					domxml_node,					NULL)
	PHP_FALIAS(node_name,				domxml_node_name,				NULL)
	PHP_FALIAS(node_type,				domxml_node_type,				NULL)
	PHP_FALIAS(node_value,				domxml_node_value,				NULL)
	PHP_FALIAS(first_child,				domxml_node_first_child,		NULL)
	PHP_FALIAS(last_child,				domxml_node_last_child,			NULL)
	PHP_FALIAS(children,				domxml_node_children,			NULL)
	PHP_FALIAS(child_nodes,				domxml_node_children,			NULL)
	PHP_FALIAS(previous_sibling,		domxml_node_previous_sibling,	NULL)
	PHP_FALIAS(next_sibling,			domxml_node_next_sibling,		NULL)
	PHP_FALIAS(has_child_nodes,			domxml_node_has_child_nodes,	NULL)
	PHP_FALIAS(parent,					domxml_node_parent,				NULL)
	PHP_FALIAS(parent_node,				domxml_node_parent,				NULL)
	PHP_FALIAS(insert_before,			domxml_node_insert_before,		NULL)
	PHP_FALIAS(append_child,			domxml_node_append_child,		NULL)
	PHP_FALIAS(remove_child,			domxml_node_remove_child,		NULL)
	PHP_FALIAS(replace_child,			domxml_node_replace_child,		NULL)
	PHP_FALIAS(owner_document,			domxml_node_owner_document,		NULL)
	PHP_FALIAS(new_child,				domxml_node_new_child,			NULL)
	PHP_FALIAS(attributes,				domxml_node_attributes,			NULL)
	PHP_FALIAS(has_attributes,			domxml_node_has_attributes,		NULL)
	PHP_FALIAS(prefix,					domxml_node_prefix,				NULL)
	PHP_FALIAS(clone_node,				domxml_clone_node,				NULL)
/* Non DOM functions start here */
	PHP_FALIAS(add_child,				domxml_node_add_child,			NULL)
	PHP_FALIAS(append_sibling,			domxml_node_append_sibling,		NULL)
	PHP_FALIAS(node,					domxml_node,					NULL)
	PHP_FALIAS(unlink,					domxml_node_unlink_node,		NULL)
	PHP_FALIAS(unlink_node,				domxml_node_unlink_node,		NULL)
	PHP_FALIAS(replace_node,			domxml_node_replace_node,		NULL)
	PHP_FALIAS(set_content,				domxml_node_set_content,		NULL)
	PHP_FALIAS(get_content,				domxml_node_get_content,		NULL)
	PHP_FALIAS(text_concat,				domxml_node_text_concat,		NULL)
	PHP_FALIAS(set_name,				domxml_node_set_name,			NULL)
	PHP_FALIAS(is_blank_node,			domxml_is_blank_node,			NULL)
	PHP_FALIAS(dump_node,				domxml_dump_node,				NULL)
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlelement_class_functions[] = {
	PHP_FALIAS(domelement,				domxml_element,					NULL)
	PHP_FALIAS(name,					domxml_elem_tagname,			NULL)
	PHP_FALIAS(tagname,					domxml_elem_tagname,			NULL)
	PHP_FALIAS(get_attribute,			domxml_elem_get_attribute,		NULL)
	PHP_FALIAS(set_attribute,			domxml_elem_set_attribute,		NULL)
	PHP_FALIAS(remove_attribute,		domxml_elem_remove_attribute,	NULL)
	PHP_FALIAS(get_attribute_node,		domxml_elem_get_attribute_node,	NULL)
	PHP_FALIAS(set_attribute_node,		domxml_elem_set_attribute_node,	NULL)
	PHP_FALIAS(get_elements_by_tagname,	domxml_elem_get_elements_by_tagname,	NULL)
	PHP_FALIAS(has_attribute,	domxml_elem_has_attribute,	NULL)
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlcdata_class_functions[] = {
	PHP_FALIAS(length,					domxml_cdata_length,			NULL)
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmltext_class_functions[] = {
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlcomment_class_functions[] = {
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlnotation_class_functions[] = {
	PHP_FALIAS(public_id,				domxml_notation_public_id,		NULL)
	PHP_FALIAS(system_id,				domxml_notation_system_id,		NULL)
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlentityref_class_functions[] = {
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlentity_class_functions[] = {
/*
	PHP_FALIAS(public_id,				domxml_entity_public_id,		NULL)
	PHP_FALIAS(system_id,				domxml_entity_system_id,		NULL)
	PHP_FALIAS(notation_name,			domxml_entity_notation_name,	NULL)
*/
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlpi_class_functions[] = {
	PHP_FALIAS(target,					domxml_pi_target,				NULL)
	PHP_FALIAS(data,					domxml_pi_data,					NULL)
	{NULL, NULL, NULL}
};

#if defined(LIBXML_XPATH_ENABLED)
static zend_function_entry php_xpathctx_class_functions[] = {
	PHP_FALIAS(xpath_eval,				xpath_eval,						NULL)
	PHP_FALIAS(xpath_eval_expression,	xpath_eval_expression,			NULL)
	PHP_FALIAS(xpath_register_ns,		xpath_register_ns,				NULL)
	{NULL, NULL, NULL}
};

static zend_function_entry php_xpathobject_class_functions[] = {
	{NULL, NULL, NULL}
};
#endif

static zend_function_entry php_domxmlattr_class_functions[] = {
	PHP_FALIAS(name,					domxml_attr_name,				NULL)
	PHP_FALIAS(value,					domxml_attr_value,				NULL)
	PHP_FALIAS(specified,				domxml_attr_specified,			NULL)
/*
	PHP_FALIAS(owner_element,			domxml_attr_owner_element,		NULL)
*/
	{NULL, NULL, NULL}
};

static zend_function_entry php_domxmlns_class_functions[] = {
	{NULL, NULL, NULL}
};

#if HAVE_DOMXSLT
static zend_function_entry php_domxsltstylesheet_class_functions[] = {
/* TODO: maybe some more methods? */
	PHP_FALIAS(process, 				domxml_xslt_process, 			NULL)
	{NULL, NULL, NULL}
};
#endif

zend_module_entry domxml_module_entry = {
	STANDARD_MODULE_HEADER,
	"domxml",
	domxml_functions,
	PHP_MINIT(domxml),
	NULL,
	PHP_RINIT(domxml),
	NULL,
	PHP_MINFO(domxml),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_DOMXML
ZEND_GET_MODULE(domxml)
#endif


static void dom_object_set_data(void *obj, zval *wrapper)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Adding %s to hash\n", tmp);
*/
	((xmlNodePtr) obj)->_private = wrapper;
}


static zval *dom_object_get_data(void *obj)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Trying getting %s from object ...", tmp);
	if(((xmlNodePtr) obj)->_private)
		fprintf(stderr, " found\n");
	else
		fprintf(stderr, " not found\n");
*/
	return ((zval *) (((xmlNodePtr) obj)->_private));
}


static inline void node_wrapper_dtor(xmlNodePtr node)
{
	zval *wrapper;

	/* FIXME: type check probably unnecessary here? */
	if (!node || Z_TYPE_P(node) == XML_DTD_NODE)
		return;

	wrapper = dom_object_get_data(node);

	if (wrapper)
		zval_ptr_dtor(&wrapper);

}


static inline void attr_list_wrapper_dtor(xmlAttrPtr attr)
{
	while (attr != NULL) {
		node_wrapper_dtor((xmlNodePtr) attr);
		attr = attr->next;
	}
}


static inline void node_list_wrapper_dtor(xmlNodePtr node)
{
	while (node != NULL) {
		node_list_wrapper_dtor(node->children);
		switch (node->type) {
			/* Skip property freeing for the following types */
			case XML_ATTRIBUTE_DECL:
			case XML_DTD_NODE:
			case XML_ENTITY_DECL:
				break;
			default:
				attr_list_wrapper_dtor(node->properties);
		}
		node_wrapper_dtor(node);
		node = node->next;
	}
}

static xmlNodeSetPtr php_get_elements_by_tagname(xmlNodePtr n, xmlChar* name)
{
	xmlNodeSetPtr rv = NULL;
	xmlNodePtr cld = NULL;

	if ( n != NULL && name != NULL ) {
		cld = n->children;
		while ( cld != NULL ) {
			if ( xmlStrcmp( name, cld->name ) == 0 ){
				if ( rv == NULL ) {
					rv = xmlXPathNodeSetCreate( cld ) ;
				}
				else {
					xmlXPathNodeSetAdd( rv, cld );
				}
			}
			cld = cld->next;
		}
	}
	return rv;
}

static void php_free_xml_doc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	xmlDoc *doc = (xmlDoc *) rsrc->ptr;

	if (doc) {
		node_list_wrapper_dtor(doc->children);

		node_wrapper_dtor((xmlNodePtr) doc);
		xmlFreeDoc(doc);
	}
}


static void php_free_xml_node(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	xmlNodePtr node = (xmlNodePtr) rsrc->ptr;

	if (node) {
		zval *wrapper = dom_object_get_data(node);
		if (wrapper)
			zval_ptr_dtor(&wrapper);
	}
}


#if defined(LIBXML_XPATH_ENABLED)
static void php_free_xpath_context(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	xmlXPathContextPtr ctx = (xmlXPathContextPtr) rsrc->ptr;
	if (ctx) {
		if (ctx->user) {
			zval *wrapper = ctx->user;
			zval_ptr_dtor(&wrapper);
		}
		xmlXPathFreeContext(ctx);
	}
}

static void php_free_xpath_object(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	xmlXPathObjectPtr obj = (xmlXPathObjectPtr) rsrc->ptr;

	if (obj) {
		if (obj->user) {
			zval *wrapper = obj->user;
			zval_ptr_dtor(&wrapper);
		}
		xmlXPathFreeObject(obj);
	}
}
#endif

static void php_free_xml_parser(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	xmlParserCtxtPtr parser = (xmlParserCtxtPtr) rsrc->ptr;

	if (parser) {
		zval *wrapper = dom_object_get_data(parser);
		zval_ptr_dtor(&wrapper);        
		xmlFreeParserCtxt(parser);
	}
}


#if HAVE_DOMXSLT
static void php_free_xslt_stylesheet(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	xsltStylesheetPtr sheet = (xsltStylesheetPtr) rsrc->ptr;

	if (sheet) {
		node_wrapper_dtor((xmlNodePtr) sheet);
		xsltFreeStylesheet(sheet);
	}
}

static void xsltstylesheet_set_data(void *obj, zval *wrapper)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Adding %s to hash\n", tmp);
*/
	((xsltStylesheetPtr) obj)->_private = wrapper;
}


static zval *xsltstylesheet_get_data(void *obj)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Trying getting %s from object ...", tmp);
	if(((xmlNodePtr) obj)->_private)
		fprintf(stderr, " found\n");
	else
		fprintf(stderr, " not found\n");
*/
	return ((zval *) (((xsltStylesheetPtr) obj)->_private));
}

void *php_xsltstylesheet_get_object(zval *wrapper, int rsrc_type1, int rsrc_type2 TSRMLS_DC)
{
	void *obj;
	zval **handle;
	int type;

	if (NULL == wrapper) {
		php_error(E_WARNING, "xsltstylesheet_get_object() invalid wrapper object passed");
		return NULL;
	}

	if (Z_TYPE_P(wrapper) != IS_OBJECT) {
		php_error(E_WARNING, "%s(): wrapper is not an object", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	if (zend_hash_index_find(Z_OBJPROP_P(wrapper), 0, (void **) &handle) ==	FAILURE) {
		php_error(E_WARNING, "%s(): underlying object missing", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	obj = zend_list_find(Z_LVAL_PP(handle), &type);
	if (!obj || ((type != rsrc_type1) && (type != rsrc_type2))) {
		php_error(E_WARNING, "%s(): underlying object missing or of invalid type", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	return obj;
}

static void php_xsltstylesheet_set_object(zval *wrapper, void *obj, int rsrc_type)
{
	zval *handle, *addr;

	MAKE_STD_ZVAL(handle);
	Z_TYPE_P(handle) = IS_LONG;
	Z_LVAL_P(handle) = zend_list_insert(obj, rsrc_type);

	MAKE_STD_ZVAL(addr);
	Z_TYPE_P(addr) = IS_LONG;
	Z_LVAL_P(addr) = (int) obj;

	zend_hash_index_update(Z_OBJPROP_P(wrapper), 0, &handle, sizeof(zval *), NULL);
	zend_hash_index_update(Z_OBJPROP_P(wrapper), 1, &addr, sizeof(zval *), NULL);
	zval_add_ref(&wrapper);
	xsltstylesheet_set_data(obj, wrapper);
}
#endif  /* HAVE_DOMXSLT */


void *php_xpath_get_object(zval *wrapper, int rsrc_type1, int rsrc_type2 TSRMLS_DC)
{
	void *obj;
	zval **handle;
	int type;

	if (NULL == wrapper) {
		php_error(E_WARNING, "php_xpath_get_object() invalid wrapper object passed");
		return NULL;
	}

	if (Z_TYPE_P(wrapper) != IS_OBJECT) {
		php_error(E_WARNING, "%s(): wrapper is not an object", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	if (zend_hash_index_find(Z_OBJPROP_P(wrapper), 0, (void **) &handle) ==	FAILURE) {
		php_error(E_WARNING, "%s(): underlying object missing", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	obj = zend_list_find(Z_LVAL_PP(handle), &type);
	if (!obj || ((type != rsrc_type1) && (type != rsrc_type2))) {
		php_error(E_WARNING, "%s(): underlying object missing or of invalid type", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	return obj;
}


static void xpath_object_set_data(void *obj, zval *wrapper)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Adding %s to hash\n", tmp);
*/
	((xmlXPathObjectPtr) obj)->user = wrapper;
}


static zval *xpath_object_get_data(void *obj)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Trying getting %s from hash ...", tmp);
	if(((xmlXPathObjectPtr) obj)->user)
		fprintf(stderr, " found\n");
	else
		fprintf(stderr, " not found\n");
*/
	return ((zval *) (((xmlXPathObjectPtr) obj)->user));
}


static void php_xpath_set_object(zval *wrapper, void *obj, int rsrc_type)
{
	zval *handle, *addr;

	MAKE_STD_ZVAL(handle);
	Z_TYPE_P(handle) = IS_LONG;
	Z_LVAL_P(handle) = zend_list_insert(obj, rsrc_type);

	MAKE_STD_ZVAL(addr);
	Z_TYPE_P(addr) = IS_LONG;
	Z_LVAL_P(addr) = (int) obj;

	zend_hash_index_update(Z_OBJPROP_P(wrapper), 0, &handle, sizeof(zval *), NULL);
	zend_hash_index_update(Z_OBJPROP_P(wrapper), 1, &addr, sizeof(zval *), NULL);
	zval_add_ref(&wrapper);
	xpath_object_set_data(obj, wrapper);
}

static zval *php_xpathobject_new(xmlXPathObjectPtr obj, int *found TSRMLS_DC)
{
	zval *wrapper;

		*found = 0;

	if (!obj) {
		MAKE_STD_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((wrapper = (zval *) xpath_object_get_data((void *) obj))) {
		zval_add_ref(&wrapper);
		*found = 1;
		return wrapper;
	}

	MAKE_STD_ZVAL(wrapper);
	object_init_ex(wrapper, xpathobject_class_entry);

/*
	rsrc_type = le_xpathobjectp;
	php_xpath_set_object(wrapper, (void *) obj, rsrc_type);
*/

	php_xpath_set_object(wrapper, (void *) obj, le_xpathobjectp);

	return (wrapper);
}

void *php_xpath_get_context(zval *wrapper, int rsrc_type1, int rsrc_type2 TSRMLS_DC)
{
	void *obj;
	zval **handle;
	int type;

	if (NULL == wrapper) {
		php_error(E_WARNING, "php_xpath_get_context() invalid wrapper object passed");
		return NULL;
	}

	if (Z_TYPE_P(wrapper) != IS_OBJECT) {
		php_error(E_WARNING, "%s(): wrapper is not an object", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	if (zend_hash_index_find(Z_OBJPROP_P(wrapper), 0, (void **) &handle) ==
		FAILURE) {
		php_error(E_WARNING, "%s(): underlying object missing", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	obj = zend_list_find(Z_LVAL_PP(handle), &type);
	if (!obj || ((type != rsrc_type1) && (type != rsrc_type2))) {
		php_error(E_WARNING, "%s(): underlying object missing or of invalid type", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	return obj;
}

static void xpath_context_set_data(void *obj, zval *wrapper)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Adding %s to hash\n", tmp);
*/
	((xmlXPathContextPtr) obj)->user = (void *) wrapper;
}

static zval *xpath_context_get_data(void *obj)
{
/*
	char tmp[20];
	sprintf(tmp, "%08X", obj);
	fprintf(stderr, "Trying getting %s from hash ...", tmp);
	if(((xmlXPathContextPtr) obj)->user)
		fprintf(stderr, " found\n");
	else
		fprintf(stderr, " not found\n");
*/
	return ((zval *) (((xmlXPathContextPtr) obj)->user));
}

static void php_xpath_set_context(zval *wrapper, void *obj, int rsrc_type)
{
	zval *handle, *addr;

	MAKE_STD_ZVAL(handle);
	Z_TYPE_P(handle) = IS_LONG;
	Z_LVAL_P(handle) = zend_list_insert(obj, rsrc_type);

	MAKE_STD_ZVAL(addr);
	Z_TYPE_P(addr) = IS_LONG;
	Z_LVAL_P(addr) = (int) obj;

	zend_hash_index_update(Z_OBJPROP_P(wrapper), 0, &handle, sizeof(zval *), NULL);
	zend_hash_index_update(Z_OBJPROP_P(wrapper), 1, &addr, sizeof(zval *), NULL);
	zval_add_ref(&wrapper);
	xpath_context_set_data(obj, wrapper);
}

static zval *php_xpathcontext_new(xmlXPathContextPtr obj, int *found TSRMLS_DC)
{
	zval *wrapper;
	int rsrc_type;

		*found = 0;

	if (!obj) {
		MAKE_STD_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((wrapper = (zval *) xpath_context_get_data((void *) obj))) {
		zval_add_ref(&wrapper);
		*found = 1;
		return wrapper;
	}

	MAKE_STD_ZVAL(wrapper);
/*
	fprintf(stderr, "Adding new XPath Context\n");
*/
	object_init_ex(wrapper, xpathctx_class_entry);
	rsrc_type = le_xpathctxp;
	php_xpath_set_context(wrapper, (void *) obj, rsrc_type);

	return (wrapper);
}

/* helper functions for xmlparser stuff */
static void xmlparser_set_data(void *obj, zval *wrapper)
{
	((xmlParserCtxtPtr) obj)->_private = wrapper;
}


static void php_xmlparser_set_object(zval *wrapper, void *obj, int rsrc_type)
{
	zval *handle, *addr;

	MAKE_STD_ZVAL(handle);
	Z_TYPE_P(handle) = IS_LONG;
	Z_LVAL_P(handle) = zend_list_insert(obj, rsrc_type);

	MAKE_STD_ZVAL(addr);
	Z_TYPE_P(addr) = IS_LONG;
	Z_LVAL_P(addr) = (int) obj;

	zend_hash_index_update(Z_OBJPROP_P(wrapper), 0, &handle, sizeof(zval *), NULL);
	zend_hash_index_update(Z_OBJPROP_P(wrapper), 1, &addr, sizeof(zval *), NULL);
	zval_add_ref(&wrapper);
	xmlparser_set_data(obj, wrapper);
}


static zval *php_xmlparser_new(xmlParserCtxtPtr obj, int *found TSRMLS_DC)
{
	zval *wrapper;
	int rsrc_type;

		*found = 0;

	if (!obj) {
		MAKE_STD_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	MAKE_STD_ZVAL(wrapper);
	object_init_ex(wrapper, domxmlparser_class_entry);
	rsrc_type = le_domxmlparserp;
	php_xmlparser_set_object(wrapper, (void *) obj, rsrc_type);

	return (wrapper);
}


void *php_dom_get_object(zval *wrapper, int rsrc_type1, int rsrc_type2 TSRMLS_DC)
{
	void *obj;
	zval **handle;
	int type;

	if (NULL == wrapper) {
		php_error(E_WARNING, "php_dom_get_object() invalid wrapper object passed");
		return NULL;
	}

	if (Z_TYPE_P(wrapper) != IS_OBJECT) {
		php_error(E_WARNING, "%s(): wrapper is not an object", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	if (zend_hash_index_find(Z_OBJPROP_P(wrapper), 0, (void **) &handle) ==	FAILURE) {
		php_error(E_WARNING, "%s(): underlying object missing", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	obj = zend_list_find(Z_LVAL_PP(handle), &type);

/* The following test should be replaced with search in all parents */
	if (!obj) {		/* || ((type != rsrc_type1) && (type != rsrc_type2))) { */
		php_error(E_WARNING, "%s(): underlying object missing or of invalid type", get_active_function_name(TSRMLS_C));
		return NULL;
	}

	return obj;
}


static void php_dom_set_object(zval *wrapper, void *obj, int rsrc_type)
{
	zval *handle, *addr;

	MAKE_STD_ZVAL(handle);
	Z_TYPE_P(handle) = IS_LONG;
	Z_LVAL_P(handle) = zend_list_insert(obj, rsrc_type);

	MAKE_STD_ZVAL(addr);
	Z_TYPE_P(addr) = IS_LONG;
	Z_LVAL_P(addr) = (int) obj;

	zend_hash_index_update(Z_OBJPROP_P(wrapper), 0, &handle, sizeof(zval *), NULL);
	zend_hash_index_update(Z_OBJPROP_P(wrapper), 1, &addr, sizeof(zval *), NULL);
	zval_add_ref(&wrapper);
	dom_object_set_data(obj, wrapper);
}


static zval *php_domobject_new(xmlNodePtr obj, int *found TSRMLS_DC)
{
	zval *wrapper;
	char *content;
	int rsrc_type;

		*found = 0;

	if (!obj) {
		MAKE_STD_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((wrapper = (zval *) dom_object_get_data((void *) obj))) {
		zval_add_ref(&wrapper);
		*found = 1;
		return wrapper;
	}

	MAKE_STD_ZVAL(wrapper);

	switch (Z_TYPE_P(obj)) {

		case XML_ELEMENT_NODE:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmlelement_class_entry);
			rsrc_type = le_domxmlelementp;
			add_property_long(wrapper, "type", Z_TYPE_P(nodep));
			add_property_stringl(wrapper, "tagname", (char *) nodep->name, strlen(nodep->name), 1);
			break;
		}

		case XML_TEXT_NODE:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmltext_class_entry);
			rsrc_type = le_domxmltextp;
			content = xmlNodeGetContent(nodep);
			add_property_long(wrapper, "type", Z_TYPE_P(nodep));
			add_property_stringl(wrapper, "name", "#text", sizeof("#text"), 1);
			if (content)
				add_property_stringl(wrapper, "content", (char *) content, strlen(content), 1);
			xmlFree(content);
			break;
		}

		case XML_COMMENT_NODE:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmlcomment_class_entry);
			rsrc_type = le_domxmlcommentp;
			content = xmlNodeGetContent(nodep);
			if (content) {
				add_property_long(wrapper, "type", Z_TYPE_P(nodep));
				add_property_stringl(wrapper, "name", "#comment", sizeof("#comment"), 1);
				add_property_stringl(wrapper, "content", (char *) content, strlen(content), 1);
				xmlFree(content);
			}
			break;
		}

		case XML_PI_NODE:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmlpi_class_entry);
			rsrc_type = le_domxmlpip;
			content = xmlNodeGetContent(nodep);
			add_property_stringl(wrapper, "name", (char *) nodep->name, strlen(nodep->name), 1);
			if (content) {
				add_property_stringl(wrapper, "value", (char *) content, strlen(content), 1);
				xmlFree(content);
			}
			break;
		}

		case XML_ENTITY_REF_NODE:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmlentityref_class_entry);
			rsrc_type = le_domxmlentityrefp;
			add_property_stringl(wrapper, "name", (char *) nodep->name, strlen(nodep->name), 1);
			break;
		}

		case XML_ENTITY_DECL:
		case XML_ELEMENT_DECL:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmlnode_class_entry);
			rsrc_type = le_domxmlnodep;
			add_property_long(wrapper, "type", Z_TYPE_P(nodep));
			add_property_stringl(wrapper, "name", (char *) nodep->name, strlen(nodep->name), 1);
			if (Z_TYPE_P(obj) == XML_ENTITY_REF_NODE) {
				content = xmlNodeGetContent(nodep);
				if (content) {
					add_property_stringl(wrapper, "content", (char *) content, strlen(content), 1);
					xmlFree(content);
				}
			}
			break;
		}

		case XML_ATTRIBUTE_NODE:
		{
			xmlAttrPtr attrp = (xmlAttrPtr) obj;
			object_init_ex(wrapper, domxmlattr_class_entry);
			rsrc_type = le_domxmlattrp;
			add_property_long(wrapper, "type", Z_TYPE_P(attrp));
			add_property_stringl(wrapper, "name", (char *) attrp->name, strlen(attrp->name), 1);
			content = xmlNodeGetContent((xmlNodePtr) attrp);
			if (content) {
				add_property_stringl(wrapper, "value", (char *) content, strlen(content), 1);
				xmlFree(content);
			}
			break;
		}

		case XML_DOCUMENT_NODE:
		case XML_HTML_DOCUMENT_NODE:
		{
			xmlDocPtr docp = (xmlDocPtr) obj;

			object_init_ex(wrapper, domxmldoc_class_entry);
			rsrc_type = le_domxmldocp;
			if (docp->name)
				add_property_stringl(wrapper, "name", (char *) docp->name, strlen(docp->name), 1);
			else
				add_property_stringl(wrapper, "name", "#document", sizeof("#document"), 1);
			if (docp->URL)
				add_property_stringl(wrapper, "url", (char *) docp->URL, strlen(docp->URL), 1);
			else
				add_property_stringl(wrapper, "url", "", 0, 1);
			if (docp->version)
				add_property_stringl(wrapper, "version", (char *) docp->version, strlen(docp->version), 1);
				else
				add_property_stringl(wrapper, "version", "", 0, 1);
			if (docp->encoding)
				add_property_stringl(wrapper, "encoding", (char *) docp->encoding, strlen(docp->encoding), 1);
			add_property_long(wrapper, "standalone", docp->standalone);
			add_property_long(wrapper, "type", Z_TYPE_P(docp));
			add_property_long(wrapper, "compression", docp->compression);
			add_property_long(wrapper, "charset", docp->charset);
			break;
		}

		/* FIXME: nodes of type XML_DTD_NODE used to be domxmldtd_class_entry.
		 * but the DOM Standard doesn't have a DomDtd class. The DocumentType
		 * class seems to be want we need and the libxml dtd functions are
		 * very much like the methods of DocumentType. I wonder what exactly
		 * is the difference between XML_DTD_NODE and XML_DOCUMENT_TYPE_NODE.
		 * Something like
		 * <!DOCTYPE chapter SYSTEM '/share/sgml/Norman_Walsh/db3xml10/db3xml10.dtd'
		 * [ <!ENTITY sp \"spanish\">
		 * ]>
		 * is considered a DTD by libxml, but from the DOM perspective it
		 * rather is a DocumentType
		 */
		case XML_DTD_NODE:
		case XML_DOCUMENT_TYPE_NODE:
		{
			xmlDtdPtr dtd = (xmlDtdPtr) obj;
			object_init_ex(wrapper, domxmldoctype_class_entry);
/*			rsrc_type = le_domxmldtdp; */
			rsrc_type = le_domxmldoctypep;
/*			add_property_long(wrapper, "type", Z_TYPE_P(dtd)); */
			add_property_long(wrapper, "type", XML_DOCUMENT_TYPE_NODE);
			if (dtd->ExternalID)
				add_property_string(wrapper, "publicId", (char *) dtd->ExternalID, 1);
			else
				add_property_string(wrapper, "publicId", "", 1);
			if (dtd->SystemID)
				add_property_string(wrapper, "systemId", (char *) dtd->SystemID, 1);
			else
				add_property_string(wrapper, "systemId", "", 1);
			if (dtd->name)
				add_property_string(wrapper, "name", (char *) dtd->name, 1);
			break;
		}

		case XML_CDATA_SECTION_NODE:
		{
			xmlNodePtr nodep = obj;
			object_init_ex(wrapper, domxmlcdata_class_entry);
			rsrc_type = le_domxmlcdatap;
			content = xmlNodeGetContent(nodep);
			add_property_long(wrapper, "type", Z_TYPE_P(nodep));
			if (content) {
				add_property_stringl(wrapper, "content", (char *) content, strlen(content), 1);
				xmlFree(content);
			}
			break;
		}

		default:
			php_error(E_WARNING, "%s(): unsupported node type: %d\n", get_active_function_name(TSRMLS_C), Z_TYPE_P(obj));
			FREE_ZVAL(wrapper);
			return NULL;
	}

	php_dom_set_object(wrapper, (void *) obj, rsrc_type);
	return (wrapper);
}


static void domxml_error(void *ctx, const char *msg, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);
	php_error(E_WARNING, buf);
}


PHP_RINIT_FUNCTION(domxml)
{
	return SUCCESS;
}


PHP_MINIT_FUNCTION(domxml)
{
	zend_class_entry ce;

	le_domxmldocp =	zend_register_list_destructors_ex(php_free_xml_doc, NULL, "domdocument", module_number);
	/* Freeing the document contains freeing the complete tree.
	   Therefore nodes, attributes etc. may not be freed seperately.
	 */
	le_domxmlnodep = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domnode", module_number);
	le_domxmlcommentp = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domnode", module_number);
	le_domxmlattrp = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domattribute", module_number);
	le_domxmltextp = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domtext", module_number);
	le_domxmlelementp =	zend_register_list_destructors_ex(php_free_xml_node, NULL, "domelement", module_number);
	le_domxmldtdp = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domdtd", module_number);
	le_domxmlcdatap = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domcdata", module_number);
	le_domxmlentityrefp = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domentityref", module_number);
	le_domxmlpip = zend_register_list_destructors_ex(php_free_xml_node, NULL, "dompi", module_number);
	le_domxmlparserp =	zend_register_list_destructors_ex(php_free_xml_parser, NULL, "domparser", module_number);
	le_domxmldoctypep = zend_register_list_destructors_ex(php_free_xml_node, NULL, "domdocumenttype", module_number);
	/* Not yet initialized le_*s */
	le_domxmlnotationp  = -10003;

#if defined(LIBXML_XPATH_ENABLED)
	le_xpathctxp = zend_register_list_destructors_ex(php_free_xpath_context, NULL, "xpathcontext", module_number);
	le_xpathobjectp = zend_register_list_destructors_ex(php_free_xpath_object, NULL, "xpathobject", module_number);
#endif

/*	le_domxmlnsp = register_list_destructors(NULL, NULL); */

#if HAVE_DOMXSLT
	le_domxsltstylesheetp =	zend_register_list_destructors_ex(php_free_xslt_stylesheet, NULL, "xsltstylesheet", module_number);
#endif

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomNode", php_domxmlnode_class_functions, NULL, NULL, NULL);
	domxmlnode_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomDocument", php_domxmldoc_class_functions, NULL, NULL, NULL);
	domxmldoc_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

   	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomParser", php_domxmlparser_class_functions, NULL, NULL, NULL);
	domxmlparser_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomDocumentType", php_domxmldoctype_class_functions, NULL,	NULL, NULL);
	domxmldoctype_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "Dtd", php_domxmldtd_class_functions, NULL, NULL, NULL);
	domxmldtd_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomElement", php_domxmlelement_class_functions, NULL, NULL, NULL);
	domxmlelement_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomAttribute", php_domxmlattr_class_functions, NULL, NULL, NULL);
	domxmlattr_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomCData", php_domxmlcdata_class_functions, NULL, NULL, NULL);
	domxmlcdata_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomText", php_domxmltext_class_functions, NULL, NULL, NULL);
	domxmltext_class_entry = zend_register_internal_class_ex(&ce, domxmlcdata_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomComment", php_domxmlcomment_class_functions, NULL, NULL, NULL);
	domxmlcomment_class_entry = zend_register_internal_class_ex(&ce, domxmlcdata_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomProcessingInstruction", php_domxmlpi_class_functions, NULL, NULL, NULL);
	domxmlpi_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomNotation", php_domxmlnotation_class_functions, NULL, NULL, NULL);
	domxmlnotation_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomEntity", php_domxmlentity_class_functions, NULL, NULL, NULL);
	domxmlentity_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomEntityReference", php_domxmlentityref_class_functions, NULL, NULL, NULL);
	domxmlentityref_class_entry = zend_register_internal_class_ex(&ce, domxmlnode_class_entry, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "DomNamespace", php_domxmlns_class_functions, NULL, NULL, NULL);
	domxmlns_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

#if defined(LIBXML_XPATH_ENABLED)
	INIT_OVERLOADED_CLASS_ENTRY(ce, "XPathContext", php_xpathctx_class_functions, NULL, NULL, NULL);
	xpathctx_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "XPathObject", php_xpathobject_class_functions, NULL, NULL, NULL);
	xpathobject_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
#endif

#if HAVE_DOMXSLT
	INIT_OVERLOADED_CLASS_ENTRY(ce, "XsltStylesheet", php_domxsltstylesheet_class_functions, NULL, NULL, NULL);
	domxsltstylesheet_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
#endif

	REGISTER_LONG_CONSTANT("XML_ELEMENT_NODE",			XML_ELEMENT_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NODE",		XML_ATTRIBUTE_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_TEXT_NODE",				XML_TEXT_NODE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_CDATA_SECTION_NODE",	XML_CDATA_SECTION_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_REF_NODE",		XML_ENTITY_REF_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_NODE",			XML_ENTITY_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_PI_NODE",				XML_PI_NODE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_COMMENT_NODE",			XML_COMMENT_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_NODE",			XML_DOCUMENT_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_TYPE_NODE",	XML_DOCUMENT_TYPE_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_FRAG_NODE",	XML_DOCUMENT_FRAG_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_NOTATION_NODE",			XML_NOTATION_NODE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_HTML_DOCUMENT_NODE",	XML_HTML_DOCUMENT_NODE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DTD_NODE",				XML_DTD_NODE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ELEMENT_DECL_NODE", 	XML_ELEMENT_DECL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_DECL_NODE",	XML_ATTRIBUTE_DECL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_DECL_NODE",		XML_ENTITY_DECL,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_NAMESPACE_DECL_NODE",	XML_NAMESPACE_DECL,			CONST_CS | CONST_PERSISTENT);
#ifdef XML_GLOBAL_NAMESPACE
	REGISTER_LONG_CONSTANT("XML_GLOBAL_NAMESPACE",		XML_GLOBAL_NAMESPACE,		CONST_CS | CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("XML_LOCAL_NAMESPACE",		XML_LOCAL_NAMESPACE,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_CDATA",		XML_ATTRIBUTE_CDATA,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ID",			XML_ATTRIBUTE_ID,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_IDREF",		XML_ATTRIBUTE_IDREF,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_IDREFS",		XML_ATTRIBUTE_IDREFS,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ENTITY",		XML_ATTRIBUTE_ENTITIES,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NMTOKEN",		XML_ATTRIBUTE_NMTOKEN,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NMTOKENS",	XML_ATTRIBUTE_NMTOKENS,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ENUMERATION",	XML_ATTRIBUTE_ENUMERATION,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NOTATION",	XML_ATTRIBUTE_NOTATION,		CONST_CS | CONST_PERSISTENT);

#if defined(LIBXML_XPATH_ENABLED)
	REGISTER_LONG_CONSTANT("XPATH_UNDEFINED",			XPATH_UNDEFINED,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_NODESET",				XPATH_NODESET,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_BOOLEAN",				XPATH_BOOLEAN,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_NUMBER",				XPATH_NUMBER,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_STRING",				XPATH_STRING,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_POINT",				XPATH_POINT,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_RANGE",				XPATH_RANGE,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_LOCATIONSET",			XPATH_LOCATIONSET,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XPATH_USERS",				XPATH_USERS,				CONST_CS | CONST_PERSISTENT);
#endif

	xmlSetGenericErrorFunc(xmlGenericErrorContext, (xmlGenericErrorFunc)domxml_error);
#if HAVE_DOMXSLT
	xsltSetGenericErrorFunc(xsltGenericErrorContext, (xmlGenericErrorFunc)domxml_error);
#if HAVE_DOMEXSLT
	exsltRegisterAll();
#endif
#endif


	return SUCCESS;
}


/* {{{ proto int domxml_test(int id)
   Unity function for testing */
PHP_FUNCTION(domxml_test)
{
	zval *id;

	if ((ZEND_NUM_ARGS() != 1) || getParameters(ht, 1, &id) == FAILURE)
		WRONG_PARAM_COUNT;

	convert_to_long(id);
	RETURN_LONG(Z_LVAL_P(id));
}
/* }}} */


PHP_MINFO_FUNCTION(domxml)
{
	/* don't know why that line was commented out in the previous version, so i left it (cmv) */
	php_info_print_table_start();
	php_info_print_table_row(2, "DOM/XML", "enabled");
	php_info_print_table_row(2, "libxml Version", LIBXML_DOTTED_VERSION);
#if defined(LIBXML_HTML_ENABLED)
	php_info_print_table_row(2, "HTML Support", "enabled");
#endif
#if defined(LIBXML_XPATH_ENABLED)
	php_info_print_table_row(2, "XPath Support", "enabled");
#endif
#if defined(LIBXML_XPTR_ENABLED)
	php_info_print_table_row(2, "XPointer Support", "enabled");
#endif
#if HAVE_DOMXSLT
	php_info_print_table_row(2, "DOM/XSLT", "enabled");
	php_info_print_table_row(2, "libxslt Version", LIBXSLT_DOTTED_VERSION);
#if HAVE_DOMEXSLT
	php_info_print_table_row(2, "DOM/EXSLT", "enabled");
	php_info_print_table_row(2, "libexslt Version", LIBEXSLT_DOTTED_VERSION);
#endif
#endif

	php_info_print_table_end();
}

/* {{{ Methods of Class DomAttribute */

/* {{{ proto array domxml_attr_name(void)
   Returns list of attribute names 
   Notice: domxml_node_name() does exactly the same for attribute-nodes, 
           is this function here still needed, or would an alias be enough?
   */
PHP_FUNCTION(domxml_attr_name)
{
	zval *id;
	xmlAttrPtr attrp;

	DOMXML_GET_THIS_OBJ(attrp, id,le_domxmlattrp);

	DOMXML_NO_ARGS();

	RETURN_STRING((char *) (attrp->name), 1);
}
/* }}} */

/* {{{ proto array domxml_attr_value(void)
   Returns list of attribute names */
PHP_FUNCTION(domxml_attr_value)
{
	zval *id;
	xmlAttrPtr attrp;

	DOMXML_GET_THIS_OBJ(attrp, id, le_domxmlattrp);

	DOMXML_NO_ARGS();

	RETURN_STRING((char *) xmlNodeGetContent((xmlNodePtr) attrp), 1);
}
/* }}} */

/* {{{ proto array domxml_attr_specified(void)
   Returns list of attribute names */
PHP_FUNCTION(domxml_attr_specified)
{
	zval *id;
	xmlAttrPtr attrp;

	DOMXML_NOT_IMPLEMENTED();

	id = getThis();
	attrp = php_dom_get_object(id, le_domxmlattrp, 0 TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* End of Methods DomAttr }}} */


/* {{{ Methods of Class DomProcessingInstruction */

/* {{{ proto array domxml_pi_target(void)
   Returns target of pi */
PHP_FUNCTION(domxml_pi_target)
{
	zval *id;
	xmlNodePtr nodep;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlpip);

	DOMXML_NO_ARGS();

	RETURN_STRING((char *) nodep->name, 1);
}
/* }}} */

/* {{{ proto array domxml_pi_data(void)
   Returns data of pi */
PHP_FUNCTION(domxml_pi_data)
{
	zval *id;
	xmlNodePtr nodep;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlpip);

	DOMXML_NO_ARGS();

	RETURN_STRING(xmlNodeGetContent(nodep), 1);
}
/* }}} */

/* End of Methods of DomProcessingInstruction }}} */


/* {{{ Methods of Class DomCData */

/* {{{ proto array domxml_cdata_length(void)
   Returns list of attribute names */
PHP_FUNCTION(domxml_cdata_length)
{
	zval *id;
	xmlNodePtr nodep;

	DOMXML_NOT_IMPLEMENTED();

	id = getThis();
	nodep = php_dom_get_object(id, le_domxmlcdatap, 0 TSRMLS_CC);

	RETURN_LONG(1);
}
/* }}} */

/* End of Methods DomCDdata }}} */


/* {{{ Methods of Class DomNode */

/* {{{ proto object domxml_node(string name)
   Creates node */
PHP_FUNCTION(domxml_node)
{
	zval *rv;
	xmlNode *node;
	int ret, name_len;
	char *name;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len)  == FAILURE) {
		return;
	}

	node = xmlNewNode(NULL, name);
	if (!node) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_name(void)
   Returns name of node */
PHP_FUNCTION(domxml_node_name)
{
	zval *id;
	xmlNode *n;
	const char *str = NULL;

	DOMXML_GET_THIS_OBJ(n, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	switch (Z_TYPE_P(n)) {
		case XML_ELEMENT_NODE:
			str = n->name;
			break;

		case XML_TEXT_NODE:
			str = "#text";
			break;

		case XML_ATTRIBUTE_NODE:
			str = n->name;
			break;

		case XML_CDATA_SECTION_NODE:
			str = "#cdata-section";
			break;

		case XML_ENTITY_REF_NODE:
			str = n->name;
			break;

		case XML_ENTITY_NODE:
			str = NULL;
			break;

		case XML_PI_NODE:
			str = n->name;
			break;

		case XML_COMMENT_NODE:
			str = "#comment";
			break;

		case XML_DOCUMENT_NODE:
			str = "#document";
			break;

		case XML_DOCUMENT_FRAG_NODE:
			str = "#document-fragment";
			break;

		default:
			str = NULL;
			break;
	}

	if(str != NULL) {
		RETURN_STRING((char *) str, 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ proto object domxml_node_value(void)
   Returns name of value */
PHP_FUNCTION(domxml_node_value)
{
	zval *id;
	xmlNode *n;
	char *str = NULL;

	DOMXML_GET_THIS_OBJ(n, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	switch (Z_TYPE_P(n)) {
		case XML_TEXT_NODE:
		case XML_COMMENT_NODE:
		case XML_CDATA_SECTION_NODE:
		case XML_PI_NODE:
			str = n->content;
			break;
		default:
			str = NULL;
			break;
	}
	if(str != NULL) {
		RETURN_STRING((char *) str, 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ proto bool domxml_is_blank_node(void)
   Returns true if node is blank */
PHP_FUNCTION(domxml_is_blank_node)
{
	zval *id;
	xmlNode *n;

	DOMXML_GET_THIS_OBJ(n, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	if(xmlIsBlankNode(n)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int domxml_node_type(void)
   Returns the type of the node */
PHP_FUNCTION(domxml_node_type)
{
	zval *id;
	xmlNode *node;

	DOMXML_GET_THIS_OBJ(node, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	RETURN_LONG(node->type);
}
/* }}} */

/* {{{ proto bool domxml_clone_node(void)
   Clones a node */
PHP_FUNCTION(domxml_clone_node)
{
	zval *rv;
	zval *id;
	xmlNode *n, *node;
	int ret, recursive = 0;;

	DOMXML_GET_THIS_OBJ(n, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &recursive) == FAILURE) {
		return;
	}

	node = xmlCopyNode(n, recursive);
	if (!node) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_first_child(void)
   Returns first child from list of children */
PHP_FUNCTION(domxml_node_first_child)
{
	zval *id, *rv;
	xmlNode *nodep, *first;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	first = nodep->children;
	if (!first) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, first, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_last_child(void)
   Returns last child from list of children */
PHP_FUNCTION(domxml_node_last_child)
{
	zval *id, *rv;
	xmlNode *nodep, *last;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	last = nodep->last;
	if (!last) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, last, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_next_sibling(void)
   Returns next child from list of children */
PHP_FUNCTION(domxml_node_next_sibling)
{
	zval *id, *rv;
	xmlNode *nodep, *first;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	first = nodep->next;
	if (!first) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, first, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_previous_sibling(void)
   Returns previous child from list of children */
PHP_FUNCTION(domxml_node_previous_sibling)
{
	zval *id, *rv;
	xmlNode *nodep, *first;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	first = nodep->prev;
	if (!first) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, first, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_owner_document(void)
   Returns document this node belongs to */
PHP_FUNCTION(domxml_node_owner_document)
{
	zval *id, *rv;
	xmlNode *nodep;
	xmlDocPtr docp;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	docp = nodep->doc;
	if (!docp) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_has_child_nodes(void)
   Returns true if node has children */
PHP_FUNCTION(domxml_node_has_child_nodes)
{
	zval *id;
	xmlNode *nodep;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	if (nodep->children) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto object domxml_node_has_attributes(void)
   Returns true if node has attributes */
PHP_FUNCTION(domxml_node_has_attributes)
{
	zval *id;
	xmlNode *nodep;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	if (Z_TYPE_P(nodep) != XML_ELEMENT_NODE)
		RETURN_FALSE;

	if (nodep->properties) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string domxml_node_prefix(void)
   Returns namespace prefix of node */
PHP_FUNCTION(domxml_node_prefix)
{
	zval *id;
	xmlNode *nodep;
	xmlNsPtr ns;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	ns = nodep->ns;
	if (!ns) {
		RETURN_EMPTY_STRING();
	}

	if (ns->prefix) {
		RETURN_STRING((char *) (ns->prefix), 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ proto object domxml_node_parent(void)
   Returns parent of node */
PHP_FUNCTION(domxml_node_parent)
{
	zval *id, *rv;
	xmlNode *nodep, *last;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	DOMXML_NO_ARGS();

	last = nodep->parent;
	if (!last) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, last, &ret);
}
/* }}} */

/* {{{ proto array domxml_node_children(void)
   Returns list of children nodes */
PHP_FUNCTION(domxml_node_children)
{
	zval *id;
	xmlNode *nodep, *last;
	int ret;

	DOMXML_PARAM_NONE(nodep, id, le_domxmlnodep);

	/* Even if the nodep is a XML_DOCUMENT_NODE the type is at the
	   same position.
	 */
	if ((Z_TYPE_P(nodep) == XML_DOCUMENT_NODE) || (Z_TYPE_P(nodep) == XML_HTML_DOCUMENT_NODE))
		last = ((xmlDoc *) nodep)->children;
	else
		last = nodep->children;
	if (!last) {
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	while (last) {
		zval *child;
		child = php_domobject_new(last, &ret TSRMLS_CC);
		add_next_index_zval(return_value, child);
		last = last->next;
	}
}
/* }}} */

/* {{{ proto void domxml_node_unlink_node([object node])
   Deletes the node */
PHP_FUNCTION(domxml_node_unlink_node)
{
	zval *id;
	xmlNode *nodep;

	DOMXML_PARAM_NONE(nodep, id, le_domxmlnodep);

	xmlUnlinkNode(nodep);
	/*	This causes a Segmentation Fault for some reason.  Removing 
		it allows the user to re-add the node at some other time, in
		addition to fixing the segfault.  Node will be freed at
		shutdown. */
	/*xmlFreeNode(nodep);
	zval_dtor(id);*/			/* This is not enough because the children won't be deleted */
}
/* }}} */

/* {{{ proto object domxml_node_add_child(object domnode)
   Adds existing node to parent node */
PHP_FUNCTION(domxml_node_add_child)
{
	zval *id, *rv, *node;
	xmlNodePtr child, nodep, new_child;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(child, node, le_domxmlnodep);

	if (child->type == XML_ATTRIBUTE_NODE) {
		php_error(E_WARNING, "%s(): can't add attribute node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	if (NULL == (new_child = xmlCopyNode(child, 1))) {
		php_error(E_WARNING, "%s(): unable to clone node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	child = xmlAddChild(nodep, new_child);

	if (NULL == child) {
		php_error(E_WARNING, "%s(): couldn't add child", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, child, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_replace_node(object domnode)
   Replaces one node with another node */
PHP_FUNCTION(domxml_node_replace_node)
{
	zval *id, *rv, *node;
	xmlNodePtr repnode, nodep, new_repnode;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(repnode, node, le_domxmlnodep);

	if (NULL == (new_repnode = xmlCopyNode(repnode, 1))) {
		php_error(E_WARNING, "%s(): unable to clone node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	repnode = xmlReplaceNode(nodep, new_repnode);

	DOMXML_RET_OBJ(rv, nodep, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_append_child(object domnode)
   Adds node to list of children */
PHP_FUNCTION(domxml_node_append_child)
{
	zval *id, *rv, *node;
	xmlNodePtr child, nodep, new_child;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(child, node, le_domxmlnodep);

	if (child->type == XML_ATTRIBUTE_NODE) {
		php_error(E_WARNING, "%s(): can't append attribute node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	if (NULL == (new_child = xmlCopyNode(child, 1))) {
		php_error(E_WARNING, "%s(): unable to clone node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	/* FIXME reverted xmlAddChildList; crashes
	 * Uwe: must have been a temporary problem. It works for me with both
	 * xmlAddChildList and xmlAddChild
	 */
//	child = xmlAddSibling(nodep, new_child);
	child = xmlAddChild(nodep, new_child);

	if (NULL == child) {
		php_error(E_WARNING, "%s(): couldn't append node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, child, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_append_sibling(object domnode)
   Adds node to list of siblings */
PHP_FUNCTION(domxml_node_append_sibling)
{
	zval *id, *rv, *node;
	xmlNodePtr child, nodep, new_child;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(child, node, le_domxmlnodep);

	if (child->type == XML_ATTRIBUTE_NODE) {
		php_error(E_WARNING, "%s(): can't append attribute node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	if (NULL == (new_child = xmlCopyNode(child, 1))) {
		php_error(E_WARNING, "%s(): unable to clone node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	// FIXME reverted xmlAddChildList; crashes
	child = xmlAddSibling(nodep, new_child);

	if (NULL == child) {
		php_error(E_WARNING, "%s(): couldn't append node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, child, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_insert_before(object newnode, object refnode)
   Adds node in list of nodes before given node */
PHP_FUNCTION(domxml_node_insert_before)
{
	zval *id, *rv, *node, *ref;
	xmlNodePtr child, new_child, nodep, refp;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &node, &ref) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(child, node, le_domxmlnodep);
	DOMXML_GET_OBJ(refp, ref, le_domxmlnodep);

	if (NULL == (new_child = xmlCopyNode(child, 1))) {
		php_error(E_WARNING, "%s(): unable to clone node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	child = xmlAddPrevSibling(refp, new_child);

	if (NULL == child) {
		php_error(E_WARNING, "%s(): couldn't add newnode as the previous sibling of refnode", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, child, &ret);
}
/* }}} */

/* {{{ proto object domxml_node_remove_child(object domnode)
   Removes node from list of children */
PHP_FUNCTION(domxml_node_remove_child)
{
	zval *id, *node;
	xmlNodePtr children, child, nodep;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(child, node, le_domxmlnodep);

	children = nodep->children;
	if (!children) {
		RETURN_FALSE;
	}

	while (children) {
		if (children == child) {
			zval *rv;
			xmlUnlinkNode(child);
			DOMXML_RET_OBJ(rv, child, &ret);
			return;
		}
		children = children->next;
	}
	RETURN_FALSE
}
/* }}} */

/* {{{ proto object domxml_node_replace_child(object newnode, object oldnode)
   Replaces node in list of children */
PHP_FUNCTION(domxml_node_replace_child)
{
	zval *id, *newnode, *oldnode;
	xmlNodePtr children, newchild, oldchild, nodep;
	int ret;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &newnode, &oldnode) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(newchild, newnode, le_domxmlnodep);
	DOMXML_GET_OBJ(oldchild, oldnode, le_domxmlnodep);

	children = nodep->children;
	if (!children) {
		RETURN_FALSE;
	}

	while (children) {
		if (children == oldchild) {
			zval *rv;
			xmlNodePtr node;
			node = xmlReplaceNode(oldchild, newchild);
			DOMXML_RET_OBJ(rv, oldchild, &ret);
			return;
		}
		children = children->next;
	}
	RETURN_FALSE
}
/* }}} */

/* {{{ proto bool domxml_node_set_name(string name)
   Sets name of a node */
PHP_FUNCTION(domxml_node_set_name)
{
	zval *id;
	xmlNode *nodep;
	int name_len;
	char *name;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	xmlNodeSetName(nodep, name);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array domxml_node_attributes(void)
   Returns list of attributes of node */
PHP_FUNCTION(domxml_node_attributes)
{
	zval *id, *attrs;
	xmlNode *nodep;
#ifdef oldstyle_for_libxml_1_8_7
	xmlAttr *attr;
#endif

	DOMXML_PARAM_NONE(nodep, id, le_domxmlnodep);

	if (node_attributes(&attrs, nodep TSRMLS_CC) < 0)
		RETURN_FALSE;

	*return_value = *attrs;
	FREE_ZVAL(attrs);

#ifdef oldstyle_for_libxml_1_8_7
	attr = nodep->properties;
	if (!attr) {
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	while (attr) {
		add_assoc_string(return_value, (char *) attr->name, xmlNodeGetContent(attr), 1);
		attr = attr->next;
	}
#endif
}
/* }}} */

/* {{{ proto object domxml_node_new_child(string name, string content)
   Adds child node to parent node */
PHP_FUNCTION(domxml_node_new_child)
{
	zval *id, *rv;
	xmlNodePtr child, nodep;
	int ret, name_len, content_len;
	char *name, *content = NULL;

	DOMXML_PARAM_FOUR(nodep, id, le_domxmlnodep, "s|s", &name, &name_len, &content, &content_len);

	child = xmlNewChild(nodep, NULL, name, content);

	if (!child) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, child, &ret);
}
/* }}} */

/* {{{ proto bool domxml_node_set_content(string content)
   Sets content of a node */
PHP_FUNCTION(domxml_node_set_content)
{
	zval *id;
	xmlNode *nodep;
	int content_len;
	char *content;

	DOMXML_PARAM_TWO(nodep, id, le_domxmlnodep, "s", &content, &content_len);

	/* FIXME: another gotcha. If node has children, calling
	 * xmlNodeSetContent will remove the children -> we loose the zval's
	 * To prevent crash, append content if children are set
	 */
	if (nodep->children) {
		xmlNodeAddContentLen(nodep, content, content_len);
	} else {
		xmlNodeSetContentLen(nodep, content, content_len);
	}

	/* FIXME: Actually the property 'content' of the node has to be updated
	   as well. Since 'content' should disappear sooner or later and being
	   replaces by a function 'content()' I skip this for now
	 */	   
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string domxml_node_get_content()
   Gets content of a node.
   
   "Read the value of a node, this can be either the text carried directly by
this node if it's a TEXT node or the aggregate string of the values carried by
this node child's (TEXT and ENTITY_REF). Entity references are substituted."
   */
PHP_FUNCTION(domxml_node_get_content)
{
	zval *id;
	xmlNode *nodep;
	xmlChar *mem;

	DOMXML_PARAM_NONE(nodep, id, le_domxmlnodep);
	mem = xmlNodeGetContent(nodep);
	if (!mem) {
		RETURN_FALSE;
	}
		
	RETVAL_STRING(mem,1);
	xmlFree(mem);
}
/* }}} */


/* End of Methods DomNode }}} */


/* {{{ Methods of Class DomNotation */

/* {{{ proto string domxml_notation_public_id(void)
   Returns public id of notation node */
PHP_FUNCTION(domxml_notation_public_id)
{
	zval *id;
	xmlNotationPtr nodep;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnotationp);

	DOMXML_NO_ARGS();

	if(nodep->PublicID) {
		RETURN_STRING((char *) (nodep->PublicID), 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ proto string domxml_notation_system_id(void)
   Returns system ID of notation node */
PHP_FUNCTION(domxml_notation_system_id)
{
	zval *id;
	xmlNotationPtr nodep;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnotationp);

	DOMXML_NO_ARGS();

	if(nodep->SystemID) {
		RETURN_STRING((char *) (nodep->SystemID), 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* End of Methods DomNotation }}} */


/* {{{ Methods of Class DomElement */

/* {{{ proto object domxml_element(string name)
   Constructor of DomElement */
PHP_FUNCTION(domxml_element)
{
	zval *rv;
	xmlNode *node;
	int ret, name_len;
	char *name;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	node = xmlNewNode(NULL, name);
	if (!node) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, node, &ret);
}

/* }}} */

/* {{{ proto string domxml_elem_tagname(void)
   Returns tag name of element node */
PHP_FUNCTION(domxml_elem_tagname)
{
	zval *id;
	xmlNode *nodep;

	DOMXML_NO_ARGS();

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlelementp);

	DOMXML_NO_ARGS();

	RETURN_STRING((char *) (nodep->name), 1);
}
/* }}} */

/* {{{ proto string domxml_elem_get_attribute(string attrname)
   Returns value of given attribute */
PHP_FUNCTION(domxml_elem_get_attribute)
{
	zval *id;
	xmlNode *nodep;
	char *name, *value;
	int name_len;

	DOMXML_PARAM_TWO(nodep, id, le_domxmlelementp, "s", &name, &name_len);

	value = xmlGetProp(nodep, name);
	if (!value) {
		RETURN_FALSE;
	} else {
		RETVAL_STRING(value, 1);
		xmlFree(value);
	}
}
/* }}} */

/* {{{ proto bool domxml_elem_set_attribute(string attrname, string value)
   Sets value of given attribute */
PHP_FUNCTION(domxml_elem_set_attribute)
{
	zval *id, *rv;
	xmlNode *nodep;
	xmlAttr *attr;
	int ret, name_len, value_len;
	char *name, *value;

	DOMXML_PARAM_FOUR(nodep, id, le_domxmlelementp, "ss", &name, &name_len, &value, &value_len);

	attr = xmlSetProp(nodep, name, value);
	if (!attr) {
		php_error(E_WARNING, "%s(): no such attribute '%s'", get_active_function_name(TSRMLS_C), name);
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) attr, &ret);
}
/* }}} */

/* {{{ proto string domxml_elem_remove_attribute(string attrname)
   Removes given attribute */
PHP_FUNCTION(domxml_elem_remove_attribute)
{
	zval *id;
	xmlNode *nodep;
	xmlAttr *attrp;
	int name_len;
	char *name;
    
	DOMXML_PARAM_TWO(nodep, id, le_domxmlelementp, "s", &name, &name_len);
	attrp = xmlHasProp(nodep,name);
	if (attrp == NULL) {
		RETURN_FALSE;
	}
	xmlUnlinkNode((xmlNodePtr)attrp);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string domxml_elem_get_attribute_node(string attrname)
   Returns value of given attribute */
PHP_FUNCTION(domxml_elem_get_attribute_node)
{
	zval *id, *rv;
	xmlNode *nodep;
	xmlAttr  *attrp;
	int name_len, ret;
	char *name;

	DOMXML_PARAM_TWO(nodep, id, le_domxmlelementp, "s", &name, &name_len);
	attrp = xmlHasProp(nodep,name);
	if (attrp == NULL) {
		RETURN_FALSE;
	}
	DOMXML_RET_OBJ(rv, (xmlNodePtr) attrp, &ret);
}
/* }}} */

/* {{{ proto bool domxml_elem_set_attribute_node(object attr)
   Sets value of given attribute */
PHP_FUNCTION(domxml_elem_set_attribute_node)
{
	zval *id, *arg1, *rv;
	xmlNode *nodep;
	xmlAttr *attrp, *newattrp;
	int ret;

	if ((ZEND_NUM_ARGS() == 1) && getParameters(ht, 1, &arg1) == SUCCESS) {
		id = getThis();
		nodep = php_dom_get_object(id, le_domxmlelementp, 0 TSRMLS_CC);
		attrp = php_dom_get_object(arg1, le_domxmlattrp, 0 TSRMLS_CC);
	} else {
		WRONG_PARAM_COUNT;
	}

	/* FIXME: The following line doesn't work */
	newattrp = xmlCopyProp(nodep, attrp);
	if (!newattrp) {
		php_error(E_WARNING, "%s(): no such attribute '%s'", get_active_function_name(TSRMLS_C), attrp->name);
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) newattrp, &ret);
}
/* }}} */

/* {{{ proto string domxml_elem_has_attribute(string attrname)
   Checks for existenz given attribute */
PHP_FUNCTION(domxml_elem_has_attribute)
{
	zval *id;
	xmlNode *nodep;
	char *name, *value;
	int name_len;

	DOMXML_PARAM_TWO(nodep, id, le_domxmlelementp, "s", &name, &name_len);

	value = xmlGetProp(nodep, name);
	if (!value) {
		RETURN_FALSE;
	} else {
		xmlFree(value);
		RETURN_TRUE;
	}
}
/* }}} */

#if defined(LIBXML_XPATH_ENABLED)
/* {{{ proto string domxml_doc_get_elements_by_tagname(string tagname [,object xpathctx_handle] )
   Returns array with nodes with given tagname in document or empty array, if not found*/
PHP_FUNCTION(domxml_doc_get_elements_by_tagname)
{
	zval *id, *rv, *contextnode = NULL,*ctxpin = NULL;
	xmlXPathContextPtr ctxp;
	xmlDocPtr docp;

	xmlXPathObjectPtr xpathobjp;
	xmlNode *contextnodep;
	int name_len;
	char *str,*name;

	contextnode = NULL;
	contextnodep = NULL;

	DOMXML_PARAM_FOUR(docp, id, le_domxmldocp, "s|oo", &name, &name_len,&ctxpin,&contextnodep);
	
	/* if no xpath_context was submitted, create a new one */
	if (ctxpin == NULL) {
		ctxp = xmlXPathNewContext(docp);
	} else {
		DOMXML_GET_OBJ(ctxp, ctxpin, le_xpathctxp);
	}

	if (contextnode) {
		DOMXML_GET_OBJ(contextnodep, contextnode, le_domxmlnodep);
	}
	ctxp->node = contextnodep;
	str = (char*) emalloc((name_len+3) * sizeof(char)) ;
	if (str == NULL) {
		php_error(E_WARNING, "%s(): cannot allocate memory for string", get_active_function_name(TSRMLS_C));
	}
	sprintf(str ,"//%s",name);

	xpathobjp = xmlXPathEval(str, ctxp);
	efree(str);
	ctxp->node = NULL;
	if (!xpathobjp) {
		RETURN_FALSE;
	}
	MAKE_STD_ZVAL(rv);

	if(array_init(rv) != SUCCESS)
	{
		php_error(E_WARNING, "%s(): cannot create required array", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	switch (Z_TYPE_P(xpathobjp)) {

		case XPATH_NODESET:
		{
			int i;
			xmlNodeSetPtr nodesetp;

			if (NULL == (nodesetp = xpathobjp->nodesetval)) {
				zval_dtor(rv);
				RETURN_FALSE;
			}

			for (i = 0; i < nodesetp->nodeNr; i++) {
				xmlNodePtr node = nodesetp->nodeTab[i];
				zval *child;
				int retnode;

				/* construct a node object */
				child = php_domobject_new(node, &retnode TSRMLS_CC);
				zend_hash_next_index_insert(Z_ARRVAL_P(rv), &child, sizeof(zval *), NULL);
			}

			break;
		}
		default:
			break;
	}

	*return_value = *rv;
	FREE_ZVAL(rv);
}
/* }}} */

/* {{{ proto string domxml_doc_get_element_by_id(string id [,object xpathctx_handle] )
   Returns element for given id or false if not found */
PHP_FUNCTION(domxml_doc_get_element_by_id)
{
	zval *id, *rv = NULL, *contextnode = NULL, *ctxpin = NULL;
	xmlXPathContextPtr ctxp ;
	xmlDocPtr docp;

	xmlXPathObjectPtr xpathobjp;
	xmlNode *contextnodep;
	int name_len;
	char *str, *name;

	contextnode = NULL;
	contextnodep = NULL;

	DOMXML_PARAM_FOUR(docp, id, le_domxmldocp, "s|oo", &name, &name_len,&ctxpin,&contextnodep);
	
	/* if no xpath_context was submitted, create a new one */
	if (ctxpin == NULL) {
		ctxp = xmlXPathNewContext(docp);
	} else {
		DOMXML_GET_OBJ(ctxp, ctxpin, le_xpathctxp);
	}
	
	if (contextnode) {
		DOMXML_GET_OBJ(contextnodep, contextnode, le_domxmlnodep);
	}
	ctxp->node = contextnodep;
	str = (char*) emalloc((name_len+14) * sizeof(char)) ;
	if (str == NULL) {
		php_error(E_WARNING, "%s(): cannot allocate memory for string", get_active_function_name(TSRMLS_C));
	}
	sprintf(str ,"//*[@ID = '%s']",name);
	xpathobjp = xmlXPathEval(str, ctxp);
	efree(str);
	ctxp->node = NULL;
	if (!xpathobjp) {
		RETURN_FALSE;
	}

	switch (Z_TYPE_P(xpathobjp)) {

		case XPATH_NODESET:
		{
			xmlNodeSetPtr nodesetp;

			if (NULL == (nodesetp = xpathobjp->nodesetval)) {
				zval_dtor(rv);
				RETURN_FALSE;
			}

			if (nodesetp->nodeNr > 0) {
				xmlNodePtr node = nodesetp->nodeTab[0];
				int retnode;
				rv = php_domobject_new(node, &retnode TSRMLS_CC);
				SEPARATE_ZVAL(&rv);
			}
			else {
				/*return false, if no nodes were found */
				RETURN_FALSE;
			}

			break;
		}
		default:
			break;
	}

	*return_value = *rv;
	FREE_ZVAL(rv);
}
/* }}} */
#endif

/* {{{ proto string domxml_elem_get_elements_by_tagname(string tagname)
   Returns array with nodes with given tagname in element or empty array, if not found */
PHP_FUNCTION(domxml_elem_get_elements_by_tagname)
{
	zval *id,*rv;
	xmlNode *nodep;
	int name_len,i;
	char *name;
	xmlNodeSet *nodesetp;

	DOMXML_PARAM_TWO(nodep, id, le_domxmlelementp, "s", &name, &name_len);

	MAKE_STD_ZVAL(rv);
	
	if(array_init(rv) != SUCCESS) {
		php_error(E_WARNING, "%s(): cannot create required array", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	nodesetp = php_get_elements_by_tagname(nodep,name);

	if(nodesetp) {
		for (i = 0; i < nodesetp->nodeNr; i++) {
			xmlNodePtr node = nodesetp->nodeTab[i];
			zval *child;
			int retnode;

			child = php_domobject_new(node, &retnode TSRMLS_CC);
			zend_hash_next_index_insert(Z_ARRVAL_P(rv), &child, sizeof(zval *), NULL);
		}
	}
	*return_value = *rv;
	FREE_ZVAL(rv);

}
/* }}} */

/* End of Methods DomElement }}} */


/* {{{ Methods of Class DomDocumentType */

/* {{{ proto array domxml_doctype_name(void)
   Returns name of DocumentType */
PHP_FUNCTION(domxml_doctype_name)
{
	zval *id;
	xmlDtdPtr attrp;

	DOMXML_NO_ARGS();

	DOMXML_GET_THIS_OBJ(attrp, id, le_domxmldoctypep);

	RETURN_STRING((char *) (attrp->name), 1);
}
/* }}} */

/* {{{ proto array domxml_doctype_system_id(void)
   Returns system id of DocumentType */
PHP_FUNCTION(domxml_doctype_system_id)
{
	zval *id;
	xmlDtdPtr attrp;

	DOMXML_NO_ARGS();

	DOMXML_GET_THIS_OBJ(attrp, id, le_domxmldoctypep);

	if(attrp->SystemID) {
		RETURN_STRING((char *) (attrp->SystemID), 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ proto array domxml_doctype_public_id(void)
   Returns public id of DocumentType */
PHP_FUNCTION(domxml_doctype_public_id)
{
	zval *id;
	xmlDtdPtr attrp;

	DOMXML_NO_ARGS();

	DOMXML_GET_THIS_OBJ(attrp, id, le_domxmldoctypep);

	if(attrp->ExternalID) {
		RETURN_STRING((char *) (attrp->ExternalID), 1);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ proto array domxml_doctype_entities(void)
   Returns list of entities */
PHP_FUNCTION(domxml_doctype_entities)
{
	zval *id;
	xmlNode *last;
	xmlDtdPtr doctypep;
	int ret;

	DOMXML_NOT_IMPLEMENTED();

	DOMXML_PARAM_NONE(doctypep, id, le_domxmldoctypep);

	last = doctypep->entities;
	if (!last) {
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	while (last) {
		zval *child;
		child = php_domobject_new(last, &ret TSRMLS_CC);
		add_next_index_zval(return_value, child);
		last = last->next;
	}
}
/* }}} */

/* {{{ proto array domxml_doctype_notations(void)
   Returns list of notations */
PHP_FUNCTION(domxml_doctype_notations)
{
	zval *id;
	xmlNode *last;
	xmlDtdPtr doctypep;
	int ret;

	DOMXML_NOT_IMPLEMENTED();

	DOMXML_PARAM_NONE(doctypep, id, le_domxmldoctypep);

	last = doctypep->notations;
	if (!last) {
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	while (last) {
		zval *child;
		child = php_domobject_new(last, &ret TSRMLS_CC);
		add_next_index_zval(return_value, child);
		last = last->next;
	}
}
/* }}} */

/* End of Methods DomElementType }}} */


/* {{{ Methods of Class DomDocument */

/* {{{ proto object domxml_doc_doctype(void)
   Returns DomDocumentType */
PHP_FUNCTION(domxml_doc_doctype)
{
	zval *id, *rv;
	xmlDtdPtr dtd;
	xmlDocPtr docp;
	int ret;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	DOMXML_NO_ARGS();

	dtd = xmlGetIntSubset(docp);
	if (!dtd) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) dtd, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_implementation(void)
   Returns DomeDOMImplementation */
PHP_FUNCTION(domxml_doc_implementation)
{
/*	zval *id;
	xmlDocPtr docp;*/

	DOMXML_NOT_IMPLEMENTED();

/*
	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	rv = php_domobject_new(node, &ret TSRMLS_CC);
	SEPARATE_ZVAL(&rv);
	*return_value = *rv;
*/
}
/* }}} */

/* {{{ proto array domxml_doc_document_element(void)
   Returns root node of document */
PHP_FUNCTION(domxml_doc_document_element)
{
	zval *id;
	xmlDoc *docp;
	xmlNode *node;
	int ret;

	id = getThis();

	if (!id) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &id) == FAILURE) {
			return;
		}
	}

	DOMXML_GET_OBJ(docp, id, le_domxmldocp);

	node = docp->children;
	if (!node) {
		RETURN_FALSE;
	}

	while (node) {
		if (Z_TYPE_P(node) == XML_ELEMENT_NODE) {
			zval *rv;
			DOMXML_RET_OBJ(rv, node, &ret);
			return;
		}
		node = node->next;
	}
}
/* }}} */

/* {{{ proto object domxml_doc_create_element(string name)
   Creates new element node */
PHP_FUNCTION(domxml_doc_create_element)
{
	zval *id, *rv;
	xmlNode *node;
	xmlDocPtr docp;
	int ret, name_len;
	char *name;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	node = xmlNewNode(NULL, name);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_create_text_node(string content)
   Creates new text node */
PHP_FUNCTION(domxml_doc_create_text_node)
{
	zval *id, *rv;
	xmlNode *node;
	xmlDocPtr docp;
	int ret, content_len;
	char *content;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_len) == FAILURE) {
		return;
	}

	node = xmlNewTextLen(content, content_len);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_create_comment(string content)
   Creates new comment node */
PHP_FUNCTION(domxml_doc_create_comment)
{
	zval *id, *rv;
	xmlNode *node;
	xmlDocPtr docp;
	int ret, content_len;
	char *content;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_len) == FAILURE) {
		return;
	}

	node = xmlNewComment(content);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_create_attribute(string name, string value)
   Creates new attribute node */
PHP_FUNCTION(domxml_doc_create_attribute)
{
	zval *id, *rv;
	xmlAttrPtr node;
	xmlDocPtr docp;
	int ret, name_len, value_len;
	char *name, *value;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &value, &value_len) == FAILURE) {
		return;
	}

	node = xmlNewProp(NULL, name, value);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, (xmlNodePtr) node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_create_cdata_section(string content)
   Creates new cdata node */
PHP_FUNCTION(domxml_doc_create_cdata_section)
{
	zval *id, *rv;
	xmlNode *node;
	xmlDocPtr docp;
	int ret, content_len;
	char *content;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_len) == FAILURE) {
		return;
	}

	node = xmlNewCDataBlock(docp, content, content_len);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_create_entity_reference(string name)
   Creates new cdata node */
PHP_FUNCTION(domxml_doc_create_entity_reference)
{
	zval *id, *rv;
	xmlNode *node;
	xmlDocPtr docp;
	int ret, name_len;
	char *name;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}
	node = xmlNewReference(docp, name);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_create_processing_instruction(string name)
   Creates new processing_instruction node */
PHP_FUNCTION(domxml_doc_create_processing_instruction)
{
	zval *id, *rv;
	xmlNode *node;
	xmlDocPtr docp;
	int ret, name_len, content_len;
	char *name, *content;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &content, &content_len) == FAILURE) {
		return;
	}

	node = xmlNewPI(name, content);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_doc_imported_node(object node, bool recursive)
   Creates new element node */
PHP_FUNCTION(domxml_doc_imported_node)
{
	zval *arg1, *id, *rv;
	xmlNodePtr node, srcnode;
	xmlDocPtr docp;
	int ret, recursive = 0;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	/* FIXME: which object type to expect? */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|l", &arg1, &recursive) == FAILURE) {
		return;
	}

	DOMXML_GET_OBJ(srcnode, arg1, le_domxmlnodep);

	node = xmlCopyNode(srcnode, recursive);
	if (!node) {
		RETURN_FALSE;
	}
	node->doc = docp;			/* Not enough because other nodes in the tree are not set */

	DOMXML_RET_OBJ(rv, node, &ret);
}
/* }}} */

/* {{{ proto object domxml_dtd(void)
   Returns DTD of document */
PHP_FUNCTION(domxml_intdtd)
{
	zval *id, *rv;
	xmlDoc *docp;
	xmlDtd *dtd;
	int ret;

	DOMXML_GET_THIS_OBJ(docp, id, le_domxmldocp);

	dtd = xmlGetIntSubset(docp);
	if (!dtd) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) dtd, &ret);
}
/* }}} */

/* {{{ proto string domxml_dump_mem(object doc_handle [, int format])
   Dumps document into string and optionally formats it */
PHP_FUNCTION(domxml_dump_mem)
{
	zval *id;
	xmlDoc *docp;
	xmlChar *mem;
	int format = 0;
	int size;

	DOMXML_PARAM_ONE(docp, id, le_domxmldocp,"|l", &format);
	if (format) {
		xmlKeepBlanksDefault(0);
		xmlDocDumpFormatMemory(docp, &mem, &size, format);
	} else {
		xmlDocDumpMemory(docp, &mem, &size);
	}

	if (!size) {
		RETURN_FALSE;
	}
	RETURN_STRINGL(mem, size, 1);
}
/* }}} */

/* {{{ proto int domxml_dump_mem_file(string filename [, int compressmode [, int format]])
   Dumps document into file and uses compression if specified. Returns false on error, otherwise the length of the xml-document (uncompressed) */
PHP_FUNCTION(domxml_dump_mem_file)
{
	zval *id;
	xmlDoc *docp;
	int file_len, bytes;
	int format = 0;
	int compressmode = 0;
	char *file;

	DOMXML_PARAM_FOUR(docp, id, le_domxmldocp, "s|ll", &file, &file_len, &compressmode, &format);

	xmlSetCompressMode (compressmode);

	if (format) {
		xmlKeepBlanksDefault(0);
		bytes = xmlSaveFormatFile(file,docp,format);
	} else {
		bytes = xmlSaveFile(file,docp);
	}

	if (bytes == -1) {
		RETURN_FALSE;
	}
	RETURN_LONG(bytes);
}
/* }}} */

/* {{{ proto string domxml_dump_node(object doc_handle, object node_handle [, int format [, int level]])
   Dumps node into string */
PHP_FUNCTION(domxml_dump_node)
{
	zval *id, *nodep;
	xmlDocPtr docp;
	xmlNodePtr elementp;
	xmlChar *mem ;
	xmlBufferPtr buf;
	int level = 0;
	int format = 0;
	
	DOMXML_PARAM_THREE(docp, id, le_domxmldocp,"o|ll",&nodep,&format,&level);
	
	DOMXML_GET_OBJ(elementp, nodep, le_domxmlnodep);

	if (Z_TYPE_P(elementp) == XML_DOCUMENT_NODE || Z_TYPE_P(elementp) == XML_HTML_DOCUMENT_NODE ) {
		php_error(E_WARNING, "%s(): cannot dump element with a document node", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}
	
	buf = xmlBufferCreate();
	if (!buf) {
		php_error(E_WARNING, "%s(): could fetch buffer", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}
	
	xmlNodeDump(buf, docp, elementp,level,format);

	mem = (xmlChar*) xmlBufferContent(buf);

	if (!mem) {
		xmlBufferFree(buf);
		RETURN_FALSE;
	}
	RETVAL_STRING(mem,  1);
	
	xmlBufferFree(buf);

}
/* }}} */


/* {{{ proto object xmldoc(string xmldoc [, bool from_file])
   Creates DOM object of XML document */
PHP_FUNCTION(xmldoc)
{
	zval *rv;
	xmlDoc *docp;
	int ret;
	char *buffer;
	int buffer_len;
	zend_bool from_file = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &buffer, &buffer_len, &from_file) == FAILURE) {
		return;
	}

	if (from_file) {
		docp = xmlParseFile(buffer);
	} else {
		docp = xmlParseDoc(buffer);
	}
	if (!docp)
		RETURN_FALSE;

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);
}
/* }}} */

/* {{{ proto object xmldocfile(string filename)
   Creates DOM object of XML document in file */
PHP_FUNCTION(xmldocfile)
{
	zval *rv;
	xmlDoc *docp;
	int ret, file_len;
	char *file;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	docp = xmlParseFile(file);
	if (!docp) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);

	add_property_resource(return_value, "doc", ret);
	if (docp->name)
		add_property_stringl(return_value, "name", (char *) docp->name, strlen(docp->name), 1);
	if (docp->URL)
		add_property_stringl(return_value, "url", (char *) docp->URL, strlen(docp->URL), 1);
	add_property_stringl(return_value, "version", (char *) docp->version, strlen(docp->version), 1);
	if (docp->encoding)
		add_property_stringl(return_value, "encoding", (char *) docp->encoding, strlen(docp->encoding), 1);
	add_property_long(return_value, "standalone", docp->standalone);
	add_property_long(return_value, "type", Z_TYPE_P(docp));
	add_property_long(return_value, "compression", docp->compression);
	add_property_long(return_value, "charset", docp->charset);
	zend_list_addref(ret);
}
/* }}} */

#if defined(LIBXML_HTML_ENABLED)
/* {{{ proto string domxml_html_dump_mem([int doc_handle])
   Dumps document into string as HTML */
PHP_FUNCTION(domxml_html_dump_mem)
{
	zval *id;
	xmlDoc *docp;
	xmlChar *mem;
	int size;

	DOMXML_PARAM_NONE(docp, id, le_domxmldocp);

	htmlDocDumpMemory(docp, &mem, &size);
	if (!size) {
		if (mem)
			xmlFree(mem);
		RETURN_FALSE;
	}
	RETURN_STRINGL(mem, size, 1);
	xmlFree(mem);
}
/* }}} */

/* {{{ proto object html_doc(string html_doc [, bool from_file])
   Creates DOM object of HTML document */
PHP_FUNCTION(html_doc)
{
	zval *rv;
	xmlDoc *docp;
	int ret;
	char *buffer;
	int buffer_len;
	zend_bool from_file = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &buffer, &buffer_len, &from_file) == FAILURE) {
		return;
	}

	if (from_file) {
		docp = htmlParseFile(buffer, NULL);
	} else {
		docp = htmlParseDoc(buffer, NULL);
	}
	if (!docp)
		RETURN_FALSE;

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);
}
/* }}} */

/* {{{ proto object html_doc_file(string filename)
   Creates DOM object of HTML document in file */
PHP_FUNCTION(html_doc_file)
{
	zval *rv;
	xmlDoc *docp;
	int ret, file_len;
	char *file;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		return;
	}

	docp = htmlParseFile(file, NULL);
	if (!docp) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);

	add_property_resource(return_value, "doc", ret);
	if (docp->name)
		add_property_stringl(return_value, "name", (char *) docp->name, strlen(docp->name), 1);
	if (docp->URL)
		add_property_stringl(return_value, "url", (char *) docp->URL, strlen(docp->URL), 1);
	if (docp->version)
		add_property_stringl(return_value, "version", (char *) docp->version, strlen(docp->version), 1);
/*	add_property_stringl(return_value, "version", (char *) docp->version, strlen(docp->version), 1);*/
	if (docp->encoding)
		add_property_stringl(return_value, "encoding", (char *) docp->encoding, strlen(docp->encoding), 1);
	add_property_long(return_value, "standalone", docp->standalone);
	add_property_long(return_value, "type", Z_TYPE_P(docp));
	add_property_long(return_value, "compression", docp->compression);
	add_property_long(return_value, "charset", docp->charset);
	zend_list_addref(ret);
}
/* }}} */
#endif  /* defined(LIBXML_HTML_ENABLED) */

/* {{{ proto bool domxml_substitute_entities_default(bool enable)
   Set and return the previous value for default entity support */
PHP_FUNCTION(domxml_substitute_entities_default)
{
	zend_bool enable;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &enable) == FAILURE) {
		return;
	}

	RETURN_BOOL(xmlSubstituteEntitiesDefault(enable));
}
/* }}} */

/* {{{ proto bool domxml_node_text_concat(string content)
   Add string tocontent of a node */
PHP_FUNCTION(domxml_node_text_concat)
{
	zval *id;
	xmlNode *nodep;
	char *content;
	int content_len;

	DOMXML_GET_THIS_OBJ(nodep, id, le_domxmlnodep);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_len) == FAILURE) {
		return;
	}

	if (content_len)
		xmlTextConcat(nodep, content, content_len);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto object domxml_add_root(string name)
   Adds root node to document */
PHP_FUNCTION(domxml_add_root)
{
	zval *id, *rv;
	xmlDoc *docp;
	xmlNode *nodep;
	int ret, name_len;
	char *name;

	DOMXML_PARAM_TWO(docp, id, le_domxmldocp, "s", &name, &name_len);

	nodep = xmlNewDocNode(docp, NULL, name, NULL);
	if (!nodep) {
		RETURN_FALSE;
	}

	xmlDocSetRootElement(docp, nodep);

	DOMXML_RET_OBJ(rv, nodep, &ret);
}
/* }}} */

/* {{{ proto object domxml_new_xmldoc(string version)
   Creates new xmldoc */
PHP_FUNCTION(domxml_new_xmldoc)
{
	zval *rv;
	xmlDoc *docp;
	int ret, buf_len;
	char *buf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buf, &buf_len) == FAILURE) {
		return;
	}

	docp = xmlNewDoc(buf);
	if (!docp) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);
}
/* }}} */

/* {{{ proto object domxml_parer([string buf[,string filename]])
   Creates new xmlparser */
PHP_FUNCTION(domxml_parser)
{
	zval *rv;
	xmlParserCtxtPtr parserp;
	int ret, buf_len = 0;
	char *buf = "";
	char *filename = NULL;
	int filename_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &buf, &buf_len, &filename, &filename_len) == FAILURE) {
		return;
	}

	parserp =  xmlCreatePushParserCtxt(NULL, NULL, buf, buf_len , filename);
	if (!parserp) {
		RETURN_FALSE;
	}

	rv = php_xmlparser_new(parserp, &ret TSRMLS_CC);    
	DOMXML_RET_ZVAL(rv);
}
/* }}} */

/* {{{ proto bool domxml_parser_add_chunk(string chunk)
   adds xml-chunk to parser */
PHP_FUNCTION(domxml_parser_add_chunk)
{
	zval *id;
	xmlParserCtxtPtr parserp;
	char *chunk;
	int chunk_len, error;

	DOMXML_PARAM_TWO(parserp, id, le_domxmlparserp,"s", &chunk, &chunk_len);
	error = xmlParseChunk(parserp, chunk, chunk_len , 0);
	if (error != 0) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto object domxml_parser_end([string chunk])
   Ends parsing and returns DomDocument*/
PHP_FUNCTION(domxml_parser_end)
{
	zval *id,*rv;
	xmlParserCtxtPtr parserp;
	char *chunk = NULL;
	int chunk_len = 0, error;
	int ret;


	DOMXML_PARAM_TWO(parserp, id, le_domxmlparserp,"|s", &chunk, &chunk_len);
	error = xmlParseChunk(parserp, chunk, chunk_len, 1);
	if (error != 0) {
		php_error(E_ERROR,"error: %d",error);		
		RETURN_FALSE;
	}
	if (parserp->myDoc != NULL) {
		DOMXML_RET_OBJ(rv, (xmlNodePtr) parserp->myDoc, &ret);
	}
	else {
		RETVAL_FALSE
	}
}
/* }}} */

#ifdef newcode
/* {{{ proto int node_namespace([int node])
   Returns list of namespaces */
static int node_namespace(zval **attributes, xmlNode *nodep TSRMLS_DC)
{
	xmlNs *ns;

	/* Get the children of the current node */
	ns = nodep->ns;
	if (!ns) {
		return -1;
	}

	/* create an php array for the children */
	MAKE_STD_ZVAL(*attributes);
	if (array_init(*attributes) == FAILURE) {
		return -1;
	}

	while (ns) {
		zval *pattr;
		int ret;

		pattr = php_domobject_new((xmlNodePtr) ns, &ret TSRMLS_CC);
		SEPARATE_ZVAL(&pattr);

/*		if(!ret) { */
		if (ns->href)
			add_property_stringl(pattr, "href", (char *) ns->href, strlen(ns->href), 1);
		if (ns->prefix)
			add_property_stringl(pattr, "prefix", (char *) ns->prefix, strlen(ns->prefix), 1);
		add_property_long(pattr, "type", Z_TYPE_P(ns));
/*		} */

		zend_hash_next_index_insert(Z_ARRVAL_PP(attributes), &pattr, sizeof(zval *), NULL);
		ns = ns->next;
	}
	return 0;
}
/* }}} */
#endif

/* We don't have a type zval. **attributes is also very unusual. */

/* {{{ proto int node_attributes(zval **attributes, int node)
   Returns list of children nodes */
static int node_attributes(zval **attributes, xmlNode *nodep TSRMLS_DC)
{
	xmlAttr *attr;
	int count = 0;

	/* Get the children of the current node */
	if (Z_TYPE_P(nodep) != XML_ELEMENT_NODE)
		return -1;
	attr = nodep->properties;
	if (!attr)
		return -1;

	/* create an php array for the children */
	MAKE_STD_ZVAL(*attributes);
	array_init(*attributes);

	while (attr) {
		zval *pattr;
		int ret;

		pattr = php_domobject_new((xmlNodePtr) attr, &ret TSRMLS_CC);
		/** XXX FIXME XXX */
/*		if(0 <= (n = node_children(&children, attr->children TSRMLS_CC))) {
			zend_hash_update(Z_OBJPROP_P(value), "children", sizeof("children"), (void *) &children, sizeof(zval *), NULL);
		}
*/		add_property_string(pattr, "name", (char *) (attr->name), 1);
		add_property_string(pattr, "value", xmlNodeGetContent((xmlNodePtr) attr), 1);
		zend_hash_next_index_insert(Z_ARRVAL_PP(attributes), &pattr, sizeof(zval *), NULL);
		attr = attr->next;
		count++;
	}
	return count;
}
/* }}} */

/* {{{ proto int node_children([int node])
   Returns list of children nodes */
static int node_children(zval **children, xmlNode *nodep TSRMLS_DC)
{
	zval *mchildren, *attributes;
	/* zval *namespace; */
	xmlNode *last;
	int count = 0;

	/* Get the children of the current node */
	last = nodep;
	if (!last) {
		return -1;
	}

	/* create an php array for the children */
	MAKE_STD_ZVAL(*children);
	array_init(*children);

	while (last) {
		zval *child;
		int ret;

		if (NULL != (child = php_domobject_new(last, &ret TSRMLS_CC))) {
			zend_hash_next_index_insert(Z_ARRVAL_PP(children), &child, sizeof(zval *), NULL);

			/* Get the namespace of the current node and add it as a property */
			/* XXX FIXME XXX */
/*
			if(!node_namespace(&namespace, last))
				zend_hash_update(Z_OBJPROP_P(child), "namespace", sizeof("namespace"), (void *) &namespace, sizeof(zval *), NULL);
*/

			/* Get the attributes of the current node and add it as a property */
			if (node_attributes(&attributes, last TSRMLS_CC) >= 0)
				zend_hash_update(Z_OBJPROP_P(child), "attributes", sizeof("attributes"), (void *) &attributes, sizeof(zval *), NULL);

			/* Get recursively the children of the current node and add it as a property */
			if (node_children(&mchildren, last->children TSRMLS_CC) >= 0)
				zend_hash_update(Z_OBJPROP_P(child), "children", sizeof("children"), (void *) &mchildren, sizeof(zval *), NULL);

			count++;
		}
		last = last->next;
	}
	return count;
}
/* }}} */

/* {{{ proto object domxml_xmltree(string xmltree)
   Creates a tree of PHP objects from an XML document */
PHP_FUNCTION(domxml_xmltree)
{
	zval *children, *rv;
	xmlDoc *docp;
	xmlNode *root;
	int ret, buf_len;
	char *buf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buf, &buf_len) == FAILURE) {
		return;
	}

	/* Create a new xml document */
	docp = xmlParseDoc(buf);
	if (!docp) {
		RETURN_FALSE;
	}

	/* get the root and add as a property to the document */
	root = docp->children;
	if (!root) {
		xmlFreeDoc(docp);
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);

	/* The root itself maybe an array. Though you may not have two Elements
	   as root, you may have a comment, pi and and element as root.
	   Thanks to Paul DuBois for pointing me at this.
	 */
	if (node_children(&children, root TSRMLS_CC) >= 0) {
		zend_hash_update(Z_OBJPROP_P(return_value), "children",sizeof("children"), (void *) &children, sizeof(zval *), NULL);
	}
/*	xmlFreeDoc(docp); */
}
/* }}} */

#if defined(LIBXML_XPATH_ENABLED)
/* {{{ proto bool xpath_init(void)
   Initializing XPath environment */
PHP_FUNCTION(xpath_init)
{
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	xmlXPathInit();
	RETURN_TRUE;
}
/* }}} */

/* {{{ php_xpathptr_new_context()
 */
static void php_xpathptr_new_context(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zval *id, *rv;
	xmlXPathContextPtr ctx;
	xmlDocPtr docp;
	int ret;

	DOMXML_PARAM_NONE(docp, id, le_domxmldocp);

#if defined(LIBXML_XPTR_ENABLED)
	if (mode == PHP_XPTR)
		ctx = xmlXPtrNewContext(docp, NULL, NULL);
	else
#endif
		ctx = xmlXPathNewContext(docp);
	if (!ctx) {
		RETURN_FALSE;
	}

	rv = php_xpathcontext_new(ctx, &ret TSRMLS_CC);
	DOMXML_RET_ZVAL(rv);
}
/* }}} */

/* {{{ proto object xpath_new_context([int doc_handle])
   Creates new XPath context */
PHP_FUNCTION(xpath_new_context)
{
	php_xpathptr_new_context(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_XPATH);
}
/* }}} */

/* {{{ proto object xptr_new_context([int doc_handle])
   Creates new XPath context */
PHP_FUNCTION(xptr_new_context)
{
	php_xpathptr_new_context(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_XPTR);
}
/* }}} */

/* {{{ php_xpathptr_eval()
 */
static void php_xpathptr_eval(INTERNAL_FUNCTION_PARAMETERS, int mode, int expr)
{
	zval *id, *rv, *contextnode = NULL;
	xmlXPathContextPtr ctxp;
	xmlXPathObjectPtr xpathobjp;
	xmlNode *contextnodep;
	int ret, str_len;
	char *str;

	contextnode = NULL;
	contextnodep = NULL;

	if (NULL == (id = getThis())) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "os|o", &id, &str, &str_len, &contextnode) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|o", &str, &str_len, &contextnode) == FAILURE) {
			return;
		}
	}

	ctxp = php_xpath_get_context(id, le_xpathctxp, 0 TSRMLS_CC);
	if (!ctxp) {
		php_error(E_WARNING, "%s(): cannot fetch XPATH context", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	if (contextnode) {
		DOMXML_GET_OBJ(contextnodep, contextnode, le_domxmlnodep);
	}
	ctxp->node = contextnodep;

#if defined(LIBXML_XPTR_ENABLED)
	if (mode == PHP_XPTR) {
		xpathobjp = xmlXPtrEval(BAD_CAST str, ctxp);
	} else {
#endif
		if (expr) {
			xpathobjp = xmlXPathEvalExpression(str, ctxp);
		} else {
			xpathobjp = xmlXPathEval(str, ctxp);
		}
#if defined(LIBXML_XPTR_ENABLED)
	}
#endif

	ctxp->node = NULL;
	if (!xpathobjp) {
		RETURN_FALSE;
	}

	if (NULL == (rv = php_xpathobject_new(xpathobjp, &ret TSRMLS_CC))) {
		php_error(E_WARNING, "%s(): cannot create required XPATH objcet", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}
	SEPARATE_ZVAL(&rv);

	add_property_long(rv, "type", Z_TYPE_P(xpathobjp));

	switch (Z_TYPE_P(xpathobjp)) {

		case XPATH_UNDEFINED:
			break;

		case XPATH_NODESET:
		{
			int i;
			zval *arr;
			xmlNodeSetPtr nodesetp;

			MAKE_STD_ZVAL(arr);
			if (array_init(arr) == FAILURE) {
				zval_dtor(rv);
				RETURN_FALSE;
			}

			if (NULL == (nodesetp = xpathobjp->nodesetval)) {
				zval_dtor(rv);
				RETURN_FALSE;
			}

			for (i = 0; i < nodesetp->nodeNr; i++) {
				xmlNodePtr node = nodesetp->nodeTab[i];
				zval *child;
				int retnode;

				/* construct a node object */
				child = php_domobject_new(node, &retnode TSRMLS_CC);
				zend_hash_next_index_insert(Z_ARRVAL_P(arr), &child, sizeof(zval *), NULL);
			}
			zend_hash_update(Z_OBJPROP_P(rv), "nodeset", sizeof("nodeset"), (void *) &arr, sizeof(zval *), NULL);
			break;
		}

		case XPATH_BOOLEAN:
			add_property_bool(rv, "value", xpathobjp->boolval);
			break;

		case XPATH_NUMBER:
			add_property_double(rv, "value", xpathobjp->floatval);
			break;

		case XPATH_STRING:
			add_property_string(rv, "value", xpathobjp->stringval, 1);
			break;

		case XPATH_POINT:
			break;

		case XPATH_RANGE:
			break;

		case XPATH_LOCATIONSET:
			break;

		case XPATH_USERS:
			break;

		case XPATH_XSLT_TREE:
			break;
	}

	*return_value = *rv;
	FREE_ZVAL(rv);
}
/* }}} */

/* {{{ proto object xpath_eval([object xpathctx_handle,] string str)
   Evaluates the XPath Location Path in the given string */
PHP_FUNCTION(xpath_eval)
{
	php_xpathptr_eval(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_XPATH, 0);
}
/* }}} */

/* {{{ proto object xpath_eval_expression([object xpathctx_handle,] string str)
   Evaluates the XPath expression in the given string */
PHP_FUNCTION(xpath_eval_expression)
{
	php_xpathptr_eval(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_XPATH, 1);
}
/* }}} */

/* {{{ proto bool xpath_register_ns([object xpathctx_handle,] string namespace_prefix, string namespace_uri)
	Registeres the given namespace in the passed XPath context */
PHP_FUNCTION(xpath_register_ns)
{
	/*
	TODO:
	- automagically register all namespaces when creating a new context
	*/

	int prefix_len, uri_len, result;
	xmlXPathContextPtr ctxp;
	char *prefix, *uri, *uri_static;
	zval *id;

	DOMXML_PARAM_FOUR(ctxp, id, le_xpathctxp, "ss", &prefix, &prefix_len, &uri, &uri_len);

	/* set the context node to NULL - what is a context node anyway? */
	ctxp->node = NULL;

	/*
		this is a hack - libxml2 doesn't copy the URI, it simply uses the string
		given in the parameter - which is normally deallocated after the function
	*/
    uri_static = estrndup(uri, uri_len);
	result = xmlXPathRegisterNs(ctxp, prefix, uri_static);

	if (0 == result) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */
#endif	/* defined(LIBXML_XPATH_ENABLED) */

#if defined(LIBXML_XPTR_ENABLED)
/* {{{ proto int xptr_eval([int xpathctx_handle,] string str)
   Evaluates the XPtr Location Path in the given string */
PHP_FUNCTION(xptr_eval)
{
	php_xpathptr_eval(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_XPTR, 0);
}
/* }}} */
#endif	/* LIBXML_XPTR_ENABLED */

/* {{{ proto string domxml_version(void)
   Get XML library version */
PHP_FUNCTION(domxml_version)
{
	RETURN_STRING(LIBXML_DOTTED_VERSION, 1);
}
/* }}} */

#if HAVE_DOMXSLT
static zval *php_xsltstylesheet_new(xsltStylesheetPtr obj, int *found TSRMLS_DC)
{
	zval *wrapper;
	int rsrc_type;

		*found = 0;

	if (!obj) {
		MAKE_STD_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((wrapper = (zval *) dom_object_get_data((void *) obj))) {
		zval_add_ref(&wrapper);
		*found = 1;
		return wrapper;
	}

	MAKE_STD_ZVAL(wrapper);

	object_init_ex(wrapper, domxsltstylesheet_class_entry);
	rsrc_type = le_domxsltstylesheetp;
	php_xsltstylesheet_set_object(wrapper, (void *) obj, rsrc_type);

	return (wrapper);
}

/* {{{ proto object domxml_xslt_stylesheet(string xsltstylesheet)
   Creates XSLT Stylesheet object from string */
PHP_FUNCTION(domxml_xslt_stylesheet)
{
	zval *rv;
	xmlDocPtr docp;
	xsltStylesheetPtr sheetp;
	int ret;
	char *buffer;
	int buffer_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buffer, &buffer_len) == FAILURE) {
		RETURN_FALSE;
	}

	docp = xmlParseDoc(buffer);

	if (!docp)
		RETURN_FALSE;

	sheetp = xsltParseStylesheetDoc(docp);

	if (!sheetp)
		RETURN_FALSE;

	rv = php_xsltstylesheet_new(sheetp, &ret TSRMLS_CC);
	DOMXML_RET_ZVAL(rv);
}
/* }}} */

/* {{{ proto object domxml_xslt_stylesheet_doc(object xmldoc)
   Creates XSLT Stylesheet object from DOM Document object */
PHP_FUNCTION(domxml_xslt_stylesheet_doc)
{
	zval *rv, *idxml;
	xmlDocPtr docp;
	xmlDocPtr newdocp;
	xsltStylesheetPtr sheetp;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &idxml) == FAILURE) {
		RETURN_FALSE;
	}

	DOMXML_GET_OBJ(docp, idxml, le_domxmldocp);

	newdocp = xmlCopyDoc(docp, 1);

	if (!newdocp)
		RETURN_FALSE;

	sheetp = xsltParseStylesheetDoc(newdocp);

	if (!sheetp)
		RETURN_FALSE;

	rv = php_xsltstylesheet_new(sheetp, &ret TSRMLS_CC);
	DOMXML_RET_ZVAL(rv);
}
/* }}} */

/* {{{ proto object domxml_xslt_stylesheet_file(string filename)
   Creates XSLT Stylesheet object from file */
PHP_FUNCTION(domxml_xslt_stylesheet_file)
{
	zval *rv;
	xsltStylesheetPtr sheetp;
	int ret, file_len;
	char *file;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len) == FAILURE) {
		RETURN_FALSE;
	}

	sheetp = xsltParseStylesheetFile(file);

	if (!sheetp)
		RETURN_FALSE;

	rv = php_xsltstylesheet_new(sheetp, &ret TSRMLS_CC);
	DOMXML_RET_ZVAL(rv);
}
/* }}} */

/* {{{ php_domxslt_string_to_xpathexpr()
   Translates a string to a XPath Expression */
static char *php_domxslt_string_to_xpathexpr(const char *str TSRMLS_DC)
{
	const xmlChar *string = (const xmlChar *)str;

	xmlChar *value;

	if (xmlStrchr(string, '"')) {
		if (xmlStrchr(string, '\'')) {
			php_error(E_WARNING, "Cannot create XPath expression (string contains both quote and double-quotes) in %s",
					  get_active_function_name(TSRMLS_C));
			return NULL;
		}
		value = xmlStrdup((const xmlChar *)"'");
		value = xmlStrcat(value, string);
		value = xmlStrcat(value, (const xmlChar *)"'");
	} else {
		value = xmlStrdup((const xmlChar *)"\"");
		value = xmlStrcat(value, string);
		value = xmlStrcat(value, (const xmlChar *)"\"");
	}

	return (char *)value;
}

/* {{{ php_domxslt_make_params()
   Translates a PHP array to a libxslt parameters array */
static char **php_domxslt_make_params(zval *idvars, int xpath_params TSRMLS_DC)
{
	HashTable *parht;
	int parsize;
	zval **value;
	char *xpath_expr, *string_key = NULL;
	ulong num_key;
	char **params = NULL;
	int i = 0;

	parht = HASH_OF(idvars);
	parsize = (2 * zend_hash_num_elements(parht) + 1) * sizeof(char *);
	params = (char **)emalloc(parsize);
	memset((char *)params, 0, parsize);

	for (zend_hash_internal_pointer_reset(parht);
		zend_hash_get_current_data(parht, (void **)&value) == SUCCESS;
		zend_hash_move_forward(parht)) {

		if (zend_hash_get_current_key(parht, &string_key, &num_key, 1) != HASH_KEY_IS_STRING) {
			php_error(E_WARNING, "Invalid argument or parameter array to %s",
					  get_active_function_name(TSRMLS_C));
			return NULL;
		}
		else {
			SEPARATE_ZVAL(value);
			convert_to_string_ex(value);

			if (!xpath_params) {
				xpath_expr = php_domxslt_string_to_xpathexpr(Z_STRVAL_PP(value) TSRMLS_CC);
			}
			else {
				xpath_expr = Z_STRVAL_PP(value);
			}

			if (xpath_expr) {
				params[i++] = string_key;
				params[i++] = xpath_expr;
			}
		}
	}

	params[i++] = NULL;

	return params;
}

/* {{{ proto object domxml_xslt_process(object xslstylesheet, object xmldoc [, array xslt_parameters [, bool xpath_parameters]])
   Perform an XSLT transformation */
PHP_FUNCTION(domxml_xslt_process)
{
/* TODO:
	- test memory deallocation
	- test other stuff
	- check xsltsp->errors ???
*/
	zval *rv, *idxsl, *idxml, *idparams = NULL;
	zend_bool xpath_params = 0;
	xsltStylesheetPtr xsltstp;
	xmlDocPtr xmldocp;
	xmlDocPtr docp;
	char **params = NULL;
	int ret;

	DOMXML_GET_THIS(idxsl);

	xsltstp = php_xsltstylesheet_get_object(idxsl, le_domxsltstylesheetp, 0 TSRMLS_CC);
	if (!xsltstp) {
		php_error(E_WARNING, "%s(): underlying object missing",
			  get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|ab", &idxml, &idparams, &xpath_params) == FAILURE) {
		RETURN_FALSE;
	}

	DOMXML_GET_OBJ(xmldocp, idxml, le_domxmldocp);

	if (idparams) {
		params = php_domxslt_make_params(idparams, xpath_params TSRMLS_CC);
	}

	docp = xsltApplyStylesheet(xsltstp, xmldocp, (const char**)params);

	if (params) {
		efree(params);
	}

	if (!docp) {
		RETURN_FALSE;
	}

	DOMXML_RET_OBJ(rv, (xmlNodePtr) docp, &ret);
}
/* }}} */

/* {{{ proto string domxml_xslt_version(void)
   Get XSLT library version */
PHP_FUNCTION(domxml_xslt_version)
{
	RETURN_STRING(LIBXSLT_DOTTED_VERSION, 1);
}
/* }}} */
#endif /* HAVE_DOMXSLT */

#endif /* HAVE_DOMXML */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
