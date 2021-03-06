/*
 * peas-dirs.h
 * This file is part of libpeas
 *
 * Copyright (C) 2008 Ignacio Casal Quinteiro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef __PEAS_DIRS_H__
#define __PEAS_DIRS_H__

#include <glib.h>

G_BEGIN_DECLS

gchar  *peas_dirs_get_data_dir           (void);
gchar  *peas_dirs_get_lib_dir            (void);
gchar  *peas_dirs_get_plugin_loaders_dir (void);
gchar  *peas_dirs_get_locale_dir         (void);

G_END_DECLS

#endif /* __PEAS_DIRS_H__ */
