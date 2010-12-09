/*
 * peas-gtk-console.c
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

#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include "peas-gtk-console.h"

#include "peas-gtk-console-buffer.h"
#include "peas-gtk-console-history.h"

struct _PeasGtkConsolePrivate {
  GSettings *console_settings;
  GSettings *desktop_settings;
  GSettings *terminal_settings;

  PeasInterpreter *interpreter;
  PeasGtkConsoleHistory *history;

  /* use GtkSourceView? */
  GtkTextView *text_view;
  PeasGtkConsoleBuffer *buffer;

  gchar *history_statement;

  guint scroll_id;
};

/* Properties */
enum {
  PROP_0,
  PROP_INTERPRETER
};

G_DEFINE_TYPE(PeasGtkConsole, peas_gtk_console, GTK_TYPE_SCROLLED_WINDOW)

#define DEFAULT_FONT "Monospace 10"

 /* Not implemented */
/*#define CONSOLE_SCHEMA  "org.gnome.libpeas.console"*/
#define DESKTOP_SCHEMA  "org.gnome.desktop.interface"
#define TERMINAL_SCHEMA "org.gnome.GnomeTerminal.profiles.Default"

/*
#define PEAS_GTK_CONSOLE_PROMPT_COLOR "prompt-color"
#define PEAS_GTK_CONSOLE_ERROR_COLOR  "error-color"
*/

#ifndef g_clear_object
#define g_clear_object(obj_ptr) \
  G_STMT_START \
    { \
      g_object_unref ((gpointer) *obj_ptr); \
\
      *obj_ptr = NULL; \
    } \
  G_STMT_END      
#endif

static gchar *
get_system_font (PeasGtkConsole *console)
{
  if (console->priv->desktop_settings == NULL)
    return NULL;

  if (g_settings_get_boolean (console->priv->desktop_settings, "use-custom-font"))
    return NULL;

  return g_settings_get_string (console->priv->desktop_settings,
                                "monospace-font-name");
}

static gchar *
get_terminal_font (PeasGtkConsole *console)
{
  /* We should use the current profile instead of the default profile */

  if (console->priv->terminal_settings == NULL)
    return NULL;

  if (g_settings_get_boolean (console->priv->terminal_settings, "use-system-font"))
    return NULL;

  return g_settings_get_string (console->priv->terminal_settings, "font");
}

static void
font_settings_changed_cb (GSettings      *settings,
                          gchar          *key,
                          PeasGtkConsole *console)
{
  const gchar *font_name;
  gchar *system_font;
  gchar *terminal_font;
  PangoFontDescription *font_desc;

  system_font = get_system_font (console);
  terminal_font = get_terminal_font (console);

  if (terminal_font != NULL)
    font_name = terminal_font;
  else if (system_font != NULL)
    font_name = system_font;
  else
    font_name = DEFAULT_FONT;

  font_desc = pango_font_description_from_string (font_name);

  if (font_desc == NULL)
    {
      if (system_font != NULL && g_strcmp0 (font_name, system_font) != 0)
        font_desc = pango_font_description_from_string (system_font);

      if (font_desc == NULL && g_strcmp0 (font_name, DEFAULT_FONT) != 0)
        font_desc = pango_font_description_from_string (DEFAULT_FONT);
    }

  if (font_desc != NULL)
    {
      gtk_widget_override_font (GTK_WIDGET (console->priv->text_view),
                                font_desc);

      pango_font_description_free (font_desc);
    }

  if (terminal_font != NULL)
    g_free (terminal_font);

  if (system_font != NULL)
    g_free (system_font);
}

static GSettings *
settings_try_new (const gchar *schema)
{
  guint i;
  const gchar * const *schemas;

  schemas = g_settings_list_schemas ();

  if (schemas == NULL)
    return NULL;

  for (i = 0; schemas[i] != NULL; ++i)
    {
      if (g_strcmp0 (schemas[i], schema) == 0)
        return g_settings_new (schema);
    }

  return NULL;
}

static gboolean
real_scroll_to_end (PeasGtkConsole *console)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (console->priv->buffer);
  GtkTextIter end;

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  /* why?? */
  gtk_text_buffer_place_cursor (text_buffer, &end);

  gtk_text_view_scroll_to_iter (console->priv->text_view,
                                &end, 0, TRUE, 0, 1);

  console->priv->scroll_id = 0;
  return FALSE;
}

static void
scroll_to_end (PeasGtkConsole *console)
{
  if (console->priv->scroll_id != 0)
    return;

  console->priv->scroll_id = g_idle_add ((GSourceFunc) real_scroll_to_end,
                                         console);
}

static gint
compare_input (PeasGtkConsole *console)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (console->priv->buffer);
  GtkTextMark *insert_mark;
  GtkTextIter insert, input;

  insert_mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer,
                                    &insert,
                                    insert_mark);

  peas_gtk_console_buffer_get_input_iter (console->priv->buffer, &input);

  return gtk_text_iter_compare (&insert, &input);
}

static gboolean
selection_start_before_input (PeasGtkConsole *console)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (console->priv->buffer);
  GtkTextMark *selection_bound_mark;
  GtkTextIter input, selection_bound;

  selection_bound_mark = gtk_text_buffer_get_selection_bound (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer,
                                    &selection_bound,
                                    selection_bound_mark);

  peas_gtk_console_buffer_get_input_iter (console->priv->buffer, &input);

  return gtk_text_iter_compare (&selection_bound, &input) < 0;
}

static void
history_changed (PeasGtkConsole *console)
{
  const gchar *statement;

  if (console->priv->history_statement == NULL)
    {
      console->priv->history_statement =
                peas_gtk_console_buffer_get_statment (console->priv->buffer);
    }

  statement = peas_gtk_console_history_get (console->priv->history);

  if (statement != NULL)
    peas_gtk_console_buffer_set_statment (console->priv->buffer, statement);
  else if (console->priv->history_statement != NULL)
    {
      peas_gtk_console_buffer_set_statment (console->priv->buffer,
                                            console->priv->history_statement);

      g_free (console->priv->history_statement);
      console->priv->history_statement = NULL;
    }

  scroll_to_end (console);
}


static gchar *
get_whitespace (PeasGtkConsole *console)
{
  gchar *input;
  gchar *pos;
  gsize length = 0;
  gchar *whitespace;

  input = peas_gtk_console_buffer_get_statment (console->priv->buffer);

  if (input == NULL)
    return g_strdup ("");

  for (pos = input; *pos != '\0'; ++length, ++pos)
    {
      if (!g_ascii_isspace (*pos))
        break;
    }

  whitespace = g_strndup (input, length);

  g_free (input);

  return whitespace;
}

static gboolean
move_left_cb (PeasGtkConsole *console,
              gboolean        extend_selection)
{
  GtkTextIter input;

  if (compare_input (console) > 0 || selection_start_before_input (console))
    return FALSE;

  peas_gtk_console_buffer_get_input_iter (console->priv->buffer, &input);
  peas_gtk_console_buffer_move_cursor (console->priv->buffer,
                                       &input,
                                       extend_selection);

  return TRUE;
}

/* the following two functions should be refactored
 * the smart home/end is broken with multiple lines
 * should support the range of smart/home
 * smart end is broken goes too far to the left by 1
 */
static gboolean
move_home_cb (PeasGtkConsole *console,
              gboolean        extend_selection)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (console->priv->buffer);
  GtkTextMark *insert_mark;
  GtkTextIter insert, iter;

/*
  if (selection started before input)
    return FALSE;
*/

  insert_mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer, &insert, insert_mark);

  peas_gtk_console_buffer_get_input_iter (console->priv->buffer, &iter);

  while (g_ascii_isspace (gtk_text_iter_get_char (&iter)))
    {
      if (!gtk_text_iter_forward_char (&iter))
        break;
    }

  if (gtk_text_iter_equal (&insert, &iter))
    peas_gtk_console_buffer_get_input_iter (console->priv->buffer, &iter);

  peas_gtk_console_buffer_move_cursor (console->priv->buffer,
                                       &iter, extend_selection);

  return TRUE;
}

static gboolean
move_end_cb (PeasGtkConsole *console,
             gboolean        extend_selection)
{
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (console->priv->buffer);
  GtkTextMark *insert_mark;
  GtkTextIter insert, iter, end;

/*
  if (selection started before input)
    return FALSE;
*/

  insert_mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer,
                                    &insert,
                                    insert_mark);

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  iter = end;
  do
    {
      if (!gtk_text_iter_backward_char (&iter))
        break;
    } while (g_ascii_isspace (gtk_text_iter_get_char (&iter)));

  gtk_text_iter_forward_char (&iter);

  if (gtk_text_iter_equal (&insert, &iter))
    gtk_text_buffer_get_end_iter (text_buffer, &iter);

  peas_gtk_console_buffer_move_cursor (console->priv->buffer,
                                       &iter, extend_selection);

  return TRUE;
}

static gboolean
move_up_cb (PeasGtkConsole *console,
            gboolean        extend_selection)
{
  peas_gtk_console_history_next (console->priv->history);
  history_changed (console);

  return TRUE;
}

static gboolean
move_down_cb (PeasGtkConsole *console,
              gboolean        extend_selection)
{
  /* FIXME: is this named correctly? */
  peas_gtk_console_history_previous (console->priv->history);
  history_changed (console);

  return TRUE;
}

static void
move_cursor_cb (GtkTextView     *text_view,
                GtkMovementStep  step,
                gint             count,
                gboolean         extend_selection,
                PeasGtkConsole  *console)
{
  gboolean handled;

  //g_message (g_strdup_printf ("Step: %i, Count: %i", step, count));

  switch (step)
    {
    case GTK_MOVEMENT_VISUAL_POSITIONS:
      if (count < 0)
        handled = move_left_cb (console, extend_selection);
      else
        /*handled = move_right_cb (console, extend_selection);*/
        handled = FALSE;
      break;
    case GTK_MOVEMENT_DISPLAY_LINES:
      if (count < 0)
        handled = move_up_cb (console, extend_selection);
      else
        handled = move_down_cb (console, extend_selection);
      break;
    case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
      if (count < 0)
        handled = move_home_cb (console, extend_selection);
      else
        handled = move_end_cb (console, extend_selection);
      break;
    default:
      handled = FALSE;
      break;
    }

  /* No idea if we should do something special for these:
   * GTK_MOVEMENT_PARAGRAPHS, GTK_MOVEMENT_BUFFER_ENDS, GTK_MOVEMENT_PAGES
   */

  if (handled)
    g_signal_stop_emission_by_name (text_view, "move-cursor");
}

static void
move_cursor_after_cb (GtkTextView     *text_view,
                      GtkMovementStep  step,
                      gint             count,
                      gboolean         extend_selection,
                      PeasGtkConsole  *console)
{
  /* We should have an after handler for certain steps, like:
   * GTK_MOVEMENT_WORDS, GTK_MOVEMENT_HORIZONTAL_PAGES
   */

  /* - We don't care if we should extend the selection
   * - If we are not a step that we care about we don't care
   * - If we are on the input line and not after the prompt
   *   - Move to after the prompt
   */

  if (extend_selection)
    return;

  if (step != GTK_MOVEMENT_WORDS &&
      step != GTK_MOVEMENT_HORIZONTAL_PAGES)
    return;
}

static gchar *
get_common_string_prefix (const gchar **strings)
{
  gint i, j;
  gint short_string_len;
  const gchar *short_string;
  gchar *prefix = NULL;

  g_assert (strings[0] != NULL);

  short_string = strings[0];
  short_string_len = strlen (short_string);

  for (i = 1; strings[i] != NULL; ++i)
    {
      gint string_len = strlen (strings[i]);

      if (short_string_len > string_len)
        {
          short_string = strings[i];
          short_string_len = string_len;
        }
    }

  for (i = 0; i < short_string_len; ++i)
    {
      for (j = 0; strings[j] != NULL; ++j)
        {
          if (strings[j][i] != short_string[i])
            {
              prefix = g_strndup (short_string, i);
              goto out;
            }
        }
    }

out:
  
  if (prefix != NULL && prefix[0] == '\0')
    {
      g_free (prefix);
      return NULL;
    }

  return prefix;
}

/* OLD: This is gross to use! replace with a < 1 second timeout that will
 * remove itself and if its not removed and this is reached again
 * output the completion, also use tab instead of space
 */
static void
peas_gtk_console_complete (PeasGtkConsole *console)
{
  gchar *statment;
  gchar **completions;
  gchar *whitespace;
  GString *new_statment;

  /* Right now completion causes a crash due to g-o-i issue */
  //return;

  statment = peas_gtk_console_buffer_get_statment (console->priv->buffer);
  completions = peas_interpreter_complete (console->priv->interpreter,
                                           statment);

  /* Need to have completions be like:
   * 
   *    peas_gtk_console_buffer_write_completions (console->priv->buffer,
   *                                               completions);
   *
   * Reason: buffer need to set a custom tag for it that will cause it to
   *         wrap diffrently or make columns out of the possible
   *         completions which is MUCH easier for it then the console.
   */

  if (completions == NULL)
    return;
  
  whitespace = get_whitespace (console);
  new_statment = g_string_new (whitespace);
  g_free (whitespace);

  if (completions[1] == NULL)
    {
      g_string_append (new_statment, completions[0]);

      peas_gtk_console_buffer_set_statment (console->priv->buffer,
                                            new_statment->str);
    }
  else
    {
      gchar *completion;
      gchar *prefix;

      completion = g_strjoinv (" ", completions);
      prefix = get_common_string_prefix ((const gchar **) completions);

      peas_interpreter_write (console->priv->interpreter, "");
      peas_interpreter_write (console->priv->interpreter, completion);

      /* Need to force the buffer back into prompt mode */
      peas_gtk_console_buffer_write_prompt (console->priv->buffer);

      if (prefix != NULL)
        g_string_append (new_statment, prefix);

      peas_gtk_console_buffer_set_statment (console->priv->buffer,
                                            new_statment->str);

      g_free (prefix);
      g_free (completion);
    }

  g_string_free (new_statment, TRUE);
  g_strfreev (completions);
  g_free (statment);
}

static void
execute_caused_reset_cb (PeasInterpreter *interpreter,
                         gboolean        *reset_called)
{
  *reset_called = TRUE;
} 

static void
peas_gtk_console_newline (PeasGtkConsole *console)
{
  gchar *statment;
  gchar *whitespace;
  gulong reset_id;
  gboolean reset_called = FALSE;
  gboolean success;

  statment = peas_gtk_console_buffer_get_statment (console->priv->buffer);
  whitespace = get_whitespace (console);

  peas_gtk_console_history_add (console->priv->history, statment);

  reset_id = g_signal_connect (console->priv->interpreter,
                               "reset",
                               G_CALLBACK (execute_caused_reset_cb),
                               &reset_called);

  success = peas_interpreter_execute (console->priv->interpreter, statment);

  g_signal_handler_disconnect (console->priv->interpreter, reset_id);

  /* Don't cause a double prompt if reset was emitted */
  if (!reset_called)
    peas_gtk_console_buffer_write_prompt (console->priv->buffer);

  if (success && !reset_called)
    peas_gtk_console_buffer_set_statment (console->priv->buffer, whitespace);

  scroll_to_end (console);

  g_free (whitespace);
  g_free (statment);
}

static void
peas_gtk_console_reset (PeasGtkConsole *console)
{
  /* We emit the reset signal on the interpreter
   * so it and the console's buffer get reset properly
   */
  g_signal_emit_by_name (console->priv->interpreter, "reset");
}

static gboolean
key_press_event_cb (GtkWidget      *widget,
                    GdkEventKey    *event,
                    PeasGtkConsole *console)
{
  if (event == NULL)
    return FALSE;

  /* Would be nice to change it to a double: GDK_KEY_Tab, GDK_KEY_KP_Tab */
  if ((event->keyval == GDK_KEY_space || event->keyval == GDK_KEY_KP_Space) &&
      (event->state & GDK_CONTROL_MASK) != 0)
    {
      peas_gtk_console_complete (console);
      return TRUE;
    }

  if (event->keyval == GDK_KEY_Return ||
      event->keyval == GDK_KEY_KP_Enter ||
      event->keyval == GDK_KEY_ISO_Enter)
    {
      peas_gtk_console_newline (console);
      return TRUE;
    }

  if ((event->keyval == GDK_KEY_d) &&
      (event->state & GDK_CONTROL_MASK) != 0)
    {
      peas_gtk_console_reset (console);
      return TRUE;
    }

  return FALSE;
}

#if 0
static gboolean
key_press_event_cb (GtkWidget      *widget,
                    GdkEventKey    *event,
                    PeasGtkConsole *console)
{

  /* Allow moving out of the input when shift is in use */
  if ((event_state & GDK_SHIFT_MASK) == 0)
    {
      /* If the insert mark is somewhere earlier than the input mark
       * then set the cursor to the end of the buffer
       */

      if (compare_input (console) < 0)
        {
          GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (console->priv->buffer);
          GtkTextIter end;

          gtk_text_buffer_get_end_iter (text_buffer, &end);
          gtk_text_buffer_place_cursor (text_buffer, &end);
        }
    }


  /* instead hookup to the GtkTextView::select-all(view, gboolean select, data)? */
  else if (event->keyval == GDK_a && event_state == GDK_CONTROL_MASK)
    {
      GtkTextMark *insert;
      GtkTextIter iter;

      peas_gtk_console_buffer_get_input_iter (console->priv->buffer, &iter);

      gtk_text_buffer_place_cursor (text_buffer, &iter);

      insert = gtk_text_buffer_get_insert (text_buffer);

      gtk_text_buffer_get_end_iter (text_buffer, &iter);
      gtk_text_buffer_move_mark (text_buffer, insert, &iter);

      return TRUE;
    }

  return FALSE;
}
#endif

static void
peas_gtk_console_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      g_value_set_object (value,
                          peas_gtk_console_get_interpreter (console));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      console->priv->interpreter = g_object_ref (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_constructed (GObject *object)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);

  console->priv->buffer = peas_gtk_console_buffer_new (console->priv->interpreter);
  gtk_text_view_set_buffer (console->priv->text_view,
                            GTK_TEXT_BUFFER (console->priv->buffer));
}

static void
peas_gtk_console_dispose (GObject *object)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);

  g_clear_object (&console->priv->buffer);
  g_clear_object (&console->priv->interpreter);

  if (console->priv->history != NULL)
    {
      peas_gtk_console_history_unref (console->priv->history);
      console->priv->history = NULL;
    }

  if (console->priv->history_statement != NULL)
    {
      g_free (console->priv->history_statement);
      console->priv->history_statement = NULL;
    }

  g_clear_object (&console->priv->console_settings);
  g_clear_object (&console->priv->desktop_settings);
  g_clear_object (&console->priv->terminal_settings);

  if (console->priv->scroll_id != 0)
    {
      g_source_remove (console->priv->scroll_id);
      console->priv->scroll_id = 0;
    }

  G_OBJECT_CLASS (peas_gtk_console_parent_class)->dispose (object);
}

static void
peas_gtk_console_class_init (PeasGtkConsoleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = peas_gtk_console_get_property;
  object_class->set_property = peas_gtk_console_set_property;
  object_class->constructed = peas_gtk_console_constructed;
  object_class->dispose = peas_gtk_console_dispose;

  /**
   * PeasGtkConsole:interpreter:
   *
   * The #PeasInterpreter the console is attached to.
   */
  g_object_class_install_property (object_class,
                                   PROP_INTERPRETER,
                                   g_param_spec_object ("interpreter",
                                                        _("Interpreter"),
                                                        _("Interpreter object to "
                                                          "execute code with"),
                                                        PEAS_TYPE_INTERPRETER,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (PeasGtkConsolePrivate));
}

static void
peas_gtk_console_init (PeasGtkConsole *console)
{
  console->priv = G_TYPE_INSTANCE_GET_PRIVATE (console,
                                               PEAS_GTK_TYPE_CONSOLE,
                                               PeasGtkConsolePrivate);

  console->priv->history = peas_gtk_console_history_new ();
  console->priv->interpreter = NULL;
  console->priv->buffer = NULL;
  console->priv->scroll_id = 0;

  /*console->priv->console_settings = g_settings_new (CONSOLE_SCHEMA);*/
  console->priv->desktop_settings = settings_try_new (DESKTOP_SCHEMA);
  console->priv->terminal_settings = settings_try_new (TERMINAL_SCHEMA);

  /* Maybe we should use detail strings to only receive on specific changes */
  if (console->priv->desktop_settings != NULL)
    g_signal_connect (console->priv->desktop_settings,
                      "changed",
                      (GCallback) font_settings_changed_cb,
                      console);

  if (console->priv->terminal_settings != NULL)
    g_signal_connect (console->priv->terminal_settings,
                      "changed",
                      (GCallback) font_settings_changed_cb,
                      console);

  /* Required to not get an error */
  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (console), NULL);
  gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (console), NULL);

  /*gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (console),
                                         GTK_SHADOW_IN);*/
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (console),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  console->priv->text_view = GTK_TEXT_VIEW (gtk_text_view_new ());
  gtk_text_view_set_cursor_visible (console->priv->text_view, TRUE);

  gtk_widget_push_composite_child ();
  gtk_container_add (GTK_CONTAINER (console),
                     GTK_WIDGET (console->priv->text_view));
  gtk_widget_pop_composite_child ();

  /* load the font */
  font_settings_changed_cb (NULL, NULL, console);

  gtk_text_view_set_editable (console->priv->text_view, TRUE);
  gtk_text_view_set_wrap_mode (console->priv->text_view, GTK_WRAP_WORD_CHAR);
/*
  gtk_text_view_set_left_margin (console->priv->text_view, 2);
  gtk_text_view_set_right_margin (console->priv->text_view, 2);
  gtk_text_view_set_pixels_above_lines (console->priv->text_view, 2);
  gtk_text_view_set_pixels_below_lines (console->priv->text_view, 3);
*/

  g_signal_connect (console->priv->text_view,
                    "move-cursor",
                    G_CALLBACK (move_cursor_cb),
                    console);

  g_signal_connect_after (console->priv->text_view,
                          "move-cursor",
                          G_CALLBACK (move_cursor_after_cb),
                          console);

  g_signal_connect (console->priv->text_view,
                    "key-press-event",
                    G_CALLBACK (key_press_event_cb),
                    console);

  gtk_widget_show (GTK_WIDGET (console->priv->text_view));
}

/**
 * peas_gtk_console_new:
 * @interpreter: A #PeasInterpreter.
 *
 * Creates a new #PeasGtkConsole with @interpreter.
 *
 * Returns: the new #PeasGtkConsole object.
 */
GtkWidget *
peas_gtk_console_new (PeasInterpreter *interpreter)
{
  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), NULL);

  return GTK_WIDGET (g_object_new (PEAS_GTK_TYPE_CONSOLE,
                                   "interpreter", interpreter,
                                   NULL));
}

/**
 * peas_gtk_console_get_interpreter:
 * @console: a #PeasGtkConsole.
 *
 * Returns the #PeasInterpreter @console is attached to.
 *
 * Returns: (transfer none): the interpreter the console is attached to.
 */
PeasInterpreter *
peas_gtk_console_get_interpreter (PeasGtkConsole *console)
{
  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE (console), NULL);

  return console->priv->interpreter;
}
