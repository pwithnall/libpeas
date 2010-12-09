#include <string.h>

#include <glib.h>
#include <gdk/gdkkeysyms.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "testing.h"

typedef struct _ConsoleFixture ConsoleFixture;

struct _ConsoleFixture {
  PeasInterpreter *interpreter;
  PeasGtkConsole *console;
  GtkWidget *view;
  GtkTextBuffer *buffer;
};

static void
console_setup (ConsoleFixture *fixture,
               gconstpointer   data)
{
  GtkWidget *console;

  fixture->interpreter = testing_interpreter_new ();
  g_assert (TESTING_IS_INTERPRETER (fixture->interpreter));

  console = gtk_test_create_widget (PEAS_GTK_TYPE_CONSOLE,
                                    "interpreter", fixture->interpreter,
                                    NULL);
  fixture->console = PEAS_GTK_CONSOLE (console);                        
  g_assert (PEAS_GTK_IS_CONSOLE (fixture->console));

  fixture->view = gtk_bin_get_child (GTK_BIN (fixture->console));
  g_assert (GTK_IS_TEXT_VIEW (fixture->view));

  fixture->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (fixture->view));
  g_assert (GTK_IS_TEXT_BUFFER (fixture->buffer));
}

static void
console_teardown (ConsoleFixture *fixture,
                  gconstpointer   data)
{
  g_object_unref (fixture->interpreter);
}


/* We specifically do not activate the keybindings by sending
 * keys to the widget because it would mean the window would
 * need to be shown and thus a "ghost" window would be present.
 */
static void
send_key (GtkWidget       *widget,
          guint            keyval,
          GdkModifierType  modifiers)
{
  if (!gtk_bindings_activate (G_OBJECT (widget), keyval, modifiers))
    {
      GdkEvent event;
      gboolean handled;

      memset(&event, 0, sizeof (event));
      event.key.keyval = keyval;
      event.key.state = modifiers;
      event.key.window = gtk_widget_get_window (widget);

      /* Assert only for key-press-event because
       * that is where they should be handled
       */

      event.key.type = GDK_KEY_PRESS;
      g_signal_emit_by_name (widget, "key-press-event", &event.key, &handled);
      g_assert (handled);

      event.key.type = GDK_KEY_RELEASE;
      g_signal_emit_by_name (widget, "key-release-event", &event.key, &handled);
    }
}

static void
cycle_keys (ConsoleFixture  *fixture,
            GdkModifierType  modifiers,
            guint           *last_key,
            guint            n_keys,
            ...)
{
  va_list args;
  guint *keys;
  gint i;

  keys = g_newa (guint, n_keys);

  va_start (args, n_keys);

  for (i = 0; i < n_keys; ++i)
    keys[i] = va_arg (args, guint);

  va_end (args);

  if (*last_key >= n_keys)
    *last_key = 0;

  send_key (fixture->view, keys[*last_key], modifiers);

  ++(*last_key);
}

#define BINDING_CYCLE_2(fixture, key_1, key_2) \
  static guint last_key = 0; \
\
  cycle_keys (fixture, 0, &last_key, 2, key_1, key_2)

static void
console_move_up (ConsoleFixture *fixture)
{
  BINDING_CYCLE_2 (fixture, GDK_KEY_Up, GDK_KEY_KP_Up);
}

static void
console_move_down (ConsoleFixture *fixture)
{
  BINDING_CYCLE_2 (fixture, GDK_KEY_Down, GDK_KEY_KP_Down);
}

static void
console_move_left (ConsoleFixture *fixture)
{
  BINDING_CYCLE_2 (fixture, GDK_KEY_Left, GDK_KEY_KP_Left);
}

static void
console_move_right (ConsoleFixture *fixture)
{
  BINDING_CYCLE_2 (fixture, GDK_KEY_Right, GDK_KEY_KP_Right);
}

static void
console_move_home (ConsoleFixture *fixture)
{
  BINDING_CYCLE_2 (fixture, GDK_KEY_Home, GDK_KEY_KP_Home);
}

static void
console_move_end (ConsoleFixture *fixture)
{
  BINDING_CYCLE_2 (fixture, GDK_KEY_End, GDK_KEY_KP_End);
}

static void
console_select_left (ConsoleFixture *fixture,
                     guint           n)
{
  guint i;

  for (i = 0; i < n; ++i)
    send_key (fixture->view, GDK_KEY_Left, GDK_SHIFT_MASK);
}

static void
console_select_right (ConsoleFixture *fixture,
                      guint           n)
{
  guint i;

  for (i = 0; i < n; ++i)
    send_key (fixture->view, GDK_KEY_Right, GDK_SHIFT_MASK);
}

static void
console_backspace (ConsoleFixture *fixture)
{
  send_key (fixture->view, GDK_KEY_BackSpace, 0);
}

static void
console_complete (ConsoleFixture *fixture)
{
  static guint last_key = 0;

  cycle_keys (fixture, GDK_CONTROL_MASK, &last_key, 2,
              GDK_KEY_space, GDK_KEY_KP_Space);
}

static void
console_newline (ConsoleFixture *fixture)
{
  static guint last_key = 0;

  cycle_keys (fixture, 0, &last_key, 3,
              GDK_KEY_Return, GDK_KEY_KP_Enter, GDK_KEY_ISO_Enter);
}

static void
console_reset (ConsoleFixture *fixture)
{
  send_key (fixture->view, GDK_KEY_d, GDK_CONTROL_MASK);
}

static void
console_write (ConsoleFixture *fixture,
               const gchar    *text)
{
  GtkTextIter insert;
  GtkTextMark *insert_mark;

  insert_mark = gtk_text_buffer_get_insert (fixture->buffer);
  gtk_text_buffer_get_iter_at_mark (fixture->buffer, &insert, insert_mark);

  gtk_text_buffer_insert (fixture->buffer, &insert, text, -1);

  gtk_text_buffer_move_mark (fixture->buffer,
                             insert_mark,
                             &insert);
}

static void
console_execute (ConsoleFixture *fixture,
                 const gchar    *code)
{
  console_write (fixture, code);
  console_newline (fixture);
}

static void
console_set_line_pos (ConsoleFixture *fixture,
                      guint           pos)
{
  GtkTextIter iter;

  gtk_text_buffer_get_end_iter (fixture->buffer, &iter);

  gtk_text_iter_set_line_offset (&iter, pos);

  gtk_text_buffer_place_cursor (fixture->buffer, &iter);
}

static gboolean
console_get_input_iter (ConsoleFixture *fixture,
                        GtkTextIter    *iter)
{
  GtkTextIter start, input;
  gchar *text;

  gtk_text_buffer_get_end_iter (fixture->buffer, &start);
  gtk_text_iter_set_line_offset (&start, 0);

  input = start;
  gtk_text_iter_set_line_offset (&input, strlen (">>> "));

  text = gtk_text_buffer_get_text (fixture->buffer, &start, &input, FALSE);

  if (g_strcmp0 (text, ">>> ") != 0 && g_strcmp0 (text, "... ") != 0)
    return FALSE;

  *iter = input;

  return TRUE;
}

/* Returns the statment text with a | where the cursor is */
static gchar *
console_get_statment_repr (ConsoleFixture *fixture)
{
  GtkTextIter input, insert, end;
  GtkTextMark *insert_mark;
  gchar *statment;
  gint cursor_offset;
  GString *statment_repr;

  g_assert (console_get_input_iter (fixture, &input));

  insert_mark = gtk_text_buffer_get_insert (fixture->buffer);
  gtk_text_buffer_get_iter_at_mark (fixture->buffer, &insert, insert_mark);

  gtk_text_buffer_get_end_iter (fixture->buffer, &end);

  statment = gtk_text_buffer_get_text (fixture->buffer,
                                       &input, &end, FALSE);

  cursor_offset = gtk_text_iter_get_line_offset (&insert) -
                  gtk_text_iter_get_line_offset (&input);

  statment_repr = g_string_new (statment);
  g_string_insert_c (statment_repr, cursor_offset, '|');

  g_free (statment);

  return g_string_free (statment_repr, FALSE);
}

/* Implemented as macros so we get correct line numbers */
#define assert_console_text(fixture, expected) \
  G_STMT_START \
    { \
      GtkTextIter start, end; \
      gchar *text; \
\
      gtk_text_buffer_get_bounds (fixture->buffer, &start, &end); \
\
      text = gtk_text_buffer_get_text (fixture->buffer, &start, &end, TRUE); \
\
      g_assert_cmpstr (text, ==, expected); \
\
      g_free (text); \
    } \
  G_STMT_END

#define assert_console_cursor_pos(fixture, expected) \
  G_STMT_START \
    { \
      gchar *statment_repr; \
\
      statment_repr = console_get_statment_repr (fixture); \
\
      g_assert_cmpstr (statment_repr, ==, expected); \
\
      g_free (statment_repr); \
    } \
  G_STMT_END

#define assert_console_selection(fixture, expected) \
  G_STMT_START \
    { \
      GtkTextIter start, end; \
      gboolean has_selection; \
\
      has_selection = gtk_text_buffer_get_selection_bounds (fixture->buffer, \
                                                            &start, &end); \
\
      if (expected == NULL) \
        g_assert (!has_selection); \
      else \
        { \
          gchar *text; \
\
          g_assert (has_selection); \
\
          text = gtk_text_buffer_get_text (fixture->buffer, &start, &end, TRUE); \
\
          g_assert_cmpstr (text, ==, expected); \
\
          g_free (text); \
      } \
    } \
  G_STMT_END

#define assert_interpreter_code(fixture, expected) \
  G_STMT_START \
    { \
      const gchar *code; \
\
      code = testing_interpreter_get_code (TESTING_INTERPRETER (fixture->interpreter)); \
\
      g_assert_cmpstr (code, ==, expected); \
    } \
  G_STMT_END


#define S show_console (fixture);
static gboolean
delete_event_cb (GtkWidget      *window,
                 GdkEvent       *event,
                 ConsoleFixture *fixture)
{
  gtk_main_quit ();

  return TRUE;
}

static gboolean
showing_console_cause_critical_error (void)
{
  return TRUE;
}

static void
show_console (ConsoleFixture *fixture)
{
  GtkWidget *window;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 100);

  gtk_container_add (GTK_CONTAINER (window),
                     GTK_WIDGET (fixture->console));

  gtk_widget_show_all (window);

  g_signal_connect (window,
                    "delete-event",
                    G_CALLBACK (delete_event_cb),
                    fixture);

  gtk_main ();

  g_assert (!showing_console_cause_critical_error ());
}


static void
test_console_get_interpreter (ConsoleFixture *fixture,
                              gconstpointer   data)
{
  PeasInterpreter *interpreter;

  interpreter = peas_gtk_console_get_interpreter (fixture->console);
  g_assert (interpreter == fixture->interpreter);


  g_object_get (G_OBJECT (fixture->console),
                "interpreter", &interpreter,
                NULL);

  g_assert (interpreter == fixture->interpreter);
}

static void
test_console_prompt_nonblock (ConsoleFixture *fixture,
                              gconstpointer   data)
{
  assert_console_text (fixture, ">>> ");
}

static void
test_console_prompt_block (ConsoleFixture *fixture,
                           gconstpointer   data)
{
  console_execute (fixture, "block");

  assert_console_text (fixture, ">>> block\n... ");
}

static void
test_console_keybindings_execute (ConsoleFixture *fixture,
                                  gconstpointer   data)
{
  console_write (fixture, "a");
  console_newline (fixture);

  console_write (fixture, "b");
  console_newline (fixture);

  console_write (fixture, "c");
  console_newline (fixture);

  assert_console_text (fixture, ">>> a\n>>> b\n>>> c\n>>> ");
  assert_interpreter_code (fixture, "a\nb\nc");
}

static void
test_console_keybindings_backspace (ConsoleFixture *fixture,
                                    gconstpointer   data)
{
  console_write (fixture, "a");

  console_backspace (fixture);
  console_backspace (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> \n>>> ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_keybindings_left (ConsoleFixture *fixture,
                               gconstpointer   data)
{
  console_move_left (fixture);

  console_write (fixture, "a");

  console_move_left (fixture);

  console_write (fixture, "b");

  console_newline (fixture);

  assert_console_text (fixture, ">>> ba\n>>> ");
  assert_interpreter_code (fixture, "ba");
}

static void
test_console_keybindings_right (ConsoleFixture *fixture,
                               gconstpointer   data)
{
  /* I know this is dumb but is has happend to me before */

  console_move_right (fixture);

  console_write (fixture, "a");

  console_move_left (fixture);
  console_move_right (fixture);

  console_write (fixture, "b");

  console_newline (fixture);

  assert_console_text (fixture, ">>> ab\n>>> ");
  assert_interpreter_code (fixture, "ab");
}

static void
test_console_keybindings_reset (ConsoleFixture *fixture,
                                gconstpointer   data)
{
  console_execute (fixture, "  the reset will delete this");

  console_reset (fixture);

  assert_console_text (fixture, ">>> ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_keybindings_complete_full (ConsoleFixture *fixture,
                                        gconstpointer   data)
{
  console_write (fixture, "complete full");

  console_complete (fixture);

  /* should full completion be: ">>> completed full "? like bash */
  assert_console_text (fixture, ">>> completed full");
  assert_interpreter_code (fixture, "");


  console_reset (fixture);


  console_write (fixture, "  complete full");

  console_complete (fixture);

  assert_console_text (fixture, ">>>   completed full");
  assert_interpreter_code (fixture, "");
}

static void
test_console_keybindings_complete_has_prefix (ConsoleFixture *fixture,
                                              gconstpointer   data)
{
  console_write (fixture, "complete partial has-prefix");

  console_complete (fixture);

  assert_console_text (fixture, ">>> complete partial has-prefix\n"
                                "pre_a_blah pre_b pre_c_blah\n"
                                ">>> pre_");
  assert_interpreter_code (fixture, "");


  console_reset (fixture);


  console_write (fixture, "  complete partial has-prefix");

  console_complete (fixture);

  assert_console_text (fixture, ">>>   complete partial has-prefix\n"
                                "pre_a_blah pre_b pre_c_blah\n"
                                ">>>   pre_");
  assert_interpreter_code (fixture, "");
}

static void
test_console_keybindings_complete_no_prefix (ConsoleFixture *fixture,
                                             gconstpointer   data)
{
  console_write (fixture, "complete partial no-prefix");

  console_complete (fixture);

  assert_console_text (fixture, ">>> complete partial no-prefix\n"
                                "a_blah b c_blah\n"
                                ">>> ");
  assert_interpreter_code (fixture, "");


  console_reset (fixture);


  console_write (fixture, "  complete partial no-prefix");

  console_complete (fixture);

  assert_console_text (fixture, ">>>   complete partial no-prefix\n"
                                "a_blah b c_blah\n"
                                ">>>   ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_keybindings_complete_nothing (ConsoleFixture *fixture,
                                           gconstpointer   data)
{
  console_write (fixture, "complete nothing");

  console_complete (fixture);

  assert_console_text (fixture, ">>> complete nothing");
  assert_interpreter_code (fixture, "");
}

static void
test_console_keybindings_home_nonsmart (ConsoleFixture *fixture,
                                        gconstpointer   data)
{
  console_write (fixture, "a statement");

  console_move_home (fixture);
  assert_console_cursor_pos (fixture, "|a statement");

  console_newline (fixture);

  console_write (fixture, "a statement");

  console_move_home (fixture);
  assert_console_cursor_pos (fixture, "|a statement");
}

static void
test_console_keybindings_home_smart (ConsoleFixture *fixture,
                                     gconstpointer   data)
{
  console_write (fixture, "  I have whitespace");

  console_move_home (fixture);
  assert_console_cursor_pos (fixture, "  |I have whitespace");

  console_move_home (fixture);
  assert_console_cursor_pos (fixture, "|  I have whitespace");

  console_move_home (fixture);
  assert_console_cursor_pos (fixture, "  |I have whitespace");
}

static void
test_console_keybindings_end_nonsmart (ConsoleFixture *fixture,
                                       gconstpointer   data)
{
  console_write (fixture, "a statement");

  console_move_end (fixture);
  assert_console_cursor_pos (fixture, "a statement|");

  console_newline (fixture);

  console_write (fixture, "a statement");

  console_move_end (fixture);
  assert_console_cursor_pos (fixture, "a statement|");
}

static void
test_console_keybindings_end_smart (ConsoleFixture *fixture,
                                    gconstpointer   data)
{
  console_write (fixture, "I have whitespace  ");

  console_move_end (fixture);
  assert_console_cursor_pos (fixture, "I have whitespace|  ");

  console_move_end (fixture);
  assert_console_cursor_pos (fixture, "I have whitespace  |");

  console_move_end (fixture);
  assert_console_cursor_pos (fixture, "I have whitespace|  ");
}

static void
test_console_selection_inside_input (ConsoleFixture *fixture,
                                     gconstpointer   data)
{
  console_write (fixture, "blah");

  console_select_left (fixture, strlen ("blah"));

  assert_console_selection (fixture, "blah");
}

static void
test_console_selection_outside_input (ConsoleFixture *fixture,
                                      gconstpointer   data)
{
  console_write (fixture, "blah");

  console_set_line_pos (fixture, 0);
  console_select_right (fixture, strlen (">>> blah"));
  assert_console_selection (fixture, ">>> blah");

  console_select_left (fixture, 1);
  assert_console_selection (fixture, ">>> bla");

  console_select_left (fixture, strlen (">>> blah"));
  assert_console_selection (fixture, NULL);
}

static void
test_console_execute_valid (ConsoleFixture *fixture,
                            gconstpointer   data)
{
  console_execute (fixture, "a valid statement");

  assert_console_text (fixture, ">>> a valid statement\n>>> ");
  assert_interpreter_code (fixture, "a valid statement");
}

static void
test_console_execute_invalid (ConsoleFixture *fixture,
                              gconstpointer   data)
{
  console_execute (fixture, "invalid");

  assert_console_text (fixture, ">>> invalid\n>>> ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_execute_reset (ConsoleFixture *fixture,
                            gconstpointer   data)
{
  console_execute (fixture, "blah blah blah");
  console_execute (fixture, "reset");

  assert_console_text (fixture, ">>> ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_execute_write (ConsoleFixture *fixture,
                            gconstpointer   data)
{
  console_execute (fixture, "print Hello, World!");

  assert_console_text (fixture, ">>> print Hello, World!\nHello, World!\n>>> ");
  assert_interpreter_code (fixture, "print Hello, World!");
}

static void
test_console_execute_write_newline (ConsoleFixture *fixture,
                                    gconstpointer   data)
{
  console_execute (fixture, "println Hello, World!");

  assert_console_text (fixture, ">>> println Hello, World!\nHello, World!\n\n>>> ");
  assert_interpreter_code (fixture, "println Hello, World!");
}

static void
test_console_execute_write_error (ConsoleFixture *fixture,
                                  gconstpointer   data)
{
  console_execute (fixture, "error Error Message");

  assert_console_text (fixture, ">>> error Error Message\n"
                                "Error ****: Error Message\n>>> ");
  assert_interpreter_code (fixture, "error Error Message");
}

static void
test_console_execute_with_whitespace_valid (ConsoleFixture *fixture,
                                            gconstpointer   data)
{
  console_execute (fixture, "  I have whitespace");

  assert_console_text (fixture, ">>>   I have whitespace\n>>>   ");
  assert_interpreter_code (fixture, "  I have whitespace");
}

static void
test_console_execute_with_whitespace_invalid (ConsoleFixture *fixture,
                                              gconstpointer   data)
{
  console_execute (fixture, "  invalid I have whitespace");

  assert_console_text (fixture, ">>>   invalid I have whitespace\n>>> ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_execute_with_whitespace_reset (ConsoleFixture *fixture,
                                            gconstpointer   data)
{
  console_execute (fixture, "  blah blah blah");
  console_execute (fixture, "  reset");

  assert_console_text (fixture, ">>> ");
  assert_interpreter_code (fixture, "");
}

static void
test_console_history_up_empty (ConsoleFixture *fixture,
                               gconstpointer   data)
{
  console_write (fixture, "a statement");

  console_move_up (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> ");
  assert_interpreter_code (fixture, "a statement");
}

static void
test_console_history_up_nonempty (ConsoleFixture *fixture,
                                  gconstpointer   data)
{
  console_execute (fixture, "a statement");

  console_move_up (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> a statement\n>>> ");
  assert_interpreter_code (fixture, "a statement\na statement");
}

static void
test_console_history_up_past_top (ConsoleFixture *fixture,
                                  gconstpointer   data)
{
  console_execute (fixture, "a statement");

  console_move_up (fixture);
  console_move_up (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> a statement\n>>> ");
  assert_interpreter_code (fixture, "a statement\na statement");
}

static void
test_console_history_down_empty (ConsoleFixture *fixture,
                                 gconstpointer   data)
{
  console_write (fixture, "a statement");

  console_move_down (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> ");
  assert_interpreter_code (fixture, "a statement");
}

static void
test_console_history_down_nonempty (ConsoleFixture *fixture,
                           gconstpointer   data)
{
  console_execute (fixture, "a statement");
  console_execute (fixture, "another statement");

  console_move_up (fixture);
  console_move_up (fixture);
  console_move_down (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> another statement\n"
                                ">>> another statement\n>>> ");
  assert_interpreter_code (fixture, "a statement\nanother statement\n"
                                    "another statement");
}

static void
test_console_history_restore_empty (ConsoleFixture *fixture,
                                    gconstpointer   data)
{
  console_write (fixture, "a statement");

  console_move_up (fixture);
  console_move_down (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> ");
  assert_interpreter_code (fixture, "a statement");
}

static void
test_console_history_restore_nonempty (ConsoleFixture *fixture,
                                       gconstpointer   data)
{
  console_execute (fixture, "a statement");

  console_write (fixture, "restored statement");

  console_move_up (fixture);
  console_move_down (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> restored statement\n>>> ");
  assert_interpreter_code (fixture, "a statement\nrestored statement");
}

static void
test_console_history_restore_past_end (ConsoleFixture *fixture,
                                       gconstpointer   data)
{
  console_execute (fixture, "a statement");

  console_write (fixture, "restored statement");

  console_move_up (fixture);
  console_move_down (fixture);
  console_move_down (fixture);
  console_newline (fixture);

  assert_console_text (fixture, ">>> a statement\n>>> restored statement\n>>> ");
  assert_interpreter_code (fixture, "a statement\nrestored statement");
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);

#define TEST(path, ftest) \
  g_test_add ("/console/" path, ConsoleFixture, NULL, \
              console_setup, (ftest), console_teardown)

  TEST ("get/interpreter", test_console_get_interpreter);

  TEST ("prompt/nonblock", test_console_prompt_nonblock);
  TEST ("prompt/block", test_console_prompt_block);

  TEST ("keybindings/execute", test_console_keybindings_execute);
  TEST ("keybindings/backspace", test_console_keybindings_backspace);
  TEST ("keybindings/left", test_console_keybindings_left);
  TEST ("keybindings/right", test_console_keybindings_right);
  TEST ("keybindings/reset", test_console_keybindings_reset);
  TEST ("keybindings/complete/full", test_console_keybindings_complete_full);
  TEST ("keybindings/complete/has-prefix", test_console_keybindings_complete_has_prefix);
  TEST ("keybindings/complete/no-prefix", test_console_keybindings_complete_no_prefix);
  TEST ("keybindings/complete/nothing", test_console_keybindings_complete_nothing);
  TEST ("keybindings/home/nonsmart", test_console_keybindings_home_nonsmart);
  TEST ("keybindings/home/smart", test_console_keybindings_home_smart);
  TEST ("keybindings/end/nonsmart", test_console_keybindings_end_nonsmart);
  TEST ("keybindings/end/smart", test_console_keybindings_end_smart);

  TEST ("selection/from-input", test_console_selection_inside_input);
  TEST ("selection/outside-input", test_console_selection_outside_input);

  TEST ("execute/valid", test_console_execute_valid);
  TEST ("execute/invalid", test_console_execute_invalid);
  TEST ("execute/reset", test_console_execute_reset);
  TEST ("execute/write", test_console_execute_write);
  TEST ("execute/write-newline", test_console_execute_write_newline);
  TEST ("execute/write-error", test_console_execute_write_error);
  TEST ("execute/with-whitespace/valid", test_console_execute_with_whitespace_valid);
  TEST ("execute/with-whitespace/invalid", test_console_execute_with_whitespace_invalid);
  TEST ("execute/with-whitespace/reset", test_console_execute_with_whitespace_reset);

  TEST ("history/up/empty", test_console_history_up_empty);
  TEST ("history/up/nonempty", test_console_history_up_nonempty);
  TEST ("history/up/past-top", test_console_history_up_past_top);
  TEST ("history/down/empty", test_console_history_down_empty);
  TEST ("history/down/nonempty", test_console_history_down_nonempty);
  TEST ("history/restore/empty", test_console_history_restore_empty);
  TEST ("history/restore/nonempty", test_console_history_restore_nonempty);
  TEST ("history/restore/past-end", test_console_history_restore_past_end);

#undef TEST

  return g_test_run ();
}
