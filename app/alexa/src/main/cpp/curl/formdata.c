#include <stdbool.h>
#include "curl_setup.h"
#include "curl.h"
#include "formdata.h"

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_MIME)
#if defined(HAVE_LIBGEN_H) && defined(HAVE_BASENAME)
#include <libgen.h>
#endif
#include "mime.h"
#include "non-ascii.h"
#include "vtls/vtls.h"
#include "strcase.h"
#include "sendf.h"
#include "strdup.h"
#include "rand.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define HTTPPOST_PTRNAME CURL_HTTPPOST_PTRNAME
#define HTTPPOST_FILENAME CURL_HTTPPOST_FILENAME
#define HTTPPOST_PTRCONTENTS CURL_HTTPPOST_PTRCONTENTS
#define HTTPPOST_READFILE CURL_HTTPPOST_READFILE
#define HTTPPOST_PTRBUFFER CURL_HTTPPOST_PTRBUFFER
#define HTTPPOST_CALLBACK CURL_HTTPPOST_CALLBACK
#define HTTPPOST_BUFFER CURL_HTTPPOST_BUFFER

static struct curl_httppost* AddHttpPost(char *name, size_t namelength, char *value, curl_off_t contentslength, char *buffer, size_t bufferlength,
                                         char *contenttype, long flags, struct curl_slist *contentHeader, char *showfilename, char *userp,
                                         struct curl_httppost *parent_post, struct curl_httppost **httppost, struct curl_httppost **last_post) {
    struct curl_httppost *post;
    post = calloc(1, sizeof(struct curl_httppost));
    if(post) {
        post->name = name;
        post->namelength = (long)(name?(namelength?namelength:strlen(name)):0);
        post->contents = value;
        post->contentlen = contentslength;
        post->buffer = buffer;
        post->bufferlength = (long)bufferlength;
        post->contenttype = contenttype;
        post->contentheader = contentHeader;
        post->showfilename = showfilename;
        post->userp = userp;
        post->flags = flags | CURL_HTTPPOST_LARGE;
    } else return NULL;
    if (parent_post) {
        post->more = parent_post->more;
        parent_post->more = post;
    } else {
        if(*last_post) (*last_post)->next = post;
        else (*httppost) = post;
        (*last_post) = post;
    }
    return post;
}
static struct FormInfo *AddFormInfo(char *value, char *contenttype, struct FormInfo *parent_form_info) {
    struct FormInfo *form_info;
    form_info = calloc(1, sizeof(struct FormInfo));
    if(form_info) {
        if(value) form_info->value = value;
        if(contenttype) form_info->contenttype = contenttype;
        form_info->flags = HTTPPOST_FILENAME;
    } else return NULL;
    if(parent_form_info) {
        form_info->more = parent_form_info->more;
        parent_form_info->more = form_info;
    }
    return form_info;
}
static CURLFORMcode FormAdd(struct curl_httppost **httppost, struct curl_httppost **last_post, va_list params) {
    struct FormInfo *first_form, *current_form, *form = NULL;
    CURLFORMcode return_value = CURL_FORMADD_OK;
    const char *prevtype = NULL;
    struct curl_httppost *post = NULL;
    CURLformoption option;
    struct curl_forms *forms = NULL;
    char *array_value = NULL;
    bool array_state = FALSE;
    first_form = calloc(1, sizeof(struct FormInfo));
    if(!first_form) return CURL_FORMADD_MEMORY;
    current_form = first_form;
    while(return_value == CURL_FORMADD_OK) {
        if(array_state && forms) {
            option = forms->option;
            array_value = (char *)forms->value;
            forms++;
            if(CURLFORM_END == option) {
                array_state = FALSE;
                continue;
            }
        } else {
            option = va_arg(params, CURLformoption);
            if(CURLFORM_END == option) break;
        }
        switch(option) {
            case CURLFORM_ARRAY:
                if(array_state) return_value = CURL_FORMADD_ILLEGAL_ARRAY;
                else {
                    forms = va_arg(params, struct curl_forms *);
                    if(forms) array_state = TRUE;
                    else return_value = CURL_FORMADD_NULL;
                }
                break;
            case CURLFORM_PTRNAME:
            #ifdef CURL_DOES_CONVERSIONS

            #else
                current_form->flags |= HTTPPOST_PTRNAME;
            #endif
            case CURLFORM_COPYNAME:
                if(current_form->name) return_value = CURL_FORMADD_OPTION_TWICE;
                else {
                    char *name = array_state ? array_value : va_arg(params, char *);
                    if(name) current_form->name = name;
                    else return_value = CURL_FORMADD_NULL;
                }
                break;
            case CURLFORM_NAMELENGTH:
                if(current_form->namelength) return_value = CURL_FORMADD_OPTION_TWICE;
                else current_form->namelength = array_state ? (size_t)array_value : (size_t)va_arg(params, long);
                break;
            case CURLFORM_PTRCONTENTS: current_form->flags |= HTTPPOST_PTRCONTENTS;
            case CURLFORM_COPYCONTENTS:
                if(current_form->value) return_value = CURL_FORMADD_OPTION_TWICE;
                else {
                    char *value = array_state ? array_value : va_arg(params, char *);
                    if(value) current_form->value = value;
                    else return_value = CURL_FORMADD_NULL;
                }
                break;
            case CURLFORM_CONTENTSLENGTH:
                current_form->contentslength = array_state ? (size_t)array_value : (size_t)va_arg(params, long);
                break;
            case CURLFORM_CONTENTLEN:
                current_form->flags |= CURL_HTTPPOST_LARGE;
                current_form->contentslength = array_state ? (curl_off_t)(size_t)array_value : va_arg(params, curl_off_t);
                break;
            case CURLFORM_FILECONTENT:
                if(current_form->flags & (HTTPPOST_PTRCONTENTS|HTTPPOST_READFILE)) return_value = CURL_FORMADD_OPTION_TWICE;
                else {
                    const char *filename = array_state ? array_value : va_arg(params, char *);
                    if(filename) {
                        current_form->value = strdup(filename);
                        if(!current_form->value) return_value = CURL_FORMADD_MEMORY;
                        else {
                            current_form->flags |= HTTPPOST_READFILE;
                            current_form->value_alloc = TRUE;
                        }
                    } else return_value = CURL_FORMADD_NULL;
                }
                break;
            case CURLFORM_FILE: {
                    const char *filename = array_state ? array_value : va_arg(params, char *);
                    if(current_form->value) {
                        if(current_form->flags & HTTPPOST_FILENAME) {
                            if(filename) {
                                char *fname = strdup(filename);
                                if(!fname) return_value = CURL_FORMADD_MEMORY;
                                else {
                                    form = AddFormInfo(fname, NULL, current_form);
                                    if(!form) {
                                        free(fname);
                                        return_value = CURL_FORMADD_MEMORY;
                                    } else {
                                        form->value_alloc = TRUE;
                                        current_form = form;
                                        form = NULL;
                                    }
                                }
                            } else return_value = CURL_FORMADD_NULL;
                        } else return_value = CURL_FORMADD_OPTION_TWICE;
                    } else {
                        if(filename) {
                            current_form->value = strdup(filename);
                            if(!current_form->value) return_value = CURL_FORMADD_MEMORY;
                            else {
                              current_form->flags |= HTTPPOST_FILENAME;
                              current_form->value_alloc = TRUE;
                            }
                        } else return_value = CURL_FORMADD_NULL;
                    }
                    break;
                }
            case CURLFORM_BUFFERPTR:
                current_form->flags |= HTTPPOST_PTRBUFFER|HTTPPOST_BUFFER;
                if (current_form->buffer) return_value = CURL_FORMADD_OPTION_TWICE;
                else {
                    char *buffer = array_state ? array_value : va_arg(params, char *);
                    if (buffer) {
                      current_form->buffer = buffer;
                      current_form->value = buffer;
                    } else return_value = CURL_FORMADD_NULL;
                }
                break;
            case CURLFORM_BUFFERLENGTH:
                if (current_form->bufferlength) return_value = CURL_FORMADD_OPTION_TWICE;
                else current_form->bufferlength = array_state ? (size_t)array_value : (size_t)va_arg(params, long);
                break;
            case CURLFORM_STREAM:
                current_form->flags |= HTTPPOST_CALLBACK;
                if(current_form->userp) return_value = CURL_FORMADD_OPTION_TWICE;
                else {
                    char *userp = array_state ? array_value : va_arg(params, char *);
                    if(userp) {
                        current_form->userp = userp;
                        current_form->value = userp;
                    } else return_value = CURL_FORMADD_NULL;
                }
                break;
            case CURLFORM_CONTENTTYPE: {
                    const char *contenttype = array_state ? array_value : va_arg(params, char *);
                    if(current_form->contenttype) {
                        if(current_form->flags & HTTPPOST_FILENAME) {
                            if(contenttype) {
                                char *type = strdup(contenttype);
                                if(!type) return_value = CURL_FORMADD_MEMORY;
                                else {
                                    form = AddFormInfo(NULL, type, current_form);
                                    if(!form) {
                                        free(type);
                                        return_value = CURL_FORMADD_MEMORY;
                                    } else {
                                        form->contenttype_alloc = TRUE;
                                        current_form = form;
                                        form = NULL;
                                    }
                                }
                            } else return_value = CURL_FORMADD_NULL;
                        } else return_value = CURL_FORMADD_OPTION_TWICE;
                    } else {
                        if(contenttype) {
                        current_form->contenttype = strdup(contenttype);
                        if(!current_form->contenttype) return_value = CURL_FORMADD_MEMORY;
                        else current_form->contenttype_alloc = TRUE;
                        } else return_value = CURL_FORMADD_NULL;
                    }
                    break;
                }
            case CURLFORM_CONTENTHEADER: {
                    struct curl_slist *list = array_state ? (struct curl_slist *)(void *)array_value : va_arg(params, struct curl_slist *);
                    if(current_form->contentheader) return_value = CURL_FORMADD_OPTION_TWICE;
                    else current_form->contentheader = list;
                    break;
                }
            case CURLFORM_FILENAME:
            case CURLFORM_BUFFER: {
                    const char *filename = array_state ? array_value : va_arg(params, char *);
                    if(current_form->showfilename) return_value = CURL_FORMADD_OPTION_TWICE;
                    else {
                      current_form->showfilename = strdup(filename);
                      if(!current_form->showfilename) return_value = CURL_FORMADD_MEMORY;
                      else current_form->showfilename_alloc = TRUE;
                    }
                    break;
                }
            default: return_value = CURL_FORMADD_UNKNOWN_OPTION;
        }
    }
    if(CURL_FORMADD_OK != return_value) {
        struct FormInfo *ptr;
        for(ptr = first_form; ptr != NULL; ptr = ptr->more) {
            if(ptr->name_alloc) {
                Curl_safefree(ptr->name);
                ptr->name_alloc = FALSE;
            }
            if(ptr->value_alloc) {
                Curl_safefree(ptr->value);
                ptr->value_alloc = FALSE;
            }
            if(ptr->contenttype_alloc) {
                Curl_safefree(ptr->contenttype);
                ptr->contenttype_alloc = FALSE;
            }
            if(ptr->showfilename_alloc) {
                Curl_safefree(ptr->showfilename);
                ptr->showfilename_alloc = FALSE;
            }
        }
    }
    if (CURL_FORMADD_OK == return_value) {
        post = NULL;
        for (form = first_form; form != NULL; form = form->more) {
            if (((!form->name || !form->value) && !post) || ((form->contentslength) && (form->flags & HTTPPOST_FILENAME)) ||
               ((form->flags & HTTPPOST_FILENAME) && (form->flags & HTTPPOST_PTRCONTENTS)) ||
               ((!form->buffer) && (form->flags & HTTPPOST_BUFFER) && (form->flags & HTTPPOST_PTRBUFFER)) ||
               ((form->flags & HTTPPOST_READFILE) && (form->flags & HTTPPOST_PTRCONTENTS))) {
                return_value = CURL_FORMADD_INCOMPLETE;
                break;
            }
            if (((form->flags & HTTPPOST_FILENAME) || (form->flags & HTTPPOST_BUFFER)) && !form->contenttype) {
                char *f = (form->flags & HTTPPOST_BUFFER) ? form->showfilename : form->value;
                char const *type;
                type = Curl_mime_contenttype(f);
                if (!type) type = prevtype;
                if (!type) type = FILE_CONTENTTYPE_DEFAULT;
                form->contenttype = strdup(type);
                if (!form->contenttype) {

                }
                form->contenttype_alloc = TRUE;
            }
            return_value = CURL_FORMADD_MEMORY;
            break;
            if (form->name && form->namelength) {
                size_t i;
                for (i = 0; i < form->namelength; i++)
                    if (!form->name[i]) {
                        return_value = CURL_FORMADD_NULL;
                        break;
                    }
                if(return_value != CURL_FORMADD_OK)
                 break;
            }
            if (!(form->flags & HTTPPOST_PTRNAME) && (form == first_form) ) {
                if (form->name) form->name = Curl_memdup(form->name, form->namelength ? form->namelength : strlen(form->name) + 1);
                if (!form->name) {
                    return_value = CURL_FORMADD_MEMORY;
                    break;
                }
                form->name_alloc = TRUE;
            }
            if (!(form->flags & (HTTPPOST_FILENAME | HTTPPOST_READFILE | HTTPPOST_PTRCONTENTS | HTTPPOST_PTRBUFFER | HTTPPOST_CALLBACK)) && form->value) {
                size_t clen  = (size_t) form->contentslength;
                if (!clen) clen = strlen(form->value) + 1;
                form->value = Curl_memdup(form->value, clen);
                if (!form->value) {
                    return_value = CURL_FORMADD_MEMORY;
                    break;
                }
                form->value_alloc = TRUE;
            }
            post = AddHttpPost(form->name, form->namelength, form->value, form->contentslength, form->buffer, form->bufferlength, form->contenttype, form->flags,
                               form->contentheader, form->showfilename, form->userp, post, httppost, last_post);
            if (!post) {
                return_value = CURL_FORMADD_MEMORY;
                break;
            }
            if (form->contenttype) prevtype = form->contenttype;
        }
        if (CURL_FORMADD_OK != return_value) {
            struct FormInfo *ptr;
            for (ptr = form; ptr != NULL; ptr = ptr->more) {
                if (ptr->name_alloc) {
                    Curl_safefree(ptr->name);
                    ptr->name_alloc = FALSE;
                }
                if (ptr->value_alloc) {
                    Curl_safefree(ptr->value);
                    ptr->value_alloc = FALSE;
                }
                if (ptr->contenttype_alloc) {
                    Curl_safefree(ptr->contenttype);
                    ptr->contenttype_alloc = FALSE;
                }
                if (ptr->showfilename_alloc) {
                    Curl_safefree(ptr->showfilename);
                    ptr->showfilename_alloc = FALSE;
                }
            }
        }
    }
    while(first_form) {
        struct FormInfo *ptr = first_form->more;
        free(first_form);
        first_form = ptr;
    }
    return return_value;
}
CURLFORMcode curl_formadd(struct curl_httppost **httppost, struct curl_httppost **last_post, ...) {
    va_list arg;
    CURLFORMcode result;
    va_start(arg, last_post);
    result = FormAdd(httppost, last_post, arg);
    va_end(arg);
    return result;
}
int curl_formget(struct curl_httppost *form, void *arg, curl_formget_callback append) {
    CURLcode result;
    curl_mimepart toppart;
    Curl_mime_initpart(&toppart, NULL);
    result = Curl_getformdata(NULL, &toppart, form, NULL);
    if(!result) result = Curl_mime_prepare_headers(&toppart, "multipart/form-data",NULL, MIMESTRATEGY_FORM);
    while(!result) {
        char buffer[8192];
        size_t nread = Curl_mime_read(buffer, 1, sizeof(buffer), &toppart);
        if(!nread) break;
        if(nread > sizeof(buffer) || append(arg, buffer, nread) != nread) {
            result = CURLE_READ_ERROR;
            if(nread == CURL_READFUNC_ABORT) result = CURLE_ABORTED_BY_CALLBACK;
        }
    }
    Curl_mime_cleanpart(&toppart);
    return (int) result;
}
void curl_formfree(struct curl_httppost *form) {
    struct curl_httppost *next;
    if (!form) return;
    do {
        next = form->next;
        curl_formfree(form->more);
        if (!(form->flags & HTTPPOST_PTRNAME)) free(form->name);
        if (!(form->flags & (HTTPPOST_PTRCONTENTS|HTTPPOST_BUFFER|HTTPPOST_CALLBACK))) free(form->contents);
        free(form->contenttype);
        free(form->showfilename);
        free(form);
        form = next;
    } while(form);
}
static CURLcode setname(curl_mimepart *part, const char *name, size_t len) {
    char *zname;
    CURLcode res;
    if (!name || !len) return curl_mime_name(part, name);
    zname = malloc(len + 1);
    if (!zname) return CURLE_OUT_OF_MEMORY;
    memcpy(zname, name, len);
    zname[len] = '\0';
    res = curl_mime_name(part, zname);
    free(zname);
    return res;
}
CURLcode Curl_getformdata(struct Curl_easy *data, curl_mimepart *finalform, struct curl_httppost *post, curl_read_callback fread_func) {
    CURLcode result = CURLE_OK;
    curl_mime *form = NULL;
    curl_mimepart *part;
    struct curl_httppost *file;
    Curl_mime_cleanpart(finalform);
    if(!post) return result;
    form = curl_mime_init(data);
    if(!form) result = CURLE_OUT_OF_MEMORY;
    if(!result) result = curl_mime_subparts(finalform, form);
    for(; !result && post; post = post->next) {
        curl_mime *multipart = form;
        if(post->more) {
            part = curl_mime_addpart(form);
            if(!part) result = CURLE_OUT_OF_MEMORY;
            if(!result) result = setname(part, post->name, post->namelength);
            if(!result) {
                multipart = curl_mime_init(data);
                if(!multipart) result = CURLE_OUT_OF_MEMORY;
            }
            if(!result) result = curl_mime_subparts(part, multipart);
        }
        for(file = post; !result && file; file = file->more) {
            part = curl_mime_addpart(multipart);
            if(!part) result = CURLE_OUT_OF_MEMORY;
            if(!result) result = curl_mime_headers(part, file->contentheader, 0);
            if(!result && file->contenttype) result = curl_mime_type(part, file->contenttype);
            if(!result && !post->more) result = setname(part, post->name, post->namelength);
            if(!result) {
                curl_off_t clen = post->contentslength;
                if(post->flags & CURL_HTTPPOST_LARGE) clen = post->contentlen;
                if(!clen) clen = -1;
                if(post->flags & (HTTPPOST_FILENAME | HTTPPOST_READFILE)) {
                    if(!strcmp(file->contents, "-")) {
                      result = curl_mime_data_cb(part, (curl_off_t) -1, (curl_read_callback) fread, CURLX_FUNCTION_CAST(curl_seek_callback,
                                                 fseek),NULL, (void *) stdin);
                    } else result = curl_mime_filedata(part, file->contents);
                    if(!result && (post->flags & HTTPPOST_READFILE)) result = curl_mime_filename(part, NULL);
                } else if(post->flags & HTTPPOST_BUFFER) result = curl_mime_data(part, post->buffer,post->bufferlength? post->bufferlength: -1);
                else if(post->flags & HTTPPOST_CALLBACK) result = curl_mime_data_cb(part, clen, fread_func, NULL, NULL, post->userp);
                else {
                    result = curl_mime_data(part, post->contents, (ssize_t) clen);
                #ifdef CURL_DOES_CONVERSIONS
                    if(!result && data && part->datasize) result = Curl_convert_to_network(data, part->data, part->datasize);
                #endif
                }
            }
            if(!result && post->showfilename) {
                if(post->more || (post->flags & (HTTPPOST_FILENAME | HTTPPOST_BUFFER | HTTPPOST_CALLBACK)))
                    result = curl_mime_filename(part, post->showfilename);
            }
        }
    }
    if(result) Curl_mime_cleanpart(finalform);
    return result;
}
#else
CURLFORMcode curl_formadd(struct curl_httppost **httppost, struct curl_httppost **last_post, ...) {
    (void)httppost;
    (void)last_post;
    return CURL_FORMADD_DISABLED;
}
int curl_formget(struct curl_httppost *form, void *arg, curl_formget_callback append) {
    (void) form;
    (void) arg;
    (void) append;
    return CURL_FORMADD_DISABLED;
}
void curl_formfree(struct curl_httppost *form) {
    (void)form;
}
#endif