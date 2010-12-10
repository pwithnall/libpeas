/*
 * engine.c
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

#include <glib.h>
#include <libpeas/peas.h>

#include <testing.h>

typedef struct _EngineFixture EngineFixture;

struct _EngineFixture {
  PeasEngine *engine;
};

static void
engine_setup (EngineFixture *fixture,
              gconstpointer  data)
{
  fixture->engine = testing_engine_get_default ();
}

static void
engine_teardown (EngineFixture *fixture,
                 gconstpointer  data)
{
  /* This causes errors during the next test:
  g_object_unref (fixture->engine);*/

  peas_engine_set_loaded_plugins (fixture->engine, NULL);
  g_assert (peas_engine_get_loaded_plugins (fixture->engine) == NULL);
}

static void
test_runner (EngineFixture *fixture,
             gconstpointer  data)
{
  ((void (*) (PeasEngine *engine)) data) (fixture->engine);
}

static void
test_engine_get_default (PeasEngine *engine)
{
  g_assert (engine != NULL);
  g_assert (engine == testing_engine_get_default ());
}

static void
test_engine_load_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (info != NULL);
  g_assert (peas_engine_load_plugin (engine, info));
  g_assert (peas_plugin_info_is_loaded (info));
}

static void
test_engine_load_plugin_with_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "has-dep");

  g_assert (info != NULL);
  g_assert (peas_engine_load_plugin (engine, info));
  g_assert (peas_plugin_info_is_loaded (info));

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (info != NULL);
  g_assert (peas_plugin_info_is_loaded (info));
}

static void
test_engine_load_plugin_with_self_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "self-dep");

  g_assert (info != NULL);
  g_assert (peas_engine_load_plugin (engine, info));
  g_assert (peas_plugin_info_is_loaded (info));
}

static void
test_engine_unload_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "loadable");

  test_engine_load_plugin (engine);

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
}

static void
test_engine_unload_plugin_with_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "loadable");

  test_engine_load_plugin_with_dep (engine);

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));

  info = peas_engine_get_plugin_info (engine, "has-dep");

  g_assert (info != NULL);
  g_assert (!peas_plugin_info_is_loaded (info));
}

static void
test_engine_unload_plugin_with_self_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "self-dep");

  test_engine_load_plugin_with_self_dep (engine);

  g_assert (info != NULL);
  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
}

static void
test_engine_unavailable_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "unavailable");

  g_assert (info != NULL);
  g_assert (!peas_engine_load_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
  g_assert (!peas_plugin_info_is_available (info));
}

#ifdef TEST_DOES_NOT_FAIL
static void
test_engine_invalid_iage_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "invalid-iage");

  g_assert (info != NULL);
  g_assert (!peas_engine_load_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
  g_assert (!peas_plugin_info_is_available (info));
}
#endif

static void
notify_loaded_plugins_cb (PeasEngine   *engine,
                          GParamSpec   *pspec,
                          gchar      ***loaded_plugins)
{
  if (*loaded_plugins != NULL)
    g_strfreev (*loaded_plugins);

  *loaded_plugins = peas_engine_get_loaded_plugins (engine);
}

static void
test_engine_loaded_plugins (PeasEngine *engine)
{
  PeasPluginInfo *info;
  gchar **loaded_plugins = NULL;

  g_signal_connect (engine,
                    "notify::loaded-plugins",
                    G_CALLBACK (notify_loaded_plugins_cb),
                    (gpointer) &loaded_plugins);

  info = peas_engine_get_plugin_info (engine, "loadable");


  g_assert (info != NULL);
  g_assert (peas_engine_load_plugin (engine, info));

  g_assert (loaded_plugins != NULL);
  g_assert_cmpstr (loaded_plugins[0], ==, "loadable");
  g_assert (loaded_plugins[1] == NULL);

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (loaded_plugins == NULL);


  /* Cannot be done as unrefing the engine causes
     issues when another test is run

  g_assert (peas_engine_load_plugin (engine, info));

  g_assert (loaded_plugins != NULL);
  g_assert_cmpstr (loaded_plugins[0], ==, "loadable");
  g_assert (loaded_plugins[1] == NULL);

  g_object_unref (engine);

  g_assert (loaded_plugins != NULL);
  g_assert_cmpstr (loaded_plugins[0], ==, "loadable");
  g_assert (loaded_plugins[1] == NULL);*/

  if (loaded_plugins != NULL)
    g_strfreev (loaded_plugins);
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/engine/" path, EngineFixture, (gpointer) (ftest), \
              engine_setup, test_runner, engine_teardown)

  TEST ("get-default", test_engine_get_default);

  TEST ("load-plugin", test_engine_load_plugin);
  TEST ("load-plugin-with-dep", test_engine_load_plugin_with_dep);
  TEST ("load-plugin-with-self-dep", test_engine_load_plugin_with_self_dep);

  TEST ("unload-plugin", test_engine_unload_plugin);
  TEST ("unload-plugin-with-dep", test_engine_unload_plugin_with_dep);
  TEST ("unload-plugin-with-self-dep", test_engine_unload_plugin_with_self_dep);

  TEST ("unavailable-plugin", test_engine_unavailable_plugin);
#ifdef TEST_DOES_NOT_FAIL
  TEST ("invalid-iage", test_engine_invalid_iage_plugin);
#endif

  TEST ("loaded-plugins", test_engine_loaded_plugins);

#undef TEST

  return g_test_run ();
}
