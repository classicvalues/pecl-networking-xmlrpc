/*
  This file is part of, or distributed with, libXMLRPC - a C library for 
  xml-encoded function calls.

  Author: Dan Libby (dan@libby.com)
  Epinions.com may be contacted at feedback@epinions-inc.com
*/

/*  
  Copyright 2001 Epinions, Inc. 

  Subject to the following 3 conditions, Epinions, Inc.  permits you, free 
  of charge, to (a) use, copy, distribute, modify, perform and display this 
  software and associated documentation files (the "Software"), and (b) 
  permit others to whom the Software is furnished to do so as well.  

  1) The above copyright notice and this permission notice shall be included 
  without modification in all copies or substantial portions of the 
  Software.  

  2) THE SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY OR CONDITION OF 
  ANY KIND, EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION ANY 
  IMPLIED WARRANTIES OF ACCURACY, MERCHANTABILITY, FITNESS FOR A PARTICULAR 
  PURPOSE OR NONINFRINGEMENT.  

  3) IN NO EVENT SHALL EPINIONS, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, 
  SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT 
  OF OR IN CONNECTION WITH THE SOFTWARE (HOWEVER ARISING, INCLUDING 
  NEGLIGENCE), EVEN IF EPINIONS, INC.  IS AWARE OF THE POSSIBILITY OF SUCH 
  DAMAGES.    

*/

/* auto-generated portions of this file are also subject to the php license */

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
   | Authors:                                                             |
   |                                                                      |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "php_ini.h"
#include "php_xmlrpc.h"
#include "xmlrpc.h"

/* You should tweak config.m4 so this symbol (or some else suitable)
   gets defined.
*/

#ifdef ZTS
int xmlrpc_globals_id;
#else
php_xmlrpc_globals xmlrpc_globals;
#endif


/* Every user visible function must have an entry in xmlrpc_functions[].
*/
function_entry xmlrpc_functions[] = {
   PHP_FE(xmlrpc_encode,    NULL) 
   PHP_FE(xmlrpc_decode,    NULL)
   PHP_FE(xmlrpc_decode_request, NULL)
   PHP_FE(xmlrpc_encode_request, NULL)
   PHP_FE(xmlrpc_get_type,    NULL)
   PHP_FE(xmlrpc_set_type,    NULL)
   PHP_FE(xmlrpc_server_create, NULL)
   PHP_FE(xmlrpc_server_destroy, NULL)
   PHP_FE(xmlrpc_server_register_method, NULL)
   PHP_FE(xmlrpc_server_call_method, NULL)
   PHP_FE(xmlrpc_parse_method_descriptions, NULL)
   PHP_FE(xmlrpc_server_add_introspection_data, NULL)
   PHP_FE(xmlrpc_server_register_introspection_callback, NULL)
   {NULL, NULL, NULL}      /* Must be the last line in xmlrpc_functions[] */
};

zend_module_entry xmlrpc_module_entry = {
   "xmlrpc",
   xmlrpc_functions,
   PHP_MINIT(xmlrpc),
   PHP_MSHUTDOWN(xmlrpc),
   PHP_RINIT(xmlrpc),      /* Replace with NULL if there's nothing to do at request start */
   PHP_RSHUTDOWN(xmlrpc),  /* Replace with NULL if there's nothing to do at request end */
   PHP_MINFO(xmlrpc),
   STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_XMLRPC
ZEND_GET_MODULE(xmlrpc)
#endif

/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
PHP_INI_END()
*/

/*******************************
* local structures and defines *
*******************************/

// per server data
typedef struct _xmlrpc_server_data {
   pval* method_map;
   pval* introspection_map;
   XMLRPC_SERVER server_ptr;
} xmlrpc_server_data;


// how to format output
typedef struct _php_output_options {
   int b_php_out;
   STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS xmlrpc_out;
} php_output_options;

// data passed to C callback
typedef struct _xmlrpc_callback_data {
   pval* xmlrpc_method;
   pval* php_function;
   pval* caller_params;
   pval* return_data;
   xmlrpc_server_data* server;
   char php_executed;
} xmlrpc_callback_data;

#define PHP_EXT_VERSION "0.41"

// output options
#define OUTPUT_TYPE_KEY "output_type"
#define OUTPUT_TYPE_KEY_LEN (sizeof(OUTPUT_TYPE_KEY) - 1)
#define OUTPUT_TYPE_VALUE_PHP "php"
#define OUTPUT_TYPE_VALUE_XML "xml"

#define VERBOSITY_KEY "verbosity"
#define VERBOSITY_KEY_LEN (sizeof(VERBOSITY_KEY) - 1)
#define VERBOSITY_VALUE_NO_WHITE_SPACE "no_white_space"
#define VERBOSITY_VALUE_NEWLINES_ONLY "newlines_only"
#define VERBOSITY_VALUE_PRETTY "pretty"

#define ESCAPING_KEY "escaping"
#define ESCAPING_KEY_LEN (sizeof(ESCAPING_KEY) - 1)
#define ESCAPING_VALUE_CDATA "cdata"
#define ESCAPING_VALUE_NON_ASCII "non-ascii"
#define ESCAPING_VALUE_NON_PRINT "non-print"
#define ESCAPING_VALUE_MARKUP "markup"

#define VERSION_KEY "version"
#define VERSION_KEY_LEN (sizeof(VERSION_KEY) - 1)
#define VERSION_VALUE_SIMPLE "simple"
#define VERSION_VALUE_XMLRPC "xmlrpc"

#define ENCODING_KEY "encoding"
#define ENCODING_KEY_LEN (sizeof(ENCODING_KEY) - 1)
#define ENCODING_DEFAULT "iso-8859-1"

// value types
#define OBJECT_TYPE_ATTR "xmlrpc_type"
#define OBJECT_VALUE_ATTR "scalar"



/***********************
* forward declarations *
***********************/
XMLRPC_VALUE_TYPE get_pval_xmlrpc_type(pval* value, pval** newvalue);
static void php_xmlrpc_introspection_callback(XMLRPC_SERVER server, void* data);

/*********************
* startup / shutdown *
*********************/

static void destroy_server_data(xmlrpc_server_data *server) {
   if(server) {
      XMLRPC_ServerDestroy(server->server_ptr);

      zval_dtor(server->method_map);
      FREE_ZVAL(server->method_map);

      zval_dtor(server->introspection_map);
      FREE_ZVAL(server->introspection_map);

      efree(server);
   }
}

/* called when server is being destructed. either when xmlrpc_server_destroy
 * is called, or when request ends.
 */
static void xmlrpc_server_destructor(zend_rsrc_list_entry *rsrc) {
   if(rsrc && rsrc->ptr) {
      destroy_server_data((xmlrpc_server_data*)rsrc->ptr);
   }
}

/* module init */
PHP_MINIT_FUNCTION(xmlrpc)
{
/* Remove comments if you have entries in php.ini
        REGISTER_INI_ENTRIES();
*/
   XMLRPCG(le_xmlrpc_server) = zend_register_list_destructors_ex(xmlrpc_server_destructor, NULL, "xmlrpc server", module_number);

   return SUCCESS;
}

/* module shutdown */
PHP_MSHUTDOWN_FUNCTION(xmlrpc)
{
/* Remove comments if you have entries in php.ini
        UNREGISTER_INI_ENTRIES();
*/
   return SUCCESS;
}

/* Remove if there's nothing to do at request start */
PHP_RINIT_FUNCTION(xmlrpc)
{
   return SUCCESS;
}

/* Remove if there's nothing to do at request end */
PHP_RSHUTDOWN_FUNCTION(xmlrpc)
{
   return SUCCESS;
}

/* display info in phpinfo() */
PHP_MINFO_FUNCTION(xmlrpc)
{
   php_info_print_table_start();
   php_info_print_table_row(2, "core library version", XMLRPC_GetVersionString());
   php_info_print_table_row(2, "php extension version", PHP_EXT_VERSION);
   php_info_print_table_row(2, "author", "Dan Libby");
   php_info_print_table_row(2, "homepage", "http://xmlrpc-epi.sourceforge.net");
   php_info_print_table_row(2, "open sourced by", "Epinions.com");
   php_info_print_table_end();
   
	/*
   DISPLAY_INI_ENTRIES();
	*/
}

/*******************
* random utilities *
*******************/

/* Utility functions for adding data types to arrays, with or without key (assoc, non-assoc).
 * Could easily be further generalized to work with objects.
 */
static int add_long(pval* list, char* id, int num) {
   if(id) return add_assoc_long(list, id, num);
   else   return add_next_index_long(list, num);
}

static int add_double(pval* list, char* id, double num) {
   if(id) return add_assoc_double(list, id, num);
   else   return add_next_index_double(list, num);
}

static int add_string(pval* list, char* id, char* string, int duplicate) {
   if(id) return add_assoc_string(list, id, string, duplicate);
   else   return add_next_index_string(list, string, duplicate);
}

static int add_stringl(pval* list, char* id, char* string, uint length, int duplicate) {
   if(id) return add_assoc_stringl(list, id, string, length, duplicate);
   else   return add_next_index_stringl(list, string, length, duplicate);
}

static int add_pval(pval* list, const char* id, pval** val) {
   if(list && val) {
      if(id) return zend_hash_update(list->value.ht, (char*)id, strlen(id)+1, (void *) val, sizeof(pval **), NULL);
      else   return zend_hash_next_index_insert(list->value.ht, (void *) val, sizeof(pval **), NULL); 
   }
}

#if ZEND_MODULE_API_NO >= 20001222
#define my_zend_hash_get_current_key(ht, my_key, num_index) zend_hash_get_current_key(ht, my_key, num_index, 0)
#else
#define my_zend_hash_get_current_key(ht, my_key, num_index) zend_hash_get_current_key(ht, my_key, num_index)
#endif 


/*************************
* input / output options *
*************************/

/* parse an array (user input) into output options suitable for use by xmlrpc engine
 * and determine whether to return data as xml or php vars
 */
static void set_output_options(php_output_options* options, pval* output_opts) {

   if(options) {

      /* defaults */
      options->b_php_out = 0;
      options->xmlrpc_out.version = xmlrpc_version_1_0;
      options->xmlrpc_out.xml_elem_opts.encoding = ENCODING_DEFAULT;
      options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_pretty;
      options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_markup_escaping | xml_elem_non_ascii_escaping | xml_elem_non_print_escaping;

     if(output_opts && output_opts->type == IS_ARRAY) {
        pval** val;

        /* verbosity of generated xml */
        if(zend_hash_find(output_opts->value.ht, 
                          OUTPUT_TYPE_KEY, OUTPUT_TYPE_KEY_LEN + 1, 
                          (void**)&val) == SUCCESS) {
           if((*val)->type == IS_STRING) {
              if(!strcmp((*val)->value.str.val, OUTPUT_TYPE_VALUE_PHP)) {
                 options->b_php_out = 1;
              }
              else if(!strcmp((*val)->value.str.val, OUTPUT_TYPE_VALUE_XML)) {
                 options->b_php_out = 0;
              }
           }
        }

        /* verbosity of generated xml */
        if(zend_hash_find(output_opts->value.ht, 
                          VERBOSITY_KEY, VERBOSITY_KEY_LEN + 1, 
                          (void**)&val) == SUCCESS) {
           if((*val)->type == IS_STRING) {
              if(!strcmp((*val)->value.str.val, VERBOSITY_VALUE_NO_WHITE_SPACE)) {
                 options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_no_white_space;
              }
              else if(!strcmp((*val)->value.str.val, VERBOSITY_VALUE_NEWLINES_ONLY)) {
                 options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_newlines_only;
              }
              else if(!strcmp((*val)->value.str.val, VERBOSITY_VALUE_PRETTY)) {
                 options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_pretty;
              }
           }
        }

        /* version of xml to output */
        if(zend_hash_find(output_opts->value.ht, 
                          VERSION_KEY, VERSION_KEY_LEN + 1, 
                          (void**)&val) == SUCCESS) {
           if((*val)->type == IS_STRING) {
              if(!strcmp((*val)->value.str.val, VERSION_VALUE_XMLRPC)) {
                 options->xmlrpc_out.version = xmlrpc_version_1_0;
              }
              else if(!strcmp((*val)->value.str.val, VERSION_VALUE_SIMPLE)) {
                 options->xmlrpc_out.version = xmlrpc_version_simple;
              }
           }
        }

        /* encoding code set */
        if(zend_hash_find(output_opts->value.ht, 
                          ENCODING_KEY, ENCODING_KEY_LEN + 1, 
                          (void**)&val) == SUCCESS) {
           if((*val)->type == IS_STRING) {
              options->xmlrpc_out.xml_elem_opts.encoding = estrdup((*val)->value.str.val);
           }
        }

        /* escaping options */
        if(zend_hash_find(output_opts->value.ht, 
                          ESCAPING_KEY, ESCAPING_KEY_LEN + 1, 
                          (void**)&val) == SUCCESS) {
           /* multiple values allowed.  check if array */
           if((*val)->type == IS_ARRAY) {
              pval** iter_val;
              zend_hash_internal_pointer_reset((*val)->value.ht);
              options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_no_escaping;
              while(1) {
                 if(zend_hash_get_current_data((*val)->value.ht, (void**)&iter_val) == SUCCESS) {
                    if((*iter_val)->type == IS_STRING && (*iter_val)->value.str.val) {
                       if(!strcmp((*iter_val)->value.str.val, ESCAPING_VALUE_CDATA)) {
                          options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_cdata_escaping;
                       }
                       else if(!strcmp((*iter_val)->value.str.val, ESCAPING_VALUE_NON_ASCII)) {
                          options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_non_ascii_escaping;
                       }
                       else if(!strcmp((*iter_val)->value.str.val, ESCAPING_VALUE_NON_PRINT)) {
                          options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_non_print_escaping;
                       }
                       else if(!strcmp((*iter_val)->value.str.val, ESCAPING_VALUE_MARKUP)) {
                          options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_markup_escaping;
                       }
                    }
                 }
                 else {
                    break;
                 }

                 zend_hash_move_forward((*val)->value.ht);
              }
           }
           /* else, check for single value */
           else if((*val)->type == IS_STRING) {
              if(!strcmp((*val)->value.str.val, ESCAPING_VALUE_CDATA)) {
                 options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_cdata_escaping;
              }
              else if(!strcmp((*val)->value.str.val, ESCAPING_VALUE_NON_ASCII)) {
                 options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_non_ascii_escaping;
              }
              else if(!strcmp((*val)->value.str.val, ESCAPING_VALUE_NON_PRINT)) {
                 options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_non_print_escaping;
              }
              else if(!strcmp((*val)->value.str.val, ESCAPING_VALUE_MARKUP)) {
                 options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_markup_escaping;
              }
           }
        }
     }
   }
}


/******************
* encode / decode *
******************/

/* php arrays have no distinction between array and struct types.
 * they even allow mixed.  Thus, we determine the type by iterating
 * through the entire array and figuring out each element.
 * room for some optimation here if we stop after a specific # of elements.
 */
static XMLRPC_VECTOR_TYPE determine_vector_type(HashTable *ht) {
    int bArray = 0, bStruct = 0, bMixed = 0;
    unsigned long num_index;
    char* my_key;

    zend_hash_internal_pointer_reset(ht);
    while(1) {
       int res = my_zend_hash_get_current_key(ht, &my_key, &num_index);
       if(res == HASH_KEY_IS_LONG) {
           if(bStruct) {
               bMixed = 1;
               break;
           }
           bArray = 1;
       }
       else if(res == HASH_KEY_NON_EXISTANT) {
          break;
       }
       else if(res == HASH_KEY_IS_STRING) {
           if(bArray) {
               bMixed = 1;
               break;
           }
           bStruct = 1;
       }

       zend_hash_move_forward(ht);
    }
    return bMixed ? xmlrpc_vector_mixed : (bStruct ? xmlrpc_vector_struct : xmlrpc_vector_array);
}

/* recursively convert php values into xmlrpc values */
static XMLRPC_VALUE PHP_to_XMLRPC_worker(const char* key, pval* in_val, int depth) {
   XMLRPC_VALUE xReturn = NULL;
   if(in_val) {
      pval* val = NULL;
      XMLRPC_VALUE_TYPE type = get_pval_xmlrpc_type(in_val, &val);
      if(val) {
         switch(type) {
            case xmlrpc_base64:
               if(val->type == IS_NULL) {
                  xReturn = XMLRPC_CreateValueBase64(key, "", 1);
               }
               else {
                  xReturn = XMLRPC_CreateValueBase64(key, val->value.str.val, val->value.str.len);
               }
               break;
            case xmlrpc_datetime:
               convert_to_string(val);
               xReturn = XMLRPC_CreateValueDateTime_ISO8601(key, val->value.str.val);
               break;
            case xmlrpc_boolean:
               convert_to_boolean(val);
               xReturn = XMLRPC_CreateValueBoolean(key, val->value.lval);
               break;
            case xmlrpc_int:
               convert_to_long(val);
               xReturn = XMLRPC_CreateValueInt(key, val->value.lval);
               break;
            case xmlrpc_double:
               convert_to_double(val);
               xReturn = XMLRPC_CreateValueDouble(key, val->value.dval);
               break;
            case xmlrpc_string:
               convert_to_string(val);
               xReturn = XMLRPC_CreateValueString(key, val->value.str.val, val->value.str.len);
               break;
            case xmlrpc_vector:
               {
                  unsigned long num_index;
                  pval** pIter;
                  char* my_key;

                  convert_to_array(val);

                  xReturn = XMLRPC_CreateVector(key, determine_vector_type(val->value.ht));

                  zend_hash_internal_pointer_reset(val->value.ht);
                  while(1) {
                     int res = my_zend_hash_get_current_key(val->value.ht, &my_key, &num_index);
                     if(res == HASH_KEY_IS_LONG) {
                        if(zend_hash_get_current_data(val->value.ht, (void**)&pIter) == SUCCESS) {
                           XMLRPC_AddValueToVector(xReturn, PHP_to_XMLRPC_worker(0, *pIter, depth++));
                        }
                     }
                     else if(res == HASH_KEY_NON_EXISTANT) {
                        break;
                     }
                     else if(res == HASH_KEY_IS_STRING) {
                        if(zend_hash_get_current_data(val->value.ht, (void**)&pIter) == SUCCESS) {
                           XMLRPC_AddValueToVector(xReturn, PHP_to_XMLRPC_worker(my_key, *pIter, depth++));
                        }
                     }

                     zend_hash_move_forward(val->value.ht);
                  }
               }
               break;
            default:
               break;
         }
      }
   }
   return xReturn;
}

static XMLRPC_VALUE PHP_to_XMLRPC(pval* root_val) {
   return PHP_to_XMLRPC_worker(NULL, root_val, 0);
}

/* recursively convert xmlrpc values into php values */
static pval* XMLRPC_to_PHP(XMLRPC_VALUE el) {
   pval* elem = NULL;
   char* pBuf;
   const char* pStr;

   if(el) {
      XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(el);

      MAKE_STD_ZVAL(elem); /* init. very important.  spent a frustrating day finding this out. */

      switch(type) {
         case xmlrpc_empty:
            elem->type = IS_NULL;
            break;
         case xmlrpc_string:
            pStr = XMLRPC_GetValueString(el);
            if(pStr) {
               elem->value.str.len = XMLRPC_GetValueStringLen(el);
               elem->value.str.val = estrndup(pStr, elem->value.str.len);
               elem->type = IS_STRING;
            }
            break;
         case xmlrpc_int:
            elem->value.lval = XMLRPC_GetValueInt(el);
            elem->type = IS_LONG;
            break;
         case xmlrpc_boolean:
            elem->value.lval = XMLRPC_GetValueBoolean(el);
            elem->type = IS_BOOL;
            break;
         case xmlrpc_double:
            elem->value.dval = XMLRPC_GetValueDouble(el);
            elem->type = IS_DOUBLE;
            break;
         case xmlrpc_datetime:
            elem->value.str.len = XMLRPC_GetValueStringLen(el);
            elem->value.str.val = estrndup(XMLRPC_GetValueDateTime_ISO8601(el), elem->value.str.len);
            elem->type = IS_STRING;
            break;
         case xmlrpc_base64:
            pStr = XMLRPC_GetValueBase64(el);
            if(pStr) {
               elem->value.str.len = XMLRPC_GetValueStringLen(el);
               elem->value.str.val = estrndup(pStr, elem->value.str.len);
               elem->type = IS_STRING;
            }
            break;
         case xmlrpc_vector:
            if(array_init(elem) == SUCCESS) {
               XMLRPC_VALUE xIter = XMLRPC_VectorRewind(el);

               while( xIter ) {
                  pval* val = XMLRPC_to_PHP(xIter);
                  if(val) {
                     add_pval(elem, XMLRPC_GetValueID(xIter), &val);
                  }
                  xIter = XMLRPC_VectorNext(el);
               }
            }
            break;
         default:
            break;
      }
      set_pval_xmlrpc_type(elem, type);
   }
   return elem;
}

/* {{{ proto string xmlrpc_encode_request(string method, mixed params)
   generate xml for a method request */
PHP_FUNCTION(xmlrpc_encode_request) {
   XMLRPC_REQUEST xRequest = NULL;
   pval* method, *vals, *out_opts;
   char* outBuf;
   php_output_options out;

   if( !(ARG_COUNT(ht) == 2 || ARG_COUNT(ht) == 3) || 
       getParameters(ht, ARG_COUNT(ht), &method, &vals, &out_opts) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   set_output_options(&out, (ARG_COUNT(ht) == 3) ? out_opts : 0);

   if(return_value_used) {
      xRequest = XMLRPC_RequestNew();

      if(xRequest) {
         XMLRPC_RequestSetOutputOptions(xRequest, &out.xmlrpc_out);
         if(method->type == IS_NULL) {
            XMLRPC_RequestSetRequestType(xRequest, xmlrpc_request_response);
         }
         else {
            XMLRPC_RequestSetMethodName(xRequest, method->value.str.val);
            XMLRPC_RequestSetRequestType(xRequest, xmlrpc_request_call);
         }
         if(vals->type != IS_NULL) {
             XMLRPC_RequestSetData(xRequest, PHP_to_XMLRPC(vals));
         }

         outBuf = XMLRPC_REQUEST_ToXML(xRequest, 0);
         if(outBuf) {
            RETVAL_STRING(outBuf, 1);
            free(outBuf);
         }
         XMLRPC_RequestFree(xRequest, 1);
      }
   }
}

/* {{{ proto string xmlrpc_encode(mixed value)
   generate xml for a PHP value */
PHP_FUNCTION(xmlrpc_encode)
{
   XMLRPC_VALUE xOut = NULL;
   pval* arg1, *out_opts;
   php_output_options out;
   char* outBuf;

   if( !(ARG_COUNT(ht) == 1)  || 
       getParameters(ht, ARG_COUNT(ht), &arg1) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   if( return_value_used ) {
      /* convert native php type to xmlrpc type */
      xOut = PHP_to_XMLRPC(arg1);

      /* generate raw xml from xmlrpc data */
      outBuf = XMLRPC_VALUE_ToXML(xOut, 0);

      if(xOut) {
         if(outBuf) {
            RETVAL_STRING(outBuf, 1);
            free(outBuf);
         }
         /* cleanup */
         XMLRPC_CleanupValue(xOut);
      }
   }
}


/* }}} */

pval* decode_request_worker(pval* xml_in, pval* encoding_in, pval* method_name_out) {
   pval* retval = NULL;
   XMLRPC_REQUEST response;
   STRUCT_XMLRPC_REQUEST_INPUT_OPTIONS opts = {0};
   opts.xml_elem_opts.encoding = encoding_in ? utf8_get_encoding_id_from_string(encoding_in->value.str.val) : ENCODING_DEFAULT;

   /* generate XMLRPC_REQUEST from raw xml */
   response = XMLRPC_REQUEST_FromXML(xml_in->value.str.val, xml_in->value.str.len, &opts);
   if(response) {
      /* convert xmlrpc data to native php types */
      retval = XMLRPC_to_PHP(XMLRPC_RequestGetData(response));

      if(XMLRPC_RequestGetRequestType(response) == xmlrpc_request_call) {
         if(method_name_out) {
            convert_to_string(method_name_out);
            method_name_out->type = IS_STRING;
            method_name_out->value.str.val = estrdup(XMLRPC_RequestGetMethodName(response));
            method_name_out->value.str.len = strlen(method_name_out->value.str.val);
         }
      }

      /* dust, sweep, and mop */
      XMLRPC_RequestFree(response, 1);
   }
   return retval;
}

/* {{{ proto array xmlrpc_decode_request(string xml, string& method, [string encoding])
   decode xml into native php types */
PHP_FUNCTION(xmlrpc_decode_request)
{
   pval* xml, *method, *encoding = NULL;

   if( !(ARG_COUNT(ht) == 2 || ARG_COUNT(ht) == 3) || getParameters(ht, ARG_COUNT(ht), &xml, &method, &encoding) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }
#if ZEND_MODULE_API_NO < 20010901
   if (!ParameterPassedByReference(ht,2)) {
       zend_error(E_WARNING,"second argument to xmlrpc_decode_request() passed by value, expecting reference");
   }
#endif

   convert_to_string(xml);
   convert_to_string(method);
   if(ARG_COUNT(ht) == 3) {
      convert_to_string(encoding);
   }

   if(return_value_used) {
      pval* retval = decode_request_worker(xml, encoding, method);
      if(retval) {
         *return_value = *retval;
         zval_copy_ctor(return_value);
      }
   }
}
/* }}} */


/* {{{ proto array xmlrpc_decode(string xml, [string encoding])
   decode xml into native php types */
PHP_FUNCTION(xmlrpc_decode)
{
   pval* arg1, *arg2 = NULL;

   if( !(ARG_COUNT(ht) == 1 || ARG_COUNT(ht) == 2) || getParameters(ht, ARG_COUNT(ht), &arg1, &arg2) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   convert_to_string(arg1);
   if(ARG_COUNT(ht) == 2) {
      convert_to_string(arg2);
   }

   if(return_value_used) {
      pval* retval = decode_request_worker(arg1, arg2, NULL);
      if(retval) {
         *return_value = *retval;
         zval_copy_ctor(return_value);
      }
   }
}
/* }}} */


/*************************
* server related methods *
*************************/

/* {{{ proto handle xmlrpc_server_create()
   create an xmlrpc server */
PHP_FUNCTION(xmlrpc_server_create) {
   if(ARG_COUNT(ht) != 0) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   if(return_value_used) {
      pval *method_map, *introspection_map;
      MAKE_STD_ZVAL(method_map);
      MAKE_STD_ZVAL(introspection_map);

      if(array_init(method_map) == SUCCESS && array_init(introspection_map) == SUCCESS) {
         /* allocate server data.  free'd in destroy_server_data() */
         xmlrpc_server_data *server = emalloc(sizeof(xmlrpc_server_data));

         if(server) {
            server->method_map = method_map;
            server->introspection_map = introspection_map;
            server->server_ptr = XMLRPC_ServerCreate();

            XMLRPC_ServerRegisterIntrospectionCallback(server->server_ptr, php_xmlrpc_introspection_callback);

            /* store for later use */
            ZEND_REGISTER_RESOURCE(return_value,server, XMLRPCG(le_xmlrpc_server));
         }
      }
   }
}

/* {{{ proto void xmlrpc_server_destroy(handle server)
   destroy server resources */
PHP_FUNCTION(xmlrpc_server_destroy) {
   pval* arg1;
   int bSuccess = FAILURE;

   if(ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   if(arg1->type == IS_RESOURCE) {
      int type;

      xmlrpc_server_data *server = zend_list_find(arg1->value.lval, &type);

      if(server && type == XMLRPCG(le_xmlrpc_server)) {
         bSuccess = zend_list_delete(arg1->value.lval);

         /* called by hashtable destructor
          * destroy_server_data(server);
          */
      }
   }
   RETVAL_LONG(bSuccess == SUCCESS);
}

           
/* called by xmlrpc C engine as method handler for all registered methods.
 * it then calls the corresponding PHP function to handle the method.
 */
static XMLRPC_VALUE php_xmlrpc_callback(XMLRPC_SERVER server, XMLRPC_REQUEST xRequest, void* data) {
   pval *retval_ptr;
   xmlrpc_callback_data* pData = (xmlrpc_callback_data*)data;
   pval* xmlrpc_params;
   pval* callback_params[3];

   /* convert xmlrpc to native php types */
   xmlrpc_params = XMLRPC_to_PHP(XMLRPC_RequestGetData(xRequest));

   /* setup data hoojum */
   callback_params[0] = pData->xmlrpc_method;
   callback_params[1] = xmlrpc_params;
   callback_params[2] = pData->caller_params;

   /* Use same C function for all methods */

   /* php func prototype: function user_func($method_name, $xmlrpc_params, $user_params) */
   call_user_function(CG(function_table), NULL, pData->php_function, pData->return_data, 3, callback_params);

   pData->php_executed = 1;
}

/* called by the C server when it first receives an introspection request.  We pass this on to
 * our PHP listeners, if any
 */
static void php_xmlrpc_introspection_callback(XMLRPC_SERVER server, void* data) {
   pval *retval_ptr, **php_function;
   pval* callback_params[1];
   xmlrpc_callback_data* pData = (xmlrpc_callback_data*)data;

   MAKE_STD_ZVAL(retval_ptr);
   retval_ptr->type = IS_NULL;

   /* setup data hoojum */
   callback_params[0] = pData->caller_params;

   /* loop through and call all registered callbacks */
   zend_hash_internal_pointer_reset(pData->server->introspection_map->value.ht);
   while(1) {
      if(zend_hash_get_current_data(pData->server->introspection_map->value.ht, 
                                    (void**)&php_function) == SUCCESS) {

         /* php func prototype: function string user_func($user_params) */
         if(call_user_function(CG(function_table), NULL, *php_function, 
                               retval_ptr, 1, callback_params) == SUCCESS) {
            XMLRPC_VALUE xData;
            STRUCT_XMLRPC_ERROR err = {0};

            /* return value should be a string */
            convert_to_string(retval_ptr);

            xData = XMLRPC_IntrospectionCreateDescription(retval_ptr->value.str.val, &err);

            if(xData) {
               if(!XMLRPC_ServerAddIntrospectionData(server, xData)) {
                  zend_error(E_WARNING, "Unable to add introspection data returned from %s(), improper element structure", (*php_function)->value.str.val);
               }
               XMLRPC_CleanupValue(xData);
            }
            else {
               /* could not create description */
               if(err.xml_elem_error.parser_code) {
                  zend_error(E_WARNING, "xml parse error: [line %i, column %i, message: %s] Unable to add introspection data returned from %s()", 
                             err.xml_elem_error.column, err.xml_elem_error.line, err.xml_elem_error.parser_error, (*php_function)->value.str.val);
               }
               else {
                  zend_error(E_WARNING, "Unable to add introspection data returned from %s()", 
                             (*php_function)->value.str.val);
               }
            }
         }
         else {
            /* user func failed */
            zend_error(E_WARNING, "Error calling user introspection callback: %s()", (*php_function)->value.str.val);
         }
      }
      else {
         break;
      }

      zend_hash_move_forward(pData->server->introspection_map->value.ht);
   }

   /* so we don't call the same callbacks ever again */
   zend_hash_clean(pData->server->introspection_map->value.ht);
}

/* {{{ proto boolean xmlrpc_server_register_method(handle server, string method_name, string function)
   register a php function to handle method matching method_name */
PHP_FUNCTION(xmlrpc_server_register_method) {

   pval* method_key, *method_name, *handle, *method_name_save;
   int type;
   xmlrpc_server_data* server;

   /* get some params.  should be 3 */
   if( !(ARG_COUNT(ht) == 3) || getParameters(ht, ARG_COUNT(ht), &handle, &method_key, &method_name) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   server = zend_list_find(handle->value.lval, &type);

   if(type == XMLRPCG(le_xmlrpc_server)) {
      /* register with C engine. every method just calls our standard callback, 
       * and it then dispatches to php as necessary
       */
      if(XMLRPC_ServerRegisterMethod(server->server_ptr, method_key->value.str.val, php_xmlrpc_callback)) {
         /* save for later use */
         MAKE_STD_ZVAL(method_name_save);
         *method_name_save = *method_name;
         zval_copy_ctor(method_name_save);

         /* register our php method */
         add_pval(server->method_map, method_key->value.str.val, &method_name_save);

         RETURN_BOOL(1);
      }
   }
   RETURN_BOOL(0);
}


/* {{{ proto boolean xmlrpc_server_register_introspection_callback(handle server, string function)
   register a php function to generate documentation */
PHP_FUNCTION(xmlrpc_server_register_introspection_callback) {

   pval* method_key, *method_name, *handle, *method_name_save;
   int type;
   xmlrpc_server_data* server;

   /* get some params.  should be 2 */
   if( !(ARG_COUNT(ht) == 2) || getParameters(ht, ARG_COUNT(ht), &handle, &method_name) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   server = zend_list_find(handle->value.lval, &type);

   if(type == XMLRPCG(le_xmlrpc_server)) {
      {
         /* save for later use */
         MAKE_STD_ZVAL(method_name_save);
         *method_name_save = *method_name;
         zval_copy_ctor(method_name_save);

         /* register our php method */
         add_pval(server->introspection_map, NULL, &method_name_save);

         RETURN_BOOL(1);
      }
   }
   RETURN_BOOL(0);
}


/* this function is itchin for a re-write */

/* {{{ proto mixed xmlrpc_server_call_method(handle server, string xml, mixed user_data, [array output_options])
   parse xml request and call method */
PHP_FUNCTION(xmlrpc_server_call_method) {
   xmlrpc_callback_data data = {0};
   XMLRPC_REQUEST xRequest;
   STRUCT_XMLRPC_REQUEST_INPUT_OPTIONS input_opts;
   xmlrpc_server_data* server;
   pval *rawxml, *caller_params, *handle, *output_opts;
   int type;
   php_output_options out;

   /* get params. 3 or 4 params ok */
   if(ARG_COUNT(ht) == 4) {
      if(getParameters(ht, ARG_COUNT(ht), &handle, &rawxml, &caller_params, &output_opts) != SUCCESS) {
         WRONG_PARAM_COUNT;
      }

      /* user output options */
      set_output_options(&out, output_opts);
   }
   else if(ARG_COUNT(ht) == 3) {
      if(getParameters(ht, ARG_COUNT(ht), &handle, &rawxml, &caller_params) != SUCCESS) {
         WRONG_PARAM_COUNT;
      }
      /* user output options */
      set_output_options(&out, NULL);
   }
   else {
      WRONG_PARAM_COUNT;
   }

   server = zend_list_find(handle->value.lval, &type);

   if(type == XMLRPCG(le_xmlrpc_server)) {
      /* HACK: use output encoding for now */
      input_opts.xml_elem_opts.encoding = utf8_get_encoding_id_from_string(out.xmlrpc_out.xml_elem_opts.encoding);

      /* generate an XMLRPC_REQUEST from the raw xml input */
      xRequest = XMLRPC_REQUEST_FromXML(rawxml->value.str.val, rawxml->value.str.len, &input_opts);

      if(xRequest) {

         /* check if we have a method name -- indicating success and all manner of good things */
         if(XMLRPC_RequestGetMethodName(xRequest)) {
            pval** php_function, *returned = NULL;
            XMLRPC_VALUE xAnswer = NULL;
            MAKE_STD_ZVAL(data.xmlrpc_method); /* init. very important.  spent a frustrating day finding this out. */
            MAKE_STD_ZVAL(data.return_data);
            data.return_data->type = IS_NULL;  /* in case value is never init'd, we don't dtor to think it is a string or something */
            data.xmlrpc_method->type = IS_NULL;

            /* setup some data to pass to the callback function */
            data.xmlrpc_method->value.str.val = estrdup(XMLRPC_RequestGetMethodName(xRequest));
            data.xmlrpc_method->value.str.len = strlen(data.xmlrpc_method->value.str.val);
            data.xmlrpc_method->type = IS_STRING;
            data.caller_params = caller_params;
            data.php_executed = 0;
            data.server = server;

            /* check if the called method has been previous registered */
            if(zend_hash_find(server->method_map->value.ht, 
                              data.xmlrpc_method->value.str.val, 
                              data.xmlrpc_method->value.str.len + 1, 
                              (void**)&php_function) == SUCCESS) {

               data.php_function = *php_function;
            }

           /* We could just call the php method directly ourselves at this point, but we do this 
            * with a C callback in case the xmlrpc library ever implements some cool usage stats,
            * or somesuch.
            */
           xAnswer = XMLRPC_ServerCallMethod(server->server_ptr, xRequest, &data);
           if(xAnswer) {
               if(out.b_php_out) {
                  zval_dtor(data.return_data);
                  FREE_ZVAL(data.return_data);
                  data.return_data = XMLRPC_to_PHP(xAnswer);
               }
           }
           else if(data.php_executed) {
               if(!out.b_php_out) {
                   xAnswer = PHP_to_XMLRPC(data.return_data);
               }
           }

           /* should we return data as xml? */
           if(!out.b_php_out) {
              XMLRPC_REQUEST xResponse = XMLRPC_RequestNew();
              if(xResponse) {
                 char* outBuf = 0;
                 int buf_len = 0;

                 /* set some required request hoojum */
                 XMLRPC_RequestSetOutputOptions(xResponse, &out.xmlrpc_out);
                 XMLRPC_RequestSetRequestType(xResponse, xmlrpc_request_response);
                 XMLRPC_RequestSetData(xResponse, xAnswer);

                 /* generate xml */
                 outBuf = XMLRPC_REQUEST_ToXML(xResponse, &buf_len);
                 if(outBuf) {
                    RETVAL_STRINGL(outBuf, buf_len, 1);
                    free(outBuf);
                 }
                 /* cleanup after ourselves.  what a sty! */
                 XMLRPC_RequestFree(xResponse, 0);
              }
           }
           /* or as native php types? */
           else {
              *return_value = *data.return_data;
              zval_copy_ctor(return_value);
           }

            /* cleanup after ourselves.  what a sty! */
            zval_dtor(data.xmlrpc_method);
            FREE_ZVAL(data.xmlrpc_method);
            zval_dtor(data.return_data);
            FREE_ZVAL(data.return_data);

            if(xAnswer) {
               XMLRPC_CleanupValue(xAnswer);
            }
         }

         XMLRPC_RequestFree(xRequest, 1);
      }
   }
}


/* {{{ proto int xmlrpc_server_add_introspection_data(handle server, array desc)
   add introspection documentation  */
PHP_FUNCTION(xmlrpc_server_add_introspection_data) {

   pval *method, *handle, *desc;
   int type;
   xmlrpc_server_data* server;

   /* get some params.  should be 2 */
   if ( !(ARG_COUNT(ht) == 2) || getParameters(ht, ARG_COUNT(ht), &handle, &desc) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   server = zend_list_find(handle->value.lval, &type);

   if (type == XMLRPCG(le_xmlrpc_server)) {
      XMLRPC_VALUE xDesc = PHP_to_XMLRPC(desc);
      if (xDesc) {
         int retval = XMLRPC_ServerAddIntrospectionData(server->server_ptr, xDesc);
         XMLRPC_CleanupValue(xDesc);
         RETURN_LONG(retval);
      }
   }
   RETURN_LONG(0);
}


/* {{{ proto array xmlrpc_parse_method_descriptions(string xml)
   decode xml into a list of method descriptions */
PHP_FUNCTION(xmlrpc_parse_method_descriptions)
{
   pval* arg1, *arg2, *retval;

   if( !(ARG_COUNT(ht) == 1) || getParameters(ht, ARG_COUNT(ht), &arg1) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   convert_to_string(arg1);

   if(return_value_used) {
      STRUCT_XMLRPC_ERROR err = {0};
      XMLRPC_VALUE xVal = XMLRPC_IntrospectionCreateDescription(arg1->value.str.val, &err);
      if(xVal) {
         retval = XMLRPC_to_PHP(xVal);

         if(retval) {
            *return_value = *retval;
            zval_copy_ctor(return_value);
         }
         /* dust, sweep, and mop */
         XMLRPC_CleanupValue(xVal);
      }
      else {
         /* could not create description */
         if(err.xml_elem_error.parser_code) {
            zend_error(E_WARNING, "xml parse error: [line %i, column %i, message: %s] Unable to create introspection data", 
                       err.xml_elem_error.column, err.xml_elem_error.line, err.xml_elem_error.parser_error);
         }
         else {
            zend_error(E_WARNING, "Invalid xml structure. Unable to create introspection data");
         }

         zend_error(E_WARNING, "xml parse error.  no method description created");
      }
   }
}


/************
* type data *
************/

#define XMLRPC_TYPE_COUNT 9
#define XMLRPC_VECTOR_TYPE_COUNT 4
#define TYPE_STR_MAP_SIZE (XMLRPC_TYPE_COUNT + XMLRPC_VECTOR_TYPE_COUNT)

/* return a string matching a given xmlrpc type */
static const char** get_type_str_mapping() {
   static const char* str_mapping[TYPE_STR_MAP_SIZE];
   static int first = 1;
   if(first) {
      /* warning. do not add/delete without changing size define */
      str_mapping[xmlrpc_none] = "none";
      str_mapping[xmlrpc_empty] = "empty";
      str_mapping[xmlrpc_base64] = "base64";
      str_mapping[xmlrpc_boolean] = "boolean";
      str_mapping[xmlrpc_datetime] = "datetime";
      str_mapping[xmlrpc_double] = "double";
      str_mapping[xmlrpc_int] = "int";
      str_mapping[xmlrpc_string] = "string";
      str_mapping[xmlrpc_vector] = "vector";
      str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_none] = "none";
      str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_array] = "array";
      str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_mixed] = "mixed";
      str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_struct] = "struct";
      first = 0;
   }
   return (const char**)str_mapping;
}

/* map an xmlrpc type to a string */
const char* xmlrpc_type_as_str(XMLRPC_VALUE_TYPE type, XMLRPC_VECTOR_TYPE vtype) {
   const char** str_mapping = get_type_str_mapping();
   if(vtype == xmlrpc_vector_none) {
      return str_mapping[type];
   }
   else {
      return str_mapping[XMLRPC_TYPE_COUNT + vtype];
   }
}

/* map a string to an xmlrpc type */
XMLRPC_VALUE_TYPE xmlrpc_str_as_type(const char* str) {
   const char** str_mapping = get_type_str_mapping();
   int i;

   if(str) {
      for(i = 0; i < XMLRPC_TYPE_COUNT; i++) {
         if(!strcmp(str_mapping[i], str)) {
            return (XMLRPC_VALUE_TYPE)i;
         }
      }
   }
   return xmlrpc_none;
}

/* map a string to an xmlrpc vector type */
XMLRPC_VECTOR_TYPE xmlrpc_str_as_vector_type(const char* str) {
   const char** str_mapping = get_type_str_mapping();
   int i;

   if(str) {
      for(i = XMLRPC_TYPE_COUNT; i < TYPE_STR_MAP_SIZE; i++) {
         if(!strcmp(str_mapping[i], str)) {
            return (XMLRPC_VECTOR_TYPE)(i - XMLRPC_TYPE_COUNT);
         }
      }
   }
   return xmlrpc_none;
}


/* set a given value to a particular type. 
 * note: this only works on strings, and only for date and base64,
 *       which do not have native php types. black magic lies herein.
 */
int set_pval_xmlrpc_type(pval* value, XMLRPC_VALUE_TYPE type) {
   int bSuccess = FAILURE;

   /* we only really care about strings because they can represent
    * base64 and datetime.  all other types have corresponding php types
    */
   if(value->type == IS_STRING) {
      if(type == xmlrpc_base64 || type == xmlrpc_datetime) {
         const char* typestr = xmlrpc_type_as_str(type, xmlrpc_vector_none);
         pval* type;

         MAKE_STD_ZVAL(type);

         type->type = IS_STRING;
         type->value.str.val = estrdup(typestr);
         type->value.str.len = strlen(typestr);

         convert_to_object(value);
         bSuccess = zend_hash_update(value->value.obj.properties, OBJECT_TYPE_ATTR, sizeof(OBJECT_TYPE_ATTR), (void *) &type, sizeof(zval *), NULL);
      }
   }
   
   return bSuccess;
}

/* return xmlrpc type of a php value */
XMLRPC_VALUE_TYPE get_pval_xmlrpc_type(pval* value, pval** newvalue) {
   XMLRPC_VALUE_TYPE type = xmlrpc_none;

   if(value) {
      switch(value->type) {
         case IS_NULL:
            type = xmlrpc_base64;
            break;
   #ifndef BOOL_AS_LONG

   /* Right thing to do, but it breaks some legacy code. */
         case IS_BOOL:
            type = xmlrpc_boolean;
            break;
   #else
         case IS_BOOL:
   #endif
         case IS_LONG:
         case IS_RESOURCE:
            type = xmlrpc_int;
            break;
         case IS_DOUBLE:
            type = xmlrpc_double;
            break;
         case IS_CONSTANT:
            type = xmlrpc_string;
            break;
         case IS_STRING:
            type = xmlrpc_string;
            break;
         case IS_ARRAY:
         case IS_CONSTANT_ARRAY:
            type = xmlrpc_vector;
            break;
         case IS_OBJECT:
         {
            pval** attr;
            type = xmlrpc_vector;

            if(zend_hash_find(value->value.obj.properties, 
                              OBJECT_TYPE_ATTR, sizeof(OBJECT_TYPE_ATTR), 
                              (void**)&attr) == SUCCESS) {
               if((*attr)->type == IS_STRING) {
                  type = xmlrpc_str_as_type((*attr)->value.str.val);
               }
            }
            break;
         }
      }

      /* if requested, return an unmolested (magic removed) copy of the value */
      if(newvalue) {
         pval** val;
         if( (type == xmlrpc_base64 && value->type != IS_NULL) || type == xmlrpc_datetime) {
            if(zend_hash_find(value->value.obj.properties, 
                           OBJECT_VALUE_ATTR, sizeof(OBJECT_VALUE_ATTR), 
                           (void**)&val) == SUCCESS) {
               *newvalue = *val;
            }
         }
         else {
            *newvalue = value;
         }
      }
   }

   return type;
}


/* {{{ proto bool xmlrpc_set_type(string value, string type)
   set xmlrpc type, base64 or datetime, for a php string value */
PHP_FUNCTION(xmlrpc_set_type) {
   pval* arg, *type;
   XMLRPC_VALUE_TYPE vtype;

   if( !(ARG_COUNT(ht) == 2) || getParameters(ht, ARG_COUNT(ht), &arg, &type) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }
#if ZEND_MODULE_API_NO < 20010901
   if (!ParameterPassedByReference(ht,1)) {
       zend_error(E_WARNING,"first argument to xmlrpc_set_type() passed by value, expecting reference");
   }
#endif

   convert_to_string(type);
   vtype = xmlrpc_str_as_type(type->value.str.val);
   if(vtype != xmlrpc_none) {
      if(set_pval_xmlrpc_type(arg, vtype) == SUCCESS) {
         RETURN_TRUE;
      }
   }
   else {
      zend_error(E_WARNING,"invalid type '%s' passed to xmlrpc_set_type()", type->value.str.val);
   }
   RETURN_FALSE;
}

/* {{{ proto string xmlrpc_get_type(mixed value)
   get xmlrpc type for a php value. especially useful for base64 and datetime strings */
PHP_FUNCTION(xmlrpc_get_type) {
   pval* arg;
   XMLRPC_VALUE_TYPE type;
   XMLRPC_VECTOR_TYPE vtype = xmlrpc_vector_none;

   if( !(ARG_COUNT(ht) == 1) || getParameters(ht, ARG_COUNT(ht), &arg) == FAILURE) {
      WRONG_PARAM_COUNT; /* prints/logs a warning and returns */
   }

   type = get_pval_xmlrpc_type(arg, 0);
   if(type == xmlrpc_vector) {
      vtype = determine_vector_type(arg->value.ht);
   }
   
   RETURN_STRING((char*)xmlrpc_type_as_str(type, vtype), 1);
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
