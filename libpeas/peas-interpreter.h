/*
 * peas-interpreter.h
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

#ifndef __PEAS_INTERPRETER_H__
#define __PEAS_INTERPRETER_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_INTERPRETER             (peas_interpreter_get_type ())
#define PEAS_INTERPRETER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_INTERPRETER, PeasInterpreter))
#define PEAS_INTERPRETER_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), PEAS_TYPE_INTERPRETER, PeasInterpreterInterface))
#define PEAS_IS_INTERPRETER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_INTERPRETER))
#define PEAS_INTERPRETER_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PEAS_TYPE_INTERPRETER, PeasInterpreterInterface))

typedef struct _PeasInterpreter           PeasInterpreter; /* dummy typedef */
typedef struct _PeasInterpreterInterface  PeasInterpreterInterface;

struct _PeasInterpreterInterface {
  GTypeInterface g_iface;

  /* Virtual methods */
  gchar    **(*complete)    (PeasInterpreter *interpreter,
                             const gchar     *code);
  gboolean   (*execute)     (PeasInterpreter *interpreter,
                             const gchar     *code);
  gchar     *(*prompt)      (PeasInterpreter *interpreter);

  /* Signals *//*
  void    (*write)       (PeasInterpreter *interpreter,
                          const gchar     *text);
  void    (*write_error) (PeasInterpreter *interpreter,
                          const gchar     *text);
  void    (*reset)       (PeasInterpreter *interpreter,
                          const gchar     *text);*/
};

/*
 * Public methods
 */
GType      peas_interpreter_get_type    (void)  G_GNUC_CONST;

gchar    **peas_interpreter_complete    (PeasInterpreter *interpreter,
                                         const gchar     *code);
gboolean   peas_interpreter_execute     (PeasInterpreter *interpreter,
                                         const gchar     *code);
gchar     *peas_interpreter_prompt      (PeasInterpreter *interpreter);

void       peas_interpreter_write       (PeasInterpreter *interpreter,
                                         const gchar     *text);
void       peas_interpreter_write_error (PeasInterpreter *interpreter,
                                         const gchar     *text);
void       peas_interpreter_reset       (PeasInterpreter *interpreter);

G_END_DECLS

#endif /* __PEAS_INTERPRETER_H__ */
