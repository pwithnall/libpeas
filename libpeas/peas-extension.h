/*
 * peas-extension.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Steve Frécinaux
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

#ifndef __PEAS_EXTENSION_H__
#define __PEAS_EXTENSION_H__

#include <glib-object.h>
#include <girepository.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_EXTENSION            (peas_extension_get_type())
#define PEAS_EXTENSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_TYPE_EXTENSION, PeasExtension))
#define PEAS_EXTENSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_TYPE_EXTENSION, PeasExtensionClass))
#define PEAS_IS_EXTENSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_TYPE_EXTENSION))
#define PEAS_IS_EXTENSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_EXTENSION))
#define PEAS_EXTENSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_TYPE_EXTENSION, PeasExtensionClass))

typedef struct _PeasExtension         PeasExtension;
typedef struct _PeasExtensionClass    PeasExtensionClass;
typedef struct _PeasExtensionPrivate  PeasExtensionPrivate;

/**
 * PeasExtension:
 *
 * The #PeasExtension structure contains only private data and should only be
 * accessed using the provided API.
 */
struct _PeasExtension {
  GObject parent;

  /*< private >*/
  PeasExtensionPrivate *priv;
};

/**
 * PeasExtensionClass:
 *
 * The #PeasExtensionClass structure contains only private data and should
 * only be accessed using the provided API.  You should not inherit from this
 * class.
 */
struct _PeasExtensionClass {
  GObjectClass parent_class;

  /*< private >*/
  gboolean   (*call)                      (PeasExtension  *exten,
                                           const gchar    *method,
                                           GIArgument     *args,
                                           GIArgument     *return_value);
};

/*
 * Public methods
 */
GType        peas_extension_get_type        (void)  G_GNUC_CONST;

GType        peas_extension_get_extension_type
                                            (PeasExtension *exten);

gboolean     peas_extension_call            (PeasExtension *exten,
                                             const gchar   *method_name,
                                             ...);
gboolean     peas_extension_call_valist     (PeasExtension *exten,
                                             const gchar   *method_name,
                                             va_list        args);
gboolean     peas_extension_callv           (PeasExtension *exten,
                                             const gchar   *method_name,
                                             GIArgument    *args,
                                             GIArgument    *return_value);

G_END_DECLS

#endif /* __PEAS_EXTENSION_H__ */
