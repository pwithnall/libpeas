/*
 * testing-unimplementable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Garrett Regier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "testing-unimplementable.h"

#include <stdlib.h>

#include <glib.h>

G_DEFINE_INTERFACE(TestingUnimplementable, testing_unimplementable, G_TYPE_OBJECT)

void
testing_unimplementable_default_init (TestingUnimplementableInterface *iface)
{
  /* Shh really you can implement this but doing
   * an abort() causes gobject-introspection to fail
   *
   * No we cannot test g_get_prgname() as its not set
   */
}

