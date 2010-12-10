/*
 * extension.c
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

#include <stdlib.h>

#include <glib.h>
#include <libpeas/peas.h>

#include "testing.h"

typedef struct _ExtensionFixture ExtensionFixture;

struct _ExtensionFixture {
  PeasEngine *engine;
};

static void
extension_setup (ExtensionFixture *fixture,
                 gconstpointer     data)
{
  fixture->engine = testing_engine_get_default ();
}

static void
extension_teardown (ExtensionFixture *fixture,
                    gconstpointer     data)
{
  /* This causes errors during the next test:
  g_object_unref (fixture->engine);*/

  peas_engine_set_loaded_plugins (fixture->engine, NULL);
  g_assert (peas_engine_get_loaded_plugins (fixture->engine) == NULL);
}

static void
test_runner (ExtensionFixture *fixture,
             gconstpointer     data)
{
  ((void (*) (PeasEngine *engine)) data) (fixture->engine);
}

static void
test_extension_create_valid (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (info != NULL);
  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            PEAS_TYPE_ACTIVATABLE,
                                            "object", NULL,
                                            NULL);

  g_assert (PEAS_IS_EXTENSION (extension));
  g_assert (PEAS_IS_ACTIVATABLE (extension));
}

static void
test_extension_create_invalid (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (info != NULL);

  /* Not loaded */
  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      extension = peas_engine_create_extension (engine, info,
                                                PEAS_TYPE_ACTIVATABLE,
                                                "object", NULL,
                                                NULL);
      exit (0);
    }
  /* Resident modules cause this to fail?
  g_test_trap_assert_failed ();*/

  peas_engine_load_plugin (engine, info);

  /* Invalid GType */
  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      extension = peas_engine_create_extension (engine, info,
                                                G_TYPE_INVALID,
                                                NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*CRITICAL*");

  /* GObject but not a GInterface */
  extension = peas_engine_create_extension (engine, info,
                                            PEAS_TYPE_ENGINE,
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));

  /* Does not implement this GType */
  extension = peas_engine_create_extension (engine, info,
                                            TESTING_TYPE_UNIMPLEMENTABLE,
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));
}

static void
test_extension_call_valid (PeasEngine *engine)
{
}

static void
test_extension_call_invalid (PeasEngine *engine)
{
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/extension/" path, ExtensionFixture, (ftest), \
              extension_setup, test_runner, extension_teardown)

  TEST ("create-valid", test_extension_create_valid);
  TEST ("create-invalid", test_extension_create_invalid);

  /*TEST ("call-no-args-valid", test_extension_call_valid);
  TEST ("call-no-args-invalid", test_extension_call_invalid);

  TEST ("call-single-arg-valid", test_extension_call_valid);
  TEST ("call-single-arg-invalid", test_extension_call_invalid);

  TEST ("call-multi-args-valid", test_extension_call_valid);
  TEST ("call-multi-args-invalid", test_extension_call_invalid);*/

#undef TEST

  return g_test_run ();
}
