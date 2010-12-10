/*
 * extension-set.c
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

/* TODO:
 *        - Check that extensions sets only contain extensions of their type
 */

typedef struct _ExtensionSetFixture ExtensionSetFixture;

struct _ExtensionSetFixture {
  PeasEngine *engine;
  PeasExtensionSet *extension_set;
  gint active;
};

/* Have dependencies before the plugin that needs them */
static const gchar *loadable_plugins[] = {
  "loadable", "has-dep", "self-dep"
};

static void
extension_added_cb (PeasExtensionSet *set,
                    PeasPluginInfo   *info,
                    PeasExtension    *exten,
                    gint             *active)
{
  ++(*active);
}

static void
extension_removed_cb (PeasExtensionSet *set,
                      PeasPluginInfo   *info,
                      PeasExtension    *exten,
                      gint             *active)
{
  --(*active);
}

static void
extension_set_setup (ExtensionSetFixture *fixture,
                     gconstpointer        data)
{
  fixture->engine = testing_engine_get_default ();

  fixture->extension_set = peas_extension_set_new (fixture->engine,
                                                   PEAS_TYPE_ACTIVATABLE,
                                                   "object", NULL,
                                                   NULL);

  g_signal_connect (fixture->extension_set,
                    "extension-added",
                    G_CALLBACK (extension_added_cb),
                    &fixture->active);
  g_signal_connect (fixture->extension_set,
                    "extension-removed",
                    G_CALLBACK (extension_removed_cb),
                    &fixture->active);   

  fixture->active = 0;
}

static void
extension_set_teardown (ExtensionSetFixture *fixture,
                        gconstpointer        data)
{
  g_object_unref (fixture->extension_set);
  g_assert_cmpint (fixture->active, ==, 0);


  /* This causes errors during the next test:
  g_object_unref (fixture->engine); */

  peas_engine_set_loaded_plugins (fixture->engine, NULL);
  g_assert (peas_engine_get_loaded_plugins (fixture->engine) == NULL);
}

static void
test_extension_set_no_extensions (ExtensionSetFixture *fixture,
                                  gconstpointer        data)
{
  /* Done in teardown */
}

static void
test_extension_set_activate (ExtensionSetFixture *fixture,
                             gconstpointer        data)
{
  gint i;
  PeasPluginInfo *info;

  for (i = 0; i < G_N_ELEMENTS (loadable_plugins); ++i)
    {
      g_assert_cmpint (fixture->active, ==, i);

      info = peas_engine_get_plugin_info (fixture->engine,
                                          loadable_plugins[i]);

      g_assert (info != NULL);
      g_assert (peas_engine_load_plugin (fixture->engine, info));
    }

  g_assert_cmpint (fixture->active, ==, G_N_ELEMENTS (loadable_plugins));
}

static void
test_extension_set_deactivate (ExtensionSetFixture *fixture,
                               gconstpointer        data)
{
  gint i;
  PeasPluginInfo *info;

  test_extension_set_activate (fixture, data);

  /* To keep deps in order */
  for (i = G_N_ELEMENTS (loadable_plugins); i > 0; --i)
    {
      g_assert_cmpint (fixture->active, ==, i);

      info = peas_engine_get_plugin_info (fixture->engine,
                                          loadable_plugins[i - 1]);

      g_assert (info != NULL);
      g_assert (peas_engine_unload_plugin (fixture->engine, info));
    }

  g_assert_cmpint (fixture->active, ==, 0);
}

static void
test_extension_set_get_extension (ExtensionSetFixture *fixture,
                                  gconstpointer        data)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (fixture->engine, loadable_plugins[0]);

  g_assert (info != NULL);
  g_assert (peas_extension_set_get_extension (fixture->extension_set, info) == NULL);
  g_assert (peas_engine_load_plugin (fixture->engine, info));

  extension = peas_extension_set_get_extension (fixture->extension_set, info);
  g_assert (extension != NULL);
  g_assert (PEAS_IS_ACTIVATABLE (extension));
}

static void
test_extension_set_call_valid (ExtensionSetFixture *fixture,
                               gconstpointer        data)
{
  test_extension_set_activate (fixture, data);

  g_assert (peas_extension_set_call (fixture->extension_set, "activate", NULL));
}

static void
test_extension_set_call_invalid (ExtensionSetFixture *fixture,
                                 gconstpointer        data)
{
  test_extension_set_activate (fixture, data);

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      peas_extension_set_call (fixture->extension_set, "invalid", NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*Method 'PeasActivatable.invalid' not found*");
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/extension-set/" path, ExtensionSetFixture, NULL, \
              extension_set_setup, (ftest), extension_set_teardown)

  TEST ("no-extensions", test_extension_set_no_extensions);
  TEST ("activate", test_extension_set_activate);
  TEST ("deactivate", test_extension_set_deactivate);

  TEST ("get-extension", test_extension_set_get_extension);

  TEST ("call-valid", test_extension_set_call_valid);
  TEST ("call-invalid", test_extension_set_call_invalid);

#undef TEST

  return g_test_run ();
}
