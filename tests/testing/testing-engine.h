/*
 * testing-engine.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
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

#if !defined (__TESTING_H_INSIDE__) && !defined (TESTING_COMPILATION)
#error "Only "testing.h" can be included directly."
#endif

#ifndef __TESTING_ENGINE_H__
#define __TESTING_ENGINE_H__

#include <libpeas/peas-engine.h>

G_BEGIN_DECLS

/*
 * Public methods
 */
PeasEngine *testing_engine_get_default (void);

G_END_DECLS

#endif /* __TESTING_ENGINE_H__ */
