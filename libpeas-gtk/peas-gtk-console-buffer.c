/*
 * peas-gtk-console-buffer.c
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

#include "peas-gtk-console-buffer.h"

#include <glib/gi18n.h>

struct _PeasGtkConsoleBufferPrivate {
  PeasInterpreter *interpreter;

  GtkTextMark *line_mark; /* what is this even for? */
  GtkTextMark *input_mark;

  GtkTextTag *uneditable_tag;

  gchar *statment;

  guint in_prompt : 2;
};

/* Properties */
enum {
  PROP_0,
  PROP_INTERPRETER
};

G_DEFINE_TYPE(PeasGtkConsoleBuffer, peas_gtk_console_buffer, GTK_TYPE_TEXT_BUFFER)

static void
update_statment (PeasGtkConsoleBuffer *buffer)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);

  if (!gtk_text_buffer_get_modified (text_buffer))
    return;

  if (buffer->priv->statment != NULL)
    {
      g_free (buffer->priv->statment);
      buffer->priv->statment = NULL;
    }

  if (buffer->priv->in_prompt)
    {
      GtkTextIter end, input;

      gtk_text_buffer_get_end_iter (text_buffer, &end);
      gtk_text_buffer_get_iter_at_mark (text_buffer,
                                        &input,
                                        buffer->priv->input_mark);

      buffer->priv->statment = gtk_text_buffer_get_text (text_buffer,
                                                         &input, &end,
                                                         FALSE);
    }

  gtk_text_buffer_set_modified (text_buffer, FALSE);
}

static void
write_newline (PeasGtkConsoleBuffer *buffer)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter end, iter;

  /* This must be before we delete the text */
  update_statment (buffer);

  gtk_text_buffer_get_end_iter (text_buffer, &end);
  peas_gtk_console_buffer_get_input_iter (buffer, &iter);

  gtk_text_buffer_delete (text_buffer, &iter, &end);

  if (buffer->priv->statment != NULL)
    gtk_text_buffer_insert_with_tags (text_buffer,
                                      &end, buffer->priv->statment, -1,
                                      buffer->priv->uneditable_tag,
                                      NULL);

  gtk_text_buffer_insert_with_tags (text_buffer,
                                    &end, "\n", -1,
                                    buffer->priv->uneditable_tag,
                                    NULL);

  /* maybe this should be a diffrent mark? */
  gtk_text_buffer_move_mark (text_buffer,
                             buffer->priv->input_mark,
                             &end);
}

static void
write_text (PeasGtkConsoleBuffer *buffer,
            const gchar          *text,
            GtkTextTag           *tag)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter end;

  if (buffer->priv->in_prompt)
    write_newline (buffer);

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  gtk_text_buffer_insert_with_tags (text_buffer,
                                    &end, text, -1,
                                    buffer->priv->uneditable_tag,
                                    tag,
                                    NULL);

  /* should we be using a diffrent mark? */
  gtk_text_buffer_move_mark (text_buffer,
                             buffer->priv->input_mark,
                             &end);

  buffer->priv->in_prompt = FALSE;
}

static void
interpreter_write_cb (PeasInterpreter      *interpreter,
                      const gchar          *text,
                      PeasGtkConsoleBuffer *buffer)
{
  if (text == NULL)
    return;

  write_text (buffer, text, NULL);
}

static void
interpreter_write_error_cb (PeasInterpreter      *interpreter,
                            const gchar          *text,
                            PeasGtkConsoleBuffer *buffer)
{
  gchar *error_text;

  if (text == NULL)
    return;

  error_text = g_strdup_printf ("Error ****: %s", text);

  write_text (buffer, error_text, NULL);

  g_free (error_text);
}

static void
interpreter_reset_cb (PeasInterpreter      *interpreter,
                      PeasGtkConsoleBuffer *buffer)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter (text_buffer, &start);
  gtk_text_buffer_get_end_iter (text_buffer, &end);

  gtk_text_buffer_delete (text_buffer, &start, &end);

  gtk_text_buffer_move_mark (text_buffer,
                             buffer->priv->line_mark,
                             &end);
  gtk_text_buffer_move_mark (text_buffer,
                             buffer->priv->input_mark,
                             &end);

  peas_gtk_console_buffer_write_prompt (buffer);
}

static void
peas_gtk_console_buffer_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      g_value_set_object (value, buffer->priv->interpreter);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_buffer_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      buffer->priv->interpreter = g_object_ref (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_buffer_constructed (GObject *object)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter end;

  gtk_text_buffer_set_modified (text_buffer, FALSE);

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  buffer->priv->line_mark = gtk_text_buffer_create_mark (text_buffer,
                                                         NULL, &end, TRUE);
  buffer->priv->input_mark = gtk_text_buffer_create_mark (text_buffer,
                                                          NULL, &end, TRUE);

  buffer->priv->uneditable_tag = gtk_text_buffer_create_tag (text_buffer,
                                                             NULL,
                                                             "editable", FALSE,
                                                             NULL);

  g_signal_connect (buffer->priv->interpreter,
                    "write",
                    G_CALLBACK (interpreter_write_cb),
                    buffer);
  g_signal_connect (buffer->priv->interpreter,
                    "write-error",
                    G_CALLBACK (interpreter_write_error_cb),
                    buffer);

  /* We need to get the reseted prompt which is only
   * available after the interpreter has been reset.
   */
  g_signal_connect_after (buffer->priv->interpreter,
                          "reset",
                          G_CALLBACK (interpreter_reset_cb),
                          buffer);

  /* Show the prompt */
  peas_gtk_console_buffer_write_prompt (buffer);
}

static void
peas_gtk_console_buffer_dispose (GObject *object)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);

  if (buffer->priv->interpreter != NULL)
    {
      g_signal_handlers_disconnect_by_func (buffer->priv->interpreter,
                                            G_CALLBACK (interpreter_write_cb),
                                            buffer);
      g_signal_handlers_disconnect_by_func (buffer->priv->interpreter,
                                            G_CALLBACK (interpreter_write_error_cb),
                                            buffer);
      g_signal_handlers_disconnect_by_func (buffer->priv->interpreter,
                                            G_CALLBACK (interpreter_reset_cb),
                                            buffer);

      g_object_unref (buffer->priv->interpreter);
      buffer->priv->interpreter = NULL;
    }

  if (buffer->priv->statment != NULL)
    {
      g_free (buffer->priv->statment);
      buffer->priv->statment = NULL;
    }
}

static void
peas_gtk_console_buffer_class_init (PeasGtkConsoleBufferClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = peas_gtk_console_buffer_get_property;
  object_class->set_property = peas_gtk_console_buffer_set_property;
  object_class->constructed = peas_gtk_console_buffer_constructed;
  object_class->dispose = peas_gtk_console_buffer_dispose;

  g_object_class_install_property (object_class,
                                   PROP_INTERPRETER,
                                   g_param_spec_object ("interpreter",
                                                        _("Interpreter"),
                                                        _("Interpreter"),
                                                        PEAS_TYPE_INTERPRETER,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (PeasGtkConsoleBufferPrivate));
}

static void
peas_gtk_console_buffer_init (PeasGtkConsoleBuffer *buffer)
{
  buffer->priv = G_TYPE_INSTANCE_GET_PRIVATE (buffer,
                                              PEAS_GTK_TYPE_CONSOLE_BUFFER,
                                              PeasGtkConsoleBufferPrivate);

  buffer->priv->statment = NULL;
}

/*
 * peas_gtk_console_buffer_new:
 * @interpreter: A #PeasInterpreter.
 *
 * Creates a new #PeasGtkConsoleBuffer with the given interpreter.
 *
 * Returns: a new #PeasGtkConsoleBuffer object.
 */
PeasGtkConsoleBuffer *
peas_gtk_console_buffer_new (PeasInterpreter *interpreter)
{
  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), NULL);

  return PEAS_GTK_CONSOLE_BUFFER (g_object_new (PEAS_GTK_TYPE_CONSOLE_BUFFER,
                                                "interpreter", interpreter,
                                                NULL));
}

/*
 * peas_gtk_console_buffer_get_interpreter:
 * @buffer: a #PeasGtkConsoleBuffer.
 *
 * Returns the #PeasInterpreter @buffer is attached to.
 *
 * Returns: the interpreter the buffer is attached to.
 */
PeasInterpreter *
peas_gtk_console_buffer_get_interpreter (PeasGtkConsoleBuffer *buffer)
{
  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer), NULL);

  return buffer->priv->interpreter;
}

void
peas_gtk_console_buffer_write_prompt (PeasGtkConsoleBuffer *buffer)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter input, end;
  gchar *prompt;

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));

  prompt = peas_interpreter_prompt (buffer->priv->interpreter);

  /* Is this enough to always be correctly on a newline? */
  peas_gtk_console_buffer_get_input_iter (buffer, &input);
  if (!gtk_text_iter_starts_line (&input))
    write_newline (buffer);

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  if (prompt != NULL)
    gtk_text_buffer_insert_with_tags (text_buffer,
                                      &end, prompt, -1,
                                      buffer->priv->uneditable_tag,
                                      NULL);

  gtk_text_buffer_move_mark (text_buffer,
                             buffer->priv->input_mark,
                             &end);

  buffer->priv->in_prompt = TRUE;

  g_free (prompt);
}

/* Weird but reduces errors because the
 * statment would change behind your back
 * quite often and thus cause segfaults.
 */
/*
 * peas_gtk_console_buffer_get_statment:
 * @buffer: A #PeasGtkConsoleBuffer.
 *
 * Returns the current statment or %NULL if not currently in prompt mode.
 *
 * Returns: the current statment.
 */
gchar *
peas_gtk_console_buffer_get_statment (PeasGtkConsoleBuffer *buffer)
{
  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer), NULL);

  update_statment (buffer);

  return g_strdup (buffer->priv->statment);
}

/*
 * peas_gtk_console_buffer_set_statment:
 * @buffer: A #PeasGtkConsoleBuffer.
 * @statment: text to set as the current statment.
 *
 * Replaces the current statment with @statment.
 *
 * Note: it is an error to call this if @buffer is not in prompt mode.
 */
void
peas_gtk_console_buffer_set_statment (PeasGtkConsoleBuffer *buffer,
                                      const gchar          *statment)
{
  GtkTextBuffer *text_buffer;
  GtkTextIter end, input;

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (statment != NULL);
  g_return_if_fail (buffer->priv->in_prompt);

  text_buffer = GTK_TEXT_BUFFER (buffer);

  if (buffer->priv->statment != NULL)
    g_free (buffer->priv->statment);

  gtk_text_buffer_get_end_iter (text_buffer, &end);
  peas_gtk_console_buffer_get_input_iter (buffer, &input);

  gtk_text_buffer_delete (text_buffer, &input, &end);
  gtk_text_buffer_insert (text_buffer, &end, statment, -1);

  buffer->priv->statment = g_strdup (statment);

  gtk_text_buffer_set_modified (text_buffer, FALSE);
}

/*
 * peas_gtk_console_buffer_get_input_mark:
 * @buffer: A #PeasGtkConsoleBuffer.
 *
 * Returns the input mark associated
 * with the starting position for input.
 *
 * Returns: the input mark.
 */
GtkTextMark *
peas_gtk_console_buffer_get_input_mark (PeasGtkConsoleBuffer *buffer)
{
  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer), NULL);

  return buffer->priv->input_mark;
}

/*
 * peas_gtk_console_buffer_get_input_iter:
 * @buffer: A #PeasGtkConsoleBuffer.
 * @iter: A #GtkTextIter.
 *
 * Sets @iter to the starting position for input.
 */
void
peas_gtk_console_buffer_get_input_iter (PeasGtkConsoleBuffer *buffer,
                                        GtkTextIter          *iter)
{
  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (iter != NULL);

  gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
                                    iter,
                                    buffer->priv->input_mark);
}

/*
 * peas_gtk_console_buffer_move_cursor:
 * @buffer: A #PeasGtkConsoleBuffer.
 * @iter: A #GtkTextIter.
 * @extend_selection: %TRUE if the move should extend the selection.
 *
 * Move the cursor according the @extend_selection.
 * If %TRUE then the selection is expanded to
 * @iter and if not the cursor is directly moved to @iter.
 */
void
peas_gtk_console_buffer_move_cursor (PeasGtkConsoleBuffer *buffer,
				                             GtkTextIter          *iter,
				                             gboolean              extend_selection)
{
  GtkTextBuffer *text_buffer;

	g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
	g_return_if_fail (iter != NULL);

  text_buffer = GTK_TEXT_BUFFER (buffer);

	if (!extend_selection)
    gtk_text_buffer_place_cursor (text_buffer, iter);
	else
	  {
      GtkTextMark *insert;

      insert = gtk_text_buffer_get_insert (text_buffer);
      gtk_text_buffer_move_mark (text_buffer, insert, iter);
	  }
}
