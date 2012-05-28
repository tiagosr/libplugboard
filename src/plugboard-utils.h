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
#define p_outlet_bang(obj,outlet) \
        p_outlet_bang((t_plugobj *)obj,outlet)
#define p_outlet_send_int(obj,outlet, data) \
        p_outlet_send_int((t_plugobj *)obj,outlet,data)
#define p_outlet_send_float(obj,outlet,data) \
        p_outlet_send_float((t_plugobj *)obj,outlet,data)
#define p_outlet_send_sym(obj,outlet,data) \
        p_outlet_send_sym((t_plugobj *)obj,outlet,data)
#define p_outlet_send_string(obj,outlet,data) \
        p_outlet_send_string((t_plugobj *)obj,outlet,data)
#define p_outlet_send_list(obj,outlet,data) \
        p_outlet_send_list((t_plugobj *)obj,outlet,data)
#define p_outlet_send_any(obj,outlet,any) \
        p_outlet_send_any((t_plugobj *)obj,outlet,any)

#define p_send_redraw(obj,data) \
        p_send_redraw((t_plugobj *)obj,data)

#define p_add_outlet(obj,name) \
        p_add_outlet((t_plugobj *)obj,gen_sym(name))
#define p_add_inlet(obj, name, fns, i_obj) \
        p_add_inlet((t_plugobj *)obj, gen_sym(name), fns, i_obj)
#define p_begin_inlet_fn_list(varname) \
        t_inlet_fn varname [] = {
#define p_end_inlet_fn_list \
            {0, NULL}\
        };
#define p_inlet_fn_trigger(fn) {t_type_trigger, .inlet_trigger = fn }
#define p_inlet_fn_int(fn) {t_type_int, .inlet_int = fn }
#define p_inlet_fn_float(fn) {t_type_float, .inlet_float = fn }
#define p_inlet_fn_any(fn) {t_type_any, .inlet_any = fn }
#define p_inlet_fn_sym(fn) {t_type_sym, .inlet_sym = fn }
#define p_inlet_fn_string(fn) {t_type_string, .inlet_string = fn }
#define p_inlet_fn_list(fn) {t_type_list, .inlet_list = fn }
#endif
