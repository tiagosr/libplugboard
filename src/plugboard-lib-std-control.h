//
//  plugboard-lib-std-control.h
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/5/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#ifndef livecode_plugboard_plugboard_lib_std_control_h
#define livecode_plugboard_plugboard_lib_std_control_h
#include "plugboard-lib-std.h"

typedef struct t_patch t_patch;

t_patch *p_create_patch(void);
void p_patch_set_current(t_patch *patch);
void p_patch_current_add_subpatch(t_patch *subpatch, t_sym name);

void p_perform_loadbang(t_patch *patch);

void p_std_control_setup(void);

#endif
