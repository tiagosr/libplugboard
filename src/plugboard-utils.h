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
#define o_bang(obj,outlet) \
        p_outlet_bang((t_plugobj *)obj,outlet)
#define o_int(obj,outlet, data) \
        p_outlet_send_int((t_plugobj *)obj,outlet,data)
#define o_float(obj,outlet,data) \
        p_outlet_send_float((t_plugobj *)obj,outlet,data)
#define o_sym(obj,outlet,data) \
        p_outlet_send_sym((t_plugobj *)obj,outlet,data)
#define o_string(obj,outlet,data) \
        p_outlet_send_string((t_plugobj *)obj,outlet,data)
#define o_list(obj,outlet,data) \
        p_outlet_send_list((t_plugobj *)obj,outlet,data)
#define o_any(obj,outlet,any) \
        p_outlet_send_any((t_plugobj *)obj,outlet,any)

#define p_redraw(obj,data) \
        p_send_redraw((t_plugobj *)obj,data)

#define add_outlet(obj,name) \
        p_add_outlet((t_plugobj *)obj,gen_sym(name))
#define add_inlet(obj, name, fns, i_obj) \
        p_add_inlet((t_plugobj *)obj, gen_sym(name), fns, i_obj)
#define inlet_fn_list(varname) \
        t_inlet_fn varname [] = {
#define inlet_fn_list_end \
            {0, NULL}\
        };

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

#define i_fn_trigger(fn) {t_type_trigger, .inlet_trigger = fn }
#define i_fn_int(fn) {t_type_int, .inlet_int = fn }
#define i_fn_float(fn) {t_type_float, .inlet_float = fn }
#define i_fn_any(fn) {t_type_any, .inlet_any = fn }
#define i_fn_sym(fn) {t_type_sym, .inlet_sym = fn }
#define i_fn_string(fn) {t_type_string, .inlet_string = fn }
#define i_fn_list(fn) {t_type_list, .inlet_list = fn }

#define inlet_trigger_fn(name, fn) t_inlet_fn name [] = \
            { {t_type_trigger, .inlet_trigger = fn }, {0, NULL} }
#define inlet_int_fn(name, fn) t_inlet_fn name [] = \
            { {t_type_int, .inlet_int = fn }, {0, NULL} }
#define inlet_float_fn(name, fn) t_inlet_fn name [] = \
            { {t_type_float, .inlet_float = fn }, {0, NULL} }
#define inlet_any_fn(name, fn) t_inlet_fn name [] = \
            { {t_type_any, .inlet_any = fn }, {0, NULL} };
#define inlet_string_fn(name, fn) t_inlet_fn name [] = \
            { {t_type_string, .inlet_string = fn }, {0, NULL} };
#define inlet_list_fn(name, fn) t_inlet_fn name [] = \
            { {t_type_list, .inlet_list = fn }, {0, NULL} };

#endif
