/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   |          Marcus Borger <helly@php.net>                               |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#if HAVE_LIBXML && HAVE_DOM
#include "ext/standard/php_rand.h"
#include "php_dom.h"
#include "dom_ce.h"
#include "dom_properties.h"

#include "ext/standard/info.h"
#define PHP_XPATH 1
#define PHP_XPTR 2

zend_object_handlers dom_object_handlers;

static HashTable classes;

static HashTable dom_domstringlist_prop_handlers;
static HashTable dom_namelist_prop_handlers;
static HashTable dom_domimplementationlist_prop_handlers;
static HashTable dom_document_prop_handlers;
static HashTable dom_node_prop_handlers;
static HashTable dom_nodelist_prop_handlers;
static HashTable dom_namednodemap_prop_handlers;
static HashTable dom_characterdata_prop_handlers;
static HashTable dom_attr_prop_handlers;
static HashTable dom_element_prop_handlers;
static HashTable dom_text_prop_handlers;
static HashTable dom_typeinfo_prop_handlers;
static HashTable dom_domerror_prop_handlers;
static HashTable dom_domlocator_prop_handlers;
static HashTable dom_documenttype_prop_handlers;
static HashTable dom_notation_prop_handlers;
static HashTable dom_entity_prop_handlers;
static HashTable dom_processinginstruction_prop_handlers;
static HashTable dom_namespace_node_prop_handlers;
#if defined(LIBXML_XPATH_ENABLED)
static HashTable dom_xpath_prop_handlers;
#endif

typedef int (*dom_read_t)(dom_object *obj, zval **retval TSRMLS_DC);
typedef int (*dom_write_t)(dom_object *obj, zval *newval TSRMLS_DC);

typedef struct _dom_prop_handler {
	dom_read_t read_func;
	dom_write_t write_func;
} dom_prop_handler;

/* {{{ int dom_node_is_read_only(xmlNodePtr node) */
int dom_node_is_read_only(xmlNodePtr node) {
	switch (node->type) {
		case XML_ENTITY_REF_NODE:
		case XML_ENTITY_NODE:
		case XML_DOCUMENT_TYPE_NODE:
		case XML_NOTATION_NODE:
		case XML_DTD_NODE:
		case XML_ELEMENT_DECL:
		case XML_ATTRIBUTE_DECL:
		case XML_ENTITY_DECL:
		case XML_NAMESPACE_DECL:
			return SUCCESS;
			break;
		default:
			if (node->doc == NULL) {
				return SUCCESS;
			} else {
				return FAILURE;
			}
	}
}
/* }}} end dom_node_is_read_only */

/* {{{ int dom_node_children_valid(xmlNodePtr node) */
int dom_node_children_valid(xmlNodePtr node) {
	switch (node->type) {
		case XML_DOCUMENT_TYPE_NODE:
		case XML_DTD_NODE:
		case XML_PI_NODE:
		case XML_COMMENT_NODE:
		case XML_TEXT_NODE:
		case XML_CDATA_SECTION_NODE:
		case XML_NOTATION_NODE:
			return FAILURE;
			break;
		default:
			return SUCCESS;
	}
}
/* }}} end dom_node_children_valid */

/* {{{ dom_get_doc_props() */
dom_doc_propsptr dom_get_doc_props(php_libxml_ref_obj *document)
{
	dom_doc_props *doc_props;

	if (document && document->doc_props) {
		return document->doc_props;
	} else {
		doc_props = emalloc(sizeof(dom_doc_props));
		doc_props->formatoutput = 0;
		doc_props->validateonparse = 0;
		doc_props->resolveexternals = 0;
		doc_props->preservewhitespace = 1;
		doc_props->substituteentities = 0;
		doc_props->stricterror = 1;
		if (document) {
			document->doc_props = doc_props;
		}
		return doc_props;
	}
}
/* }}} */

/* {{{ dom_get_strict_error() */
int dom_get_strict_error(php_libxml_ref_obj *document) {
	int stricterror;
	dom_doc_props *doc_props;

	doc_props = dom_get_doc_props(document);
	stricterror = doc_props->stricterror;
	if (document == NULL) {
		efree(doc_props);
	}

	return stricterror;
}
/* }}} */

/* {{{ xmlNodePtr dom_object_get_node(dom_object *obj) */
xmlNodePtr dom_object_get_node(dom_object *obj)
{
	if (obj->ptr != NULL) {
		return ((php_libxml_node_ptr *)obj->ptr)->node;
	} else {
		return NULL;
	}
}
/* }}} end dom_object_get_node */

/* {{{ dom_object *php_dom_object_get_data(xmlNodePtr obj) */
dom_object *php_dom_object_get_data(xmlNodePtr obj)
{
	if (obj->_private != NULL) {
		return (dom_object *) ((php_libxml_node_ptr *) obj->_private)->_private;
	} else {
		return NULL;
	}
}
/* }}} end php_dom_object_get_data */

/* {{{ dom_read_na */
static int dom_read_na(dom_object *obj, zval **retval TSRMLS_DC)
{
	*retval = NULL;
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot read property");
	return FAILURE;
}
/* }}} */

/* {{{ dom_write_na */
static int dom_write_na(dom_object *obj, zval *newval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

/* {{{ dom_register_prop_handler */
static void dom_register_prop_handler(HashTable *prop_handler, char *name, dom_read_t read_func, dom_write_t write_func TSRMLS_DC)
{
	dom_prop_handler hnd;
	
	hnd.read_func = read_func ? read_func : dom_read_na;
	hnd.write_func = write_func ? write_func : dom_write_na;
	zend_hash_add(prop_handler, name, strlen(name)+1, &hnd, sizeof(dom_prop_handler), NULL);
}
/* }}} */

/* {{{ dom_read_property */
zval *dom_read_property(zval *object, zval *member, zend_bool silent TSRMLS_DC)
{
	dom_object *obj;
	zval tmp_member;
	zval *retval;
	dom_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;
	obj = (dom_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
		ret = hnd->read_func(obj, &retval TSRMLS_CC);
		if (ret == SUCCESS) {
			/* ensure we're creating a temporary variable */
			retval->refcount = 0;
		} else {
			retval = EG(uninitialized_zval_ptr);
		}
	} else {
		std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->read_property(object, member, silent TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}
/* }}} */

/* {{{ dom_write_property */
void dom_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	dom_object *obj;
	zval tmp_member;
	dom_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;
	obj = (dom_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find((HashTable *)obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
		hnd->write_func(obj, value TSRMLS_CC);
		if (! PZVAL_IS_REF(value) && value->refcount == 0) {
			value->refcount++;
			zval_ptr_dtor(&value);
		}
	} else {
		std_hnd = zend_get_std_object_handlers();
		std_hnd->write_property(object, member, value TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

/* {{{ proto somNode dom_import_simplexml(sxeobject node)
   Get a simplexml_element object from dom to allow for processing */
PHP_FUNCTION(dom_import_simplexml)
{
#ifdef HAVE_SIMPLEXML
	zval *rv = NULL;
	zval *node;
	xmlNodePtr nodep;
	php_libxml_node_object *nodeobj;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) == FAILURE) {
		return;
	}

	NODE_GET_OBJ(nodep, node, xmlNodePtr, nodeobj);

	if (nodep->type == XML_ELEMENT_NODE || nodep->type == XML_ATTRIBUTE_NODE) {
		DOM_RET_OBJ(rv, (xmlNodePtr) nodep, &ret, (dom_object *)nodeobj);
	} else {
		php_error(E_WARNING, "Invalid Nodetype to import");
		RETURN_NULL();
	}
#else
	php_error(E_WARNING, "SimpleXML support is not enabled");
	return;
#endif
}
/* }}} */

static zend_function_entry dom_functions[] = {
	PHP_FE(dom_import_simplexml, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry dom_module_entry = {
	STANDARD_MODULE_HEADER,
	"dom",
	dom_functions,
	PHP_MINIT(dom),
	PHP_MSHUTDOWN(dom),
	NULL,
	NULL,
	PHP_MINFO(dom),
	DOM_API_VERSION, /* Extension versionnumber */
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_DOM
ZEND_GET_MODULE(dom)
#endif

/* {{{ PHP_MINIT_FUNCTION(dom) */
PHP_MINIT_FUNCTION(dom)
{
	zend_class_entry ce;

	memcpy(&dom_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dom_object_handlers.read_property = dom_read_property;
	dom_object_handlers.write_property = dom_write_property;
	dom_object_handlers.get_property_ptr_ptr = NULL;

	zend_hash_init(&classes, 0, NULL, NULL, 1);

	INIT_CLASS_ENTRY(ce, "domexception", php_dom_domexception_class_functions);
	dom_domexception_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(), NULL TSRMLS_CC);
	dom_domexception_class_entry->ce_flags |= ZEND_ACC_FINAL;
	dom_domexception_class_entry->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;
	zend_declare_property_long(dom_domexception_class_entry, "code", sizeof("code")-1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	REGISTER_DOM_CLASS(ce, "domstringlist", NULL, php_dom_domstringlist_class_functions, dom_domstringlist_class_entry);
	
	zend_hash_init(&dom_domstringlist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domstringlist_prop_handlers, "length", dom_domstringlist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domstringlist_prop_handlers, sizeof(dom_domstringlist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnamelist", NULL, php_dom_namelist_class_functions, dom_namelist_class_entry);
	
	zend_hash_init(&dom_namelist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_namelist_prop_handlers, "length", dom_namelist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_namelist_prop_handlers, sizeof(dom_namelist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domimplementationlist", NULL, php_dom_domimplementationlist_class_functions, dom_domimplementationlist_class_entry);
	
	zend_hash_init(&dom_domimplementationlist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domimplementationlist_prop_handlers, "length", dom_domimplementationlist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domimplementationlist_prop_handlers, sizeof(dom_domimplementationlist_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domimplementationsource", NULL, php_dom_domimplementationsource_class_functions, dom_domimplementationsource_class_entry);
	REGISTER_DOM_CLASS(ce, "domimplementation", NULL, php_dom_domimplementation_class_functions, dom_domimplementation_class_entry);

	REGISTER_DOM_CLASS(ce, "domnode", NULL, php_dom_node_class_functions, dom_node_class_entry);
	
	zend_hash_init(&dom_node_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_node_prop_handlers, "nodeName", dom_node_node_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "nodeValue", dom_node_node_value_read, dom_node_node_value_write TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "nodeType", dom_node_node_type_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "parentNode", dom_node_parent_node_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "childNodes", dom_node_child_nodes_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "firstChild", dom_node_first_child_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "lastChild", dom_node_last_child_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "previousSibling", dom_node_previous_sibling_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "nextSibling", dom_node_next_sibling_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "attributes", dom_node_attributes_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "ownerDocument", dom_node_owner_document_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "namespaceURI", dom_node_namespace_uri_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "prefix", dom_node_prefix_read, dom_node_prefix_write TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "localName", dom_node_local_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "baseURI", dom_node_base_uri_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_node_prop_handlers, "textContent", dom_node_text_content_read, dom_node_text_content_write TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_node_prop_handlers, sizeof(dom_node_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnamespacenode", NULL, NULL, dom_namespace_node_class_entry);

	zend_hash_init(&dom_namespace_node_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "nodeName", dom_node_node_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "nodeValue", dom_node_node_value_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "nodeType", dom_node_node_type_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "prefix", dom_node_prefix_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "localName", dom_node_local_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "namespaceURI", dom_node_namespace_uri_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "ownerDocument", dom_node_owner_document_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_namespace_node_prop_handlers, "parentNode", dom_node_parent_node_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_namespace_node_prop_handlers, sizeof(dom_namespace_node_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domdocumentfragment", dom_node_class_entry, php_dom_documentfragment_class_functions, dom_documentfragment_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_node_prop_handlers, sizeof(dom_node_prop_handlers), NULL);
	
	REGISTER_DOM_CLASS(ce, "domdocument", dom_node_class_entry, php_dom_document_class_functions, dom_document_class_entry);
	zend_hash_init(&dom_document_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_document_prop_handlers, "doctype", dom_document_doctype_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "implementation", dom_document_implementation_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "documentElement", dom_document_document_element_read, NULL TSRMLS_CC);
/* actualEncoding currently set as read only alias to encoding
	dom_register_prop_handler(&dom_document_prop_handlers, "actualEncoding", dom_document_actual_encoding_read, dom_document_actual_encoding_write TSRMLS_CC); */
	dom_register_prop_handler(&dom_document_prop_handlers, "actualEncoding", dom_document_encoding_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "encoding", dom_document_encoding_read, dom_document_encoding_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "standalone", dom_document_standalone_read, dom_document_standalone_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "version", dom_document_version_read, dom_document_version_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "strictErrorChecking", dom_document_strict_error_checking_read, dom_document_strict_error_checking_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "documentURI", dom_document_document_uri_read, dom_document_document_uri_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "config", dom_document_config_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "formatOutput", dom_document_format_output_read, dom_document_format_output_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "validateOnParse", dom_document_validate_on_parse_read, dom_document_validate_on_parse_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "resolveExternals", dom_document_resolve_externals_read, dom_document_resolve_externals_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "preserveWhiteSpace", dom_document_preserve_whitespace_read, dom_document_preserve_whitespace_write TSRMLS_CC);
	dom_register_prop_handler(&dom_document_prop_handlers, "substituteEntities", dom_document_substitue_entities_read, dom_document_substitue_entities_write TSRMLS_CC);

	zend_hash_merge(&dom_document_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_document_prop_handlers, sizeof(dom_document_prop_handlers), NULL);

	INIT_CLASS_ENTRY(ce, "domnodelist", php_dom_nodelist_class_functions);
	ce.create_object = dom_nnodemap_objects_new;
	dom_nodelist_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	dom_nodelist_class_entry->get_iterator = php_dom_get_iterator;

	zend_hash_init(&dom_nodelist_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_nodelist_prop_handlers, "length", dom_nodelist_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_nodelist_prop_handlers, sizeof(dom_nodelist_prop_handlers), NULL);

	INIT_CLASS_ENTRY(ce, "domnamednodemap", php_dom_namednodemap_class_functions);
	ce.create_object = dom_nnodemap_objects_new;
	dom_namednodemap_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	dom_namednodemap_class_entry->get_iterator = php_dom_get_iterator;

	zend_hash_init(&dom_namednodemap_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_namednodemap_prop_handlers, "length", dom_namednodemap_length_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_namednodemap_prop_handlers, sizeof(dom_namednodemap_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domcharacterdata", dom_node_class_entry, php_dom_characterdata_class_functions, dom_characterdata_class_entry);
	
	zend_hash_init(&dom_characterdata_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_characterdata_prop_handlers, "data", dom_characterdata_data_read, dom_characterdata_data_write TSRMLS_CC);
	dom_register_prop_handler(&dom_characterdata_prop_handlers, "length", dom_characterdata_length_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_characterdata_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_characterdata_prop_handlers, sizeof(dom_characterdata_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domattr", dom_node_class_entry, php_dom_attr_class_functions, dom_attr_class_entry);
	
	zend_hash_init(&dom_attr_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_attr_prop_handlers, "name", dom_attr_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "specified", dom_attr_specified_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "value", dom_attr_value_read, dom_attr_value_write TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "ownerElement", dom_attr_owner_element_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_attr_prop_handlers, "schemaTypeInfo", dom_attr_schema_type_info_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_attr_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_attr_prop_handlers, sizeof(dom_attr_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domelement", dom_node_class_entry, php_dom_element_class_functions, dom_element_class_entry);
	
	zend_hash_init(&dom_element_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_element_prop_handlers, "tagName", dom_element_tag_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_element_prop_handlers, "schemaTypeInfo", dom_element_schema_type_info_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_element_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_element_prop_handlers, sizeof(dom_element_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domtext", dom_characterdata_class_entry, php_dom_text_class_functions, dom_text_class_entry);
	
	zend_hash_init(&dom_text_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_text_prop_handlers, "wholeText", dom_text_whole_text_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_text_prop_handlers, &dom_characterdata_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_text_prop_handlers, sizeof(dom_text_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domcomment", dom_characterdata_class_entry, php_dom_comment_class_functions, dom_comment_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_characterdata_prop_handlers, sizeof(dom_typeinfo_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domtypeinfo", NULL, php_dom_typeinfo_class_functions, dom_typeinfo_class_entry);
	
	zend_hash_init(&dom_typeinfo_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_typeinfo_prop_handlers, "typeName", dom_typeinfo_type_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_typeinfo_prop_handlers, "typeNamespace", dom_typeinfo_type_namespace_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_typeinfo_prop_handlers, sizeof(dom_typeinfo_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domuserdatahandler", NULL, php_dom_userdatahandler_class_functions, dom_userdatahandler_class_entry);
	REGISTER_DOM_CLASS(ce, "domdomerror", NULL, php_dom_domerror_class_functions, dom_domerror_class_entry);
	
	zend_hash_init(&dom_domerror_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "severity", dom_domerror_severity_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "message", dom_domerror_message_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "type", dom_domerror_type_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "relatedException", dom_domerror_related_exception_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "related_data", dom_domerror_related_data_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domerror_prop_handlers, "location", dom_domerror_location_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domerror_prop_handlers, sizeof(dom_domerror_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domerrorhandler", NULL, php_dom_domerrorhandler_class_functions, dom_domerrorhandler_class_entry);
	REGISTER_DOM_CLASS(ce, "domlocator", NULL, php_dom_domlocator_class_functions, dom_domlocator_class_entry);
	
	zend_hash_init(&dom_domlocator_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "lineNumber", dom_domlocator_line_number_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "columnNumber", dom_domlocator_column_number_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "offset", dom_domlocator_offset_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "relatedNode", dom_domlocator_related_node_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_domlocator_prop_handlers, "uri", dom_domlocator_uri_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_domlocator_prop_handlers, sizeof(dom_domlocator_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domconfiguration", NULL, php_dom_domconfiguration_class_functions, dom_domconfiguration_class_entry);
	REGISTER_DOM_CLASS(ce, "domcdatasection", dom_text_class_entry, php_dom_cdatasection_class_functions, dom_cdatasection_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_text_prop_handlers, sizeof(dom_documenttype_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domdocumenttype", dom_node_class_entry, php_dom_documenttype_class_functions, dom_documenttype_class_entry);
	
	zend_hash_init(&dom_documenttype_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "name", dom_documenttype_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "entities", dom_documenttype_entities_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "notations", dom_documenttype_notations_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "publicId", dom_documenttype_public_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "systemId", dom_documenttype_system_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_documenttype_prop_handlers, "internalSubset", dom_documenttype_internal_subset_read, NULL TSRMLS_CC);
	zend_hash_merge(&dom_documenttype_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_documenttype_prop_handlers, sizeof(dom_documenttype_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domnotation", NULL, php_dom_notation_class_functions, dom_notation_class_entry);
	
	zend_hash_init(&dom_notation_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_notation_prop_handlers, "publicId", dom_notation_public_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_notation_prop_handlers, "systemId", dom_notation_system_id_read, NULL TSRMLS_CC);
	/* Notation nodes are special */
	dom_register_prop_handler(&dom_notation_prop_handlers, "nodeName", dom_node_node_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_notation_prop_handlers, "nodeValue", dom_node_node_value_read, dom_node_node_value_write TSRMLS_CC);
	dom_register_prop_handler(&dom_notation_prop_handlers, "attributes", dom_node_attributes_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_notation_prop_handlers, sizeof(dom_notation_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domentity", dom_node_class_entry, php_dom_entity_class_functions, dom_entity_class_entry);
	
	zend_hash_init(&dom_entity_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_entity_prop_handlers, "publicId", dom_entity_public_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "systemId", dom_entity_system_id_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "notationName", dom_entity_notation_name_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "actualEncoding", dom_entity_actual_encoding_read, dom_entity_actual_encoding_write TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "encoding", dom_entity_encoding_read, dom_entity_encoding_write TSRMLS_CC);
	dom_register_prop_handler(&dom_entity_prop_handlers, "version", dom_entity_version_read, dom_entity_version_write TSRMLS_CC);
	zend_hash_merge(&dom_entity_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);

	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_entity_prop_handlers, sizeof(dom_entity_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domentityreference", dom_node_class_entry, php_dom_entityreference_class_functions, dom_entityreference_class_entry);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_node_prop_handlers, sizeof(dom_entity_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domprocessinginstruction", dom_node_class_entry, php_dom_processinginstruction_class_functions, dom_processinginstruction_class_entry);
	
	zend_hash_init(&dom_processinginstruction_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_processinginstruction_prop_handlers, "target", dom_processinginstruction_target_read, NULL TSRMLS_CC);
	dom_register_prop_handler(&dom_processinginstruction_prop_handlers, "data", dom_processinginstruction_data_read, dom_processinginstruction_data_write TSRMLS_CC);
	zend_hash_merge(&dom_processinginstruction_prop_handlers, &dom_node_prop_handlers, NULL, NULL, sizeof(dom_prop_handler), 0);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_processinginstruction_prop_handlers, sizeof(dom_processinginstruction_prop_handlers), NULL);

	REGISTER_DOM_CLASS(ce, "domstring_extend", NULL, php_dom_string_extend_class_functions, dom_string_extend_class_entry);

#if defined(LIBXML_XPATH_ENABLED)
	INIT_CLASS_ENTRY(ce, "domxpath", php_dom_xpath_class_functions);
	ce.create_object = dom_xpath_objects_new;
	dom_xpath_class_entry = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_hash_init(&dom_xpath_prop_handlers, 0, NULL, NULL, 1);
	dom_register_prop_handler(&dom_xpath_prop_handlers, "document", dom_xpath_document_read, NULL TSRMLS_CC);
	zend_hash_add(&classes, ce.name, ce.name_length + 1, &dom_xpath_prop_handlers, sizeof(dom_xpath_prop_handlers), NULL);
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

	/* domException Codes */
	REGISTER_LONG_CONSTANT("DOM_INDEX_SIZE_ERR",		INDEX_SIZE_ERR,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOMSTRING_SIZE_ERR",		DOMSTRING_SIZE_ERR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_HIERARCHY_REQUEST_ERR",	HIERARCHY_REQUEST_ERR,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_WRONG_DOCUMENT_ERR",	WRONG_DOCUMENT_ERR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_CHARACTER_ERR",	INVALID_CHARACTER_ERR,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NO_DATA_ALLOWED_ERR",	NO_DATA_ALLOWED_ERR,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NO_MODIFICATION_ALLOWED_ERR", NO_MODIFICATION_ALLOWED_ERR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NOT_FOUND_ERR",			NOT_FOUND_ERR,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NOT_SUPPORTED_ERR",		NOT_SUPPORTED_ERR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INUSE_ATTRIBUTE_ERR",	INUSE_ATTRIBUTE_ERR,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_STATE_ERR",		INVALID_STATE_ERR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_SYNTAX_ERR",			SYNTAX_ERR,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_MODIFICATION_ERR",	INVALID_MODIFICATION_ERR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NAMESPACE_ERR",			NAMESPACE_ERR,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_ACCESS_ERR",	INVALID_ACCESS_ERR,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_VALIDATION_ERR",		VALIDATION_ERR,			CONST_CS | CONST_PERSISTENT);

	php_libxml_initialize();

	return SUCCESS;
}
/* }}} */

/* {{{ */
PHP_MINFO_FUNCTION(dom)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "DOM/XML", "enabled");
	php_info_print_table_row(2, "DOM/XML API Version", DOM_API_VERSION);
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
#ifdef LIBXML_SCHEMAS_ENABLED
	php_info_print_table_row(2, "Schema Support", "enabled");
	php_info_print_table_row(2, "RelaxNG Support", "enabled");
#endif
	php_info_print_table_end();
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(dom)
{
	php_libxml_shutdown();

	zend_hash_destroy(&dom_domstringlist_prop_handlers);
	zend_hash_destroy(&dom_namelist_prop_handlers);
	zend_hash_destroy(&dom_domimplementationlist_prop_handlers);
	zend_hash_destroy(&dom_document_prop_handlers);
	zend_hash_destroy(&dom_node_prop_handlers);
	zend_hash_destroy(&dom_namespace_node_prop_handlers);
	zend_hash_destroy(&dom_nodelist_prop_handlers);
	zend_hash_destroy(&dom_namednodemap_prop_handlers);
	zend_hash_destroy(&dom_characterdata_prop_handlers);
	zend_hash_destroy(&dom_attr_prop_handlers);
	zend_hash_destroy(&dom_element_prop_handlers);
	zend_hash_destroy(&dom_text_prop_handlers);
	zend_hash_destroy(&dom_typeinfo_prop_handlers);
	zend_hash_destroy(&dom_domerror_prop_handlers);
	zend_hash_destroy(&dom_domlocator_prop_handlers);
	zend_hash_destroy(&dom_documenttype_prop_handlers);
	zend_hash_destroy(&dom_notation_prop_handlers);
	zend_hash_destroy(&dom_entity_prop_handlers);
	zend_hash_destroy(&dom_processinginstruction_prop_handlers);
#if defined(LIBXML_XPATH_ENABLED)
	zend_hash_destroy(&dom_xpath_prop_handlers);
#endif
	zend_hash_destroy(&classes);
	
/*	If you want do find memleaks in this module, compile libxml2 with --with-mem-debug and
	uncomment the following line, this will tell you the amount of not freed memory
	and the total used memory into apaches error_log  */
/*  xmlMemoryDump();*/

	return SUCCESS;
}

/* {{{ node_list_unlink */
void node_list_unlink(xmlNodePtr node TSRMLS_DC)
{
	dom_object *wrapper;

	while (node != NULL) {

		wrapper = php_dom_object_get_data(node);

		if (wrapper != NULL ) {
			xmlUnlinkNode(node);
		} else {
			node_list_unlink(node->children TSRMLS_CC);

			switch (node->type) {
				case XML_ATTRIBUTE_DECL:
				case XML_DTD_NODE:
				case XML_DOCUMENT_TYPE_NODE:
				case XML_ENTITY_DECL:
				case XML_ATTRIBUTE_NODE:
					break;
				default:
					node_list_unlink((xmlNodePtr) node->properties TSRMLS_CC);
			}

		}

		node = node->next;
	}
}
/* }}} end node_list_unlink */

/* {{{ dom_objects_clone */
void dom_objects_clone(void *object, void **object_clone TSRMLS_DC)
{
	/* TODO */
}
/* }}} */

#if defined(LIBXML_XPATH_ENABLED)
/* {{{ dom_xpath_objects_dtor */
void dom_xpath_objects_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	dom_object *intern = (dom_object *)object;

	zend_hash_destroy(intern->std.properties);
	FREE_HASHTABLE(intern->std.properties);

	if (intern->ptr != NULL) {
		xmlXPathFreeContext((xmlXPathContextPtr) intern->ptr);
		php_libxml_decrement_doc_ref((php_libxml_node_object *) intern TSRMLS_CC);
		intern->ptr = NULL;
	}

	efree(object);
}
/* }}} */
#endif

/* {{{ dom_objects_dtor */
void dom_objects_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	dom_object *intern = (dom_object *)object;
	int retcount;

	zend_hash_destroy(intern->std.properties);
	FREE_HASHTABLE(intern->std.properties);

	if (intern->ptr != NULL && ((php_libxml_node_ptr *)intern->ptr)->node != NULL) {
		if (((xmlNodePtr) ((php_libxml_node_ptr *)intern->ptr)->node)->type != XML_DOCUMENT_NODE && ((xmlNodePtr) ((php_libxml_node_ptr *)intern->ptr)->node)->type != XML_HTML_DOCUMENT_NODE) {
			// php_libxml_node_free_resource(dom_object_get_node(intern) TSRMLS_CC);
			php_libxml_node_decrement_resource((php_libxml_node_object *) intern TSRMLS_CC);
		} else {
			php_libxml_decrement_node_ptr((php_libxml_node_object *) intern TSRMLS_CC);
			retcount = php_libxml_decrement_doc_ref((php_libxml_node_object *)intern TSRMLS_CC);
		}
		intern->ptr = NULL;
	}

	efree(object);
}
/* }}} */

void dom_namednode_iter(dom_object *basenode, int ntype, dom_object *intern, xmlHashTablePtr ht, xmlChar *local, xmlChar *ns)
{
	dom_nnodemap_object *mapptr;
	zval *baseobj = NULL;

	mapptr = (dom_nnodemap_object *)intern->ptr;
	if (basenode) {
		MAKE_STD_ZVAL(baseobj);
		baseobj->type = IS_OBJECT;
		baseobj->is_ref = 1;
		baseobj->value.obj.handle = basenode->handle;
		baseobj->value.obj.handlers = &dom_object_handlers;
		zval_copy_ctor(baseobj);
	}
	mapptr->baseobjptr = baseobj;
	mapptr->baseobj = basenode;
	mapptr->nodetype = ntype;
	mapptr->ht = ht;
	mapptr->local = local;
	mapptr->ns = ns;

}

static dom_object* dom_objects_set_class(zend_class_entry *class_type TSRMLS_DC)
{
	zend_class_entry *base_class;
	zval *tmp;
	dom_object *intern;

	intern = emalloc(sizeof(dom_object));
	intern->std.ce = class_type;
	intern->std.in_get = 0;
	intern->std.in_set = 0;
	intern->ptr = NULL;
	intern->prop_handler = NULL;
	intern->document = NULL;

	base_class = class_type;
	while(base_class->type != ZEND_INTERNAL_CLASS && base_class->parent != NULL) {
		base_class = base_class->parent;
	}

	zend_hash_find(&classes, base_class->name, base_class->name_length + 1, (void **) &intern->prop_handler);

	ALLOC_HASHTABLE(intern->std.properties);
	zend_hash_init(intern->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	return intern;
}

/* {{{ dom_objects_new */
zend_object_value dom_objects_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	dom_object *intern;
	
	intern = dom_objects_set_class(class_type TSRMLS_CC);

	retval.handle = zend_objects_store_put(intern, dom_objects_dtor, dom_objects_clone TSRMLS_CC);
	intern->handle = retval.handle;
	retval.handlers = &dom_object_handlers;

	return retval;
}
/* }}} */

#if defined(LIBXML_XPATH_ENABLED)
/* {{{ zend_object_value dom_xpath_objects_new(zend_class_entry *class_type TSRMLS_DC) */
zend_object_value dom_xpath_objects_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	dom_object *intern;
	
	intern = dom_objects_set_class(class_type TSRMLS_CC);

	retval.handle = zend_objects_store_put(intern, dom_xpath_objects_dtor, dom_objects_clone TSRMLS_CC);
	intern->handle = retval.handle;
	retval.handlers = &dom_object_handlers;

	return retval;
}
/* }}} */
#endif

void dom_nnodemap_objects_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	dom_nnodemap_object *objmap;
	zval *baseobj;
	dom_object *intern = (dom_object *)object;

	php_libxml_decrement_doc_ref((php_libxml_node_object *)intern TSRMLS_CC);
	objmap = (dom_nnodemap_object *)intern->ptr;
	if (objmap) {
		if (objmap->local) {
			xmlFree(objmap->local);
		}
		if (objmap->ns) {
			xmlFree(objmap->ns);
		}
		if (objmap->baseobjptr) {
			baseobj = objmap->baseobjptr;
			zval_ptr_dtor((zval **)&baseobj);
		}
		efree(objmap);
	}

	zend_hash_destroy(intern->std.properties);
	FREE_HASHTABLE(intern->std.properties);

	efree(object);
}

zend_object_value dom_nnodemap_objects_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	dom_object *intern;
	dom_nnodemap_object *objmap;
	
	intern = dom_objects_set_class(class_type TSRMLS_CC);
	intern->ptr = emalloc(sizeof(dom_nnodemap_object));
	objmap = (dom_nnodemap_object *)intern->ptr;
	objmap->baseobj = NULL;
	objmap->baseobjptr = NULL;
	objmap->nodetype = 0;
	objmap->ht = NULL;
	objmap->local = NULL;
	objmap->ns = NULL;

	retval.handle = zend_objects_store_put(intern, dom_nnodemap_objects_dtor, dom_objects_clone TSRMLS_CC);
	intern->handle = retval.handle;
	retval.handlers = &dom_object_handlers;

	return retval;
}

void php_dom_create_interator(zval *return_value, int ce_type TSRMLS_DC)
{
	zend_class_entry *ce;

	if (ce_type == DOM_NAMEDNODEMAP) {
		ce = dom_namednodemap_class_entry;
	} else {
		ce = dom_nodelist_class_entry;
	}

	object_init_ex(return_value, ce);
}

/* {{{ php_dom_create_object */
zval *php_dom_create_object(xmlNodePtr obj, int *found, zval *wrapper_in, zval *return_value, dom_object *domobj TSRMLS_DC)
{
	zval *wrapper;
	zend_class_entry *ce;
	dom_object *intern;

	*found = 0;

	if (!obj) {
		ALLOC_ZVAL(wrapper);
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((intern = (dom_object *) php_dom_object_get_data((void *) obj))) {
		return_value->type = IS_OBJECT;
		return_value->is_ref = 1;
		return_value->value.obj.handle = intern->handle;
		return_value->value.obj.handlers = &dom_object_handlers;
		zval_copy_ctor(return_value);
		*found = 1;
		return return_value;
	}

	wrapper = return_value;

	switch (obj->type) {
		case XML_DOCUMENT_NODE:
		case XML_HTML_DOCUMENT_NODE:
		{
			ce = dom_document_class_entry;
			break;
		}
		case XML_DTD_NODE:
		case XML_DOCUMENT_TYPE_NODE:
		{
			ce = dom_documenttype_class_entry;
			break;
		}
		case XML_ELEMENT_NODE:
		{
			ce = dom_element_class_entry;
			break;
		}
		case XML_ATTRIBUTE_NODE:
		{
			ce = dom_attr_class_entry;
			break;
		}
		case XML_TEXT_NODE:
		{
			ce = dom_text_class_entry;
			break;
		}
		case XML_COMMENT_NODE:
		{
			ce = dom_comment_class_entry;
			break;
		}
		case XML_PI_NODE:
		{
			ce = dom_processinginstruction_class_entry;
			break;
		}
		case XML_ENTITY_REF_NODE:
		{
			ce = dom_entityreference_class_entry;
			break;
		}
		case XML_ENTITY_DECL:
		case XML_ELEMENT_DECL:
		{
			ce = dom_entity_class_entry;
			break;
		}
		case XML_CDATA_SECTION_NODE:
		{
			ce = dom_cdatasection_class_entry;
			break;
		}
		case XML_DOCUMENT_FRAG_NODE:
		{
			ce = dom_documentfragment_class_entry;
			break;
		}
		case XML_NOTATION_NODE:
		{
			ce = dom_notation_class_entry;
			break;
		}
		case XML_NAMESPACE_DECL:
		{
			ce = dom_namespace_node_class_entry;
			break;
		}
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported node type: %d\n", Z_TYPE_P(obj));
			ZVAL_NULL(wrapper);
			return wrapper;
	}

	object_init_ex(wrapper, ce);

	intern = (dom_object *)zend_objects_get_address(wrapper TSRMLS_CC);
	if (obj->doc != NULL) {
		if (domobj != NULL) {
			intern->document = domobj->document;
		}
		php_libxml_increment_doc_ref((php_libxml_node_object *)intern, obj->doc TSRMLS_CC);
	}

	php_libxml_increment_node_ptr((php_libxml_node_object *)intern, obj, (void *)intern TSRMLS_CC);
	return (wrapper);
}
/* }}} end php_domobject_new */


void php_dom_create_implementation(zval **retval  TSRMLS_DC) {
	object_init_ex(*retval, dom_domimplementation_class_entry);
}

/* {{{ int dom_hierarchy(xmlNodePtr parent, xmlNodePtr child) */
int dom_hierarchy(xmlNodePtr parent, xmlNodePtr child) 
{
	xmlNodePtr nodep;

    if (parent == NULL || child == NULL || child->doc != parent->doc) {
        return SUCCESS;
    }

	nodep = parent;

	while (nodep) {
		if (nodep == child) {
			return FAILURE;
		}
		nodep = nodep->parent;
	}

    return SUCCESS;
}
/* }}} end dom_hierarchy */

/* {{{ dom_has_feature(char *feature, char *version) */
int dom_has_feature(char *feature, char *version)
{
	int retval = 0;

	if (!(strcmp (version, "1.0") && strcmp (version,"2.0") && strcmp(version, ""))) {
		if ((!strcasecmp(feature, "Core") && strcmp (version, "1.0")) || !strcasecmp(feature, "XML"))
			retval = 1;
	}

	return retval;
}
/* }}} end dom_has_feature */

xmlNode *dom_get_elements_by_tag_name_ns_raw(xmlNodePtr nodep, char *ns, char *local, int *cur, int index)
{
	xmlNodePtr ret = NULL;

	while (nodep != NULL && (*cur <= index || index == -1)) {
		if (nodep->type == XML_ELEMENT_NODE && xmlStrEqual(nodep->name, local)) {
			if (ns == NULL || (nodep->ns != NULL && xmlStrEqual(nodep->ns->href, ns))) {
				if (*cur == index) {
					ret = nodep;
					break;
				}
				(*cur)++;
			}
		}
		ret = dom_get_elements_by_tag_name_ns_raw(nodep->children, ns, local, cur, index);
		if (ret != NULL) {
			break;
		}
		nodep = nodep->next;
	}
	return ret;
}
/* }}} end dom_element_get_elements_by_tag_name_ns_raw */


/* {{{ void dom_normalize (xmlNodePtr nodep TSRMLS_DC) */
void dom_normalize (xmlNodePtr nodep TSRMLS_DC)
{
	xmlNodePtr child, nextp, newnextp;
	xmlAttrPtr attr;
	xmlChar	*strContent; 

	child = nodep->children;
	while(child != NULL) {
		switch (child->type) {
			case XML_TEXT_NODE:
				nextp = child->next;
				while (nextp != NULL) {
					if (nextp->type == XML_TEXT_NODE) {
						newnextp = nextp->next;
						strContent = xmlNodeGetContent(nextp);
						xmlNodeAddContent(child, strContent);
						xmlFree(strContent);
						xmlUnlinkNode(nextp);
						php_libxml_node_free_resource(nextp TSRMLS_CC);
						nextp = newnextp;
					} else {
						break;
					}
				}
				break;
			case XML_ELEMENT_NODE:
				dom_normalize (child TSRMLS_CC);
				attr = child->properties;
				while (attr != NULL) {
					dom_normalize((xmlNodePtr) attr TSRMLS_CC);
					attr = attr->next;
				}
				break;
			case XML_ATTRIBUTE_NODE:
				dom_normalize (child TSRMLS_CC);
				break;
			default:
				break;
		}
		child = child->next;
	}
}
/* }}} end dom_normalize */


/* {{{ void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) */
void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) {
	xmlNs *cur;

	if (doc == NULL)
		return;

	if (doc->oldNs == NULL) {
		doc->oldNs = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
		if (doc->oldNs == NULL) {
			return;
		}
		memset(doc->oldNs, 0, sizeof(xmlNs));
		doc->oldNs->type = XML_LOCAL_NAMESPACE;
		doc->oldNs->href = xmlStrdup(XML_XML_NAMESPACE); 
		doc->oldNs->prefix = xmlStrdup((const xmlChar *)"xml"); 
	}

	cur = doc->oldNs;
	while (cur->next != NULL) {
		cur = cur->next;
	}
	cur->next = ns;
}
/* }}} end dom_set_old_ns */

int dom_check_qname(char *qname, char **localname, char **prefix, int uri_len, int name_len) {
	int errorcode = 0;

	if (name_len > 0) {
		*localname = xmlSplitQName2(qname, (xmlChar **) prefix);
		if (*localname == NULL) {
			*localname = xmlStrdup(qname);
			if (*prefix == NULL && uri_len == 0) {
				return errorcode;
			}
		}
		if (uri_len == 0 || *localname == NULL || (xmlStrchr(*localname, (xmlChar) ':') != NULL)) {
			errorcode = NAMESPACE_ERR;
		}
	} else {
		errorcode = NAMESPACE_ERR;
	}

	return errorcode;
}

/* {{{ xmlNsPtr dom_get_ns(xmlNodePtr nodep, char *uri, int *errorcode, char *prefix) */
xmlNsPtr dom_get_ns(xmlNodePtr nodep, char *uri, int *errorcode, char *prefix) {
	xmlNsPtr nsptr = NULL;

	*errorcode = 0;

	if (! (prefix && !strcmp (prefix, "xml") && strcmp(uri, XML_XML_NAMESPACE))) {
		nsptr = xmlNewNs(nodep, uri, prefix);
	}

	if (nsptr == NULL) {
		*errorcode = NAMESPACE_ERR;
	}

	return nsptr;

}
/* }}} end dom_get_ns */

/* {{{ xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) */
xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) {
	xmlNsPtr cur;
	xmlNs *ret = NULL;
	if (node == NULL)
		return NULL;

	if (localName == NULL || xmlStrEqual(localName, "")) {
		cur = node->nsDef;
		while (cur != NULL) {
			if (cur->prefix == NULL  && cur->href != NULL) {
				ret = cur;
				break;
			}
			cur = cur->next;
		}
	} else {
		cur = node->nsDef;
		while (cur != NULL) {
			if (cur->prefix != NULL && xmlStrEqual(localName, cur->prefix)) {
				ret = cur;
				break;
			}
			cur = cur->next;
		}
	}
	return ret;
}
/* }}} end dom_get_nsdecl */

#endif /* HAVE_DOM */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
