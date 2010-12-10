/*
 * testing-unimplementable.h
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

#ifndef __TESTING_UNIMPLEMENTABLE_H__
#define __TESTING_UNIMPLEMENTABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define TESTING_TYPE_UNIMPLEMENTABLE             (testing_unimplementable_get_type ())
#define TESTING_UNIMPLEMENTABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TESTING_TYPE_UNIMPLEMENTABLE, TestingUnimplementable))
#define TESTING_UNIMPLEMENTABLE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), TESTING_TYPE_UNIMPLEMENTABLE, TestingUnimplementableInterface))
#define TESTING_IS_UNIMPLEMENTABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TESTING_TYPE_UNIMPLEMENTABLE))
#define TESTING_UNIMPLEMENTABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TESTING_TYPE_UNIMPLEMENTABLE, TestingUnimplementableInterface))

typedef struct _TestingUnimplementable           TestingUnimplementable; /* dummy typedef */
typedef struct _TestingUnimplementableInterface  TestingUnimplementableInterface;

struct _TestingUnimplementableInterface {
  GTypeInterface g_iface;
};

/*
 * Public methods
 */
GType testing_unimplementable_get_type (void)  G_GNUC_CONST;

G_END_DECLS

#endif /* __TESTING_UNIMPLEMENTABLE_H__ */
