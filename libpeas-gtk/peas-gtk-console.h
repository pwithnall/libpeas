/*
 * peas-gtk-console.h
 * This file is part of libpeas.
 *
 * Copyright (C) 2010 - Garrett Regier
 *
 * libpeas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libpeas.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PEAS_GTK_CONSOLE_H__
#define __PEAS_GTK_CONSOLE_H__

#include <gtk/gtk.h>

#include <libpeas/peas-interpreter.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_GTK_TYPE_CONSOLE            (peas_gtk_console_get_type())
#define PEAS_GTK_CONSOLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_GTK_TYPE_CONSOLE, PeasGtkConsole))
#define PEAS_GTK_CONSOLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_GTK_TYPE_CONSOLE, PeasGtkConsoleClass))
#define PEAS_GTK_IS_CONSOLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_GTK_TYPE_CONSOLE))
#define PEAS_GTK_IS_CONSOLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_GTK_TYPE_CONSOLE))
#define PEAS_GTK_CONSOLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_GTK_TYPE_CONSOLE, PeasGtkConsoleClass))

typedef struct _PeasGtkConsole        PeasGtkConsole;
typedef struct _PeasGtkConsoleClass   PeasGtkConsoleClass;
typedef struct _PeasGtkConsolePrivate PeasGtkConsolePrivate;

struct _PeasGtkConsole {
  GtkScrolledWindow parent;

  /*< private >*/
  PeasGtkConsolePrivate *priv;
};

struct _PeasGtkConsoleClass {
  GtkScrolledWindowClass parent_class;
};

GType            peas_gtk_console_get_type         (void) G_GNUC_CONST;

GtkWidget       *peas_gtk_console_new              (PeasInterpreter *interpreter);

PeasInterpreter *peas_gtk_console_get_interpreter  (PeasGtkConsole  *console);

G_END_DECLS

#endif  /* __PEAS_GTK_CONSOLE_H__  */
