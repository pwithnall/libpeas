/*
 * peas-interpreter.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#include "peas-interpreter.h"

/**
 * SECTION:peas-interpreter
 * @short_description: Interface for interpreter extensions
 **/

/* Signals */
enum {
  WRITE,
  WRITE_ERROR,
  RESET,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_INTERFACE(PeasInterpreter, peas_interpreter, G_TYPE_OBJECT)

void
peas_interpreter_default_init (PeasInterpreterInterface *iface)
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;
    
  /**
   * PeasInterpreter:object:
   *
   * The object property contains the targetted object for this
   * #PeasInterpreter instance, for example a toplevel window in a typical
   * windowed application. It is set at construction time and won't change.
   */
  g_object_interface_install_property (iface,
                                       g_param_spec_object ("object",
                                                            "Object",
                                                            "Object",
                                                            G_TYPE_OBJECT,
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_STATIC_STRINGS));


  /**
   * PeasInterpreter::write:
   * @interpreter: A #PeasInterpreter.
   * @text: the text to write.
   *
   * The write signal is used by a console to write
   * @text to its buffer.
   */
  signals[WRITE] =
    g_signal_new_class_handler ("write",
                                PEAS_TYPE_INTERPRETER,
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                NULL,
                                NULL, NULL,
                                g_cclosure_marshal_VOID__STRING,
                                G_TYPE_NONE,
                                1, G_TYPE_STRING);

  /**
   * PeasInterpreter::write-error:
   * @interpreter: A #PeasInterpreter.
   * @text: the text to write as an error.
   *
   * The error signal is used by a console to write
   * @text as an error message to its buffer.
   */
  signals[WRITE_ERROR] =
    g_signal_new_class_handler ("write-error",
                                PEAS_TYPE_INTERPRETER,
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                NULL,
                                NULL, NULL,
                                g_cclosure_marshal_VOID__STRING,
                                G_TYPE_NONE,
                                1, G_TYPE_STRING);

  /**
   * PeasInterpreter::reset:
   * @interpreter: A #PeasInterpreter.
   *
   * The reset signal is used by a console to reset its internal state.
   *
   * The interpreter should also connect to this signal to
   * properly reset when a reset keybinding is activated.
   */
  signals[RESET] =
    g_signal_new_class_handler ("reset",
                                PEAS_TYPE_INTERPRETER,
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                NULL,
                                NULL, NULL,
                                g_cclosure_marshal_VOID__VOID,
                                G_TYPE_NONE,
                                0);

  initialized = TRUE;
}

/**
 * peas_interpreter_complete:
 * @interpreter: A #PeasInterpreter.
 * @code: code to complete.
 *
 * Returns a list of possible completions.
 *
 * Returns: (transfer full): a newly-allocated %NULL-terminated
 * list of completions, or %NULL.
 */
gchar **
peas_interpreter_complete (PeasInterpreter *interpreter,
                           const gchar     *code)
{
  PeasInterpreterInterface *iface;

  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), NULL);
  g_return_val_if_fail (code != NULL, NULL);

  iface = PEAS_INTERPRETER_GET_IFACE (interpreter);
  if (iface->complete != NULL)
    return iface->complete (interpreter, code);

  return NULL;
}

/**
 * peas_interpreter_execute:
 * @interpreter: A #PeasInterpreter.
 * @code: statement to execute.
 *
 * Executes @code and returns if it did not
 * cause an error, for instance if it was invalid.
 *
 * Returns: if an error did not occurred.
 */
gboolean
peas_interpreter_execute (PeasInterpreter *interpreter,
                          const gchar     *code)
{
  PeasInterpreterInterface *iface;

  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), FALSE);
  g_return_val_if_fail (code != NULL, FALSE);

  iface = PEAS_INTERPRETER_GET_IFACE (interpreter);
  if (iface->execute != NULL)
    return iface->execute (interpreter, code);

  return FALSE;
}

/**
 * peas_interpreter_prompt:
 * @interpreter: A #PeasInterpreter.
 *
 * Returns: the @interpreter's prompt.
 */
gchar *
peas_interpreter_prompt (PeasInterpreter *interpreter)
{
  PeasInterpreterInterface *iface;

  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), NULL);

  iface = PEAS_INTERPRETER_GET_IFACE (interpreter);
  if (iface->prompt != NULL)
    return iface->prompt (interpreter);

  return NULL;
}

/**
 * peas_interpreter_write:
 * @interpreter: A #PeasInterpreter.
 * @text: text to write.
 *
 * Emits the "write" signal for @text.
 */
void
peas_interpreter_write (PeasInterpreter *interpreter,
                        const gchar     *text)
{
  g_return_if_fail (PEAS_IS_INTERPRETER (interpreter));
  g_return_if_fail (text != NULL);

  g_signal_emit (interpreter, signals[WRITE], 0, text);
}

/**
 * peas_interpreter_write_error:
 * @interpreter: A #PeasInterpreter.
 * @text: text to write as an error.
 *
 * Emits the "write-error" signal for @text.
 */
void
peas_interpreter_write_error (PeasInterpreter *interpreter,
                              const gchar     *text)
{
  g_return_if_fail (PEAS_IS_INTERPRETER (interpreter));
  g_return_if_fail (text != NULL);

  g_signal_emit (interpreter, signals[WRITE_ERROR], 0, text);
}

/**
 * peas_interpreter_reset:
 * @interpreter: A #PeasInterpreter.
 *
 * Emits the "reset" signal.
 */
void
peas_interpreter_reset (PeasInterpreter *interpreter)
{
  g_return_if_fail (PEAS_IS_INTERPRETER (interpreter));

  g_signal_emit (interpreter, signals[RESET], 0);
}
