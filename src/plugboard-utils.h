//
//  plugobj-utils.h
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/5/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#ifndef livecode_plugboard_plugobj_utils_h
#define livecode_plugboard_plugobj_utils_h
#include "plugboard.h"

/*
 * typing savers.
 */
#define o_bang(outlet) \
        p_outlet_bang(outlet)
#define o_int(outlet, data) \
        p_outlet_send_int(outlet,data)
#define o_float(outlet,data) \
        p_outlet_send_float(outlet,data)
#define o_sym(outlet,data) \
        p_outlet_send_sym(outlet,data)
#define o_string(outlet,data) \
        p_outlet_send_string(outlet,data)
#define o_list(outlet,data) \
        p_outlet_send_list(outlet,data)
#define o_any(outlet,any) \
        p_outlet_send_any(outlet,any)

#define p_redraw(obj,data) \
        p_send_redraw((t_plugobj *)obj,data)

#define add_outlet(obj,name) \
        p_add_outlet((t_plugobj *)obj,gen_sym(name))
#define add_inlet(obj, name, i_obj) \
        p_add_inlet((t_plugobj *)obj, gen_sym(name), i_obj)


#define i_setter(name, type, struct_type, field) \
void name (void *obj, type __data, void *idata) { \
    ((struct_type *)obj)->field = __data;\
}

#define i_string_setter(name, struct_type, field) \
void name (void *obj, const char* __data, void *idata) { \
    struct_type *tobj = obj;\
    free(tobj->field); \
    if (!__data) { tobj->field = strdup(""); } \
    else { tobj->field = strdup(__data); } \
}

#define i_bangsender(name, type, struct_type, field, outlet)\
void name (void *obj, void *idata) { \
    struct_type *tobj = obj; \
    p_outlet_send_##type(tobj->outlet, tobj->field); \
}

#endif
