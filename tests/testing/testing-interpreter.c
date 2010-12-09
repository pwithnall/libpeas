/*
 * testing-interpreter.c
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

#include "testing-interpreter.h"

#include <string.h>

#include <glib-object.h>


/*
 * This is a testinging interpreter.
 *
 * Its non-block prompt is ">>> " and its block prompt is "... ".
 *
 * It "understands" the following command:
 *   - "block", a block statement
 *   - "invalid", an invalid statement
 *   - "reset", causes a reset
 *   - "print ", writes the following text
 *   - "println ", writes the following text and a newline
 *   - "error ", writes an error with the following text
 *
 * Only the "invalid" command returns FALSE all others return TRUE.
 *
 * For commands leading whitespace and following text is ignored.
 * Everything else is a normal valid statement.
 *
 * It can complete 3 diffrent commands, the string must
 * start with "complete" and then be folled by:
 *
 *   - "full" -> "completed full"
 *   - "partial":
 *     - "has-prefix" -> "pre_a_blah", "pre_b" and "pre_c_blah"
 *     - "no-prefix" -> "a_blah", "b_blah" and "c_blah"
 *   - "nothing" -> NULL
 *
 * Anything else will cause it to assert.
 */

struct _TestingInterpreterPrivate {
  GObject *object;
  GString *code;
  gboolean in_block;
};

/* Properties */
enum {
  PROP_0,
  PROP_OBJECT
};

static void peas_interpreter_iface_init (PeasInterpreterInterface *iface);

G_DEFINE_TYPE_EXTENDED (TestingInterpreter,
                        testing_interpreter,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (PEAS_TYPE_INTERPRETER,
                                               peas_interpreter_iface_init))


static void
testing_interpreter_reset_cb (PeasInterpreter *peas_interpreter)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);

  interpreter->priv->in_block = FALSE;

  g_string_free (interpreter->priv->code, TRUE);
  interpreter->priv->code = g_string_new ("");
}

static gchar *
testing_interpreter_prompt (PeasInterpreter *peas_interpreter)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);

  if (!interpreter->priv->in_block)
    return g_strdup (">>> ");
  else
    return g_strdup ("... ");
};

static gboolean
testing_interpreter_execute (PeasInterpreter *peas_interpreter,
                          const gchar     *code)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);
  const gchar *command;

  if (code == NULL)
    return FALSE;

  for (command = code; *command != '\0'; ++command)
    {
      if (!g_ascii_isspace (*command))
        break;
    }

  if (g_str_has_prefix (command, "invalid"))
    return FALSE;

  if (g_str_has_prefix (command, "reset"))
    {
      peas_interpreter_reset (peas_interpreter);
      return TRUE;
    }

  if (g_str_has_prefix (command, "block"))
    interpreter->priv->in_block = TRUE;
  else
    interpreter->priv->in_block = FALSE;

  if (g_str_has_prefix (command, "print "))
    peas_interpreter_write (peas_interpreter, command + strlen ("print "));

  if (g_str_has_prefix (command, "println "))
    {
      peas_interpreter_write (peas_interpreter, command + strlen ("println "));
      peas_interpreter_write (peas_interpreter, "\n\n");
    }

  if (g_str_has_prefix (command, "error "))
    peas_interpreter_write_error (peas_interpreter, command + strlen ("error "));

  if (interpreter->priv->code->len != 0)
    g_string_append_c (interpreter->priv->code, '\n');

  g_string_append (interpreter->priv->code, code);

  return TRUE;
}

static gchar **
testing_interpreter_complete (PeasInterpreter *peas_interpreter,
                           const gchar     *code)
{
  const gchar *command;
  gchar **completions = NULL;

  if (code == NULL)
    return FALSE;

  for (command = code; *command != '\0'; ++command)
    {
      if (!g_ascii_isspace (*command))
        break;
    }

  g_assert (g_str_has_prefix (command, "complete "));

  command += strlen ("complete ");

  if (g_strcmp0 (command, "full") == 0)
    {
      completions = g_new (gchar *, 2);
      completions[0] = g_strdup ("completed full");
      completions[1] = NULL;
    }
  else if (g_strcmp0 (command, "partial has-prefix") == 0)
    {
      completions = g_new (gchar *, 4);
      completions[0] = g_strdup ("pre_a_blah");
      completions[1] = g_strdup ("pre_b");
      completions[2] = g_strdup ("pre_c_blah");
      completions[3] = NULL;
    }
  else if (g_strcmp0 (command, "partial no-prefix") == 0)
    {
      completions = g_new (gchar *, 4);
      completions[0] = g_strdup ("a_blah");
      completions[1] = g_strdup ("b");
      completions[2] = g_strdup ("c_blah");
      completions[3] = NULL;
    }
  else if (g_strcmp0 (command, "nothing") != 0)
    {
      g_assert_not_reached ();
    }

  return completions;
}

static void
peas_interpreter_iface_init (PeasInterpreterInterface *iface)
{
  iface->prompt = testing_interpreter_prompt;
  iface->execute = testing_interpreter_execute;
  iface->complete = testing_interpreter_complete;
}

static void
testing_interpreter_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      interpreter->priv->object = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_interpreter_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, interpreter->priv->object);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_interpreter_dispose (GObject *object)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (object);

  if (interpreter->priv->object != NULL)
    {
      g_object_unref (interpreter->priv->object);
      interpreter->priv->object = NULL;
    }

  if (interpreter->priv->code != NULL)
    {
      g_string_free (interpreter->priv->code, TRUE);
      interpreter->priv->code = NULL;
    }
}

static void
testing_interpreter_class_init (TestingInterpreterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = testing_interpreter_get_property;
  object_class->set_property = testing_interpreter_set_property;
  object_class->dispose = testing_interpreter_dispose;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (TestingInterpreterPrivate));
}

static void
testing_interpreter_init (TestingInterpreter *interpreter)
{
  interpreter->priv = G_TYPE_INSTANCE_GET_PRIVATE (interpreter,
                                                   TESTING_TYPE_INTERPRETER,
                                                   TestingInterpreterPrivate);

  interpreter->priv->object = NULL;
  interpreter->priv->code = g_string_new ("");
  interpreter->priv->in_block = FALSE;

  g_signal_connect (interpreter,
                    "reset",
                    G_CALLBACK (testing_interpreter_reset_cb),
                    interpreter);
}

/**
 * testing_interpreter_new:
 *
 * Returns: (transfer full):
 */
PeasInterpreter *
testing_interpreter_new (void)
{
  return PEAS_INTERPRETER (g_object_new (TESTING_TYPE_INTERPRETER, NULL));
}

const gchar *
testing_interpreter_get_code (TestingInterpreter *interpreter)
{
  g_return_val_if_fail (TESTING_IS_INTERPRETER (interpreter), "");

  return interpreter->priv->code->str;
}

