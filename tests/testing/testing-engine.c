/*
 * testing-engine.c
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
#include <girepository.h>

#include "testing-engine.h"

static GLogFunc default_log_func;

static const gchar *allowed_patterns[] = {
  "*Plugin not found: does-not-exist*",
  "*libcloader.so*cannot open shared object file: No such file or directory*"
};

static void
log_handler (const gchar    *log_domain,
             GLogLevelFlags  log_level,
             const gchar    *message,
             gpointer        user_data)
{
  gint i;

  if ((log_level & G_LOG_LEVEL_DEBUG) != 0 ||
      (log_level & G_LOG_LEVEL_INFO) != 0 ||
      (log_level & G_LOG_LEVEL_MESSAGE) != 0)
    {
      default_log_func (log_domain, log_level, message, user_data);
      return;
    }

  if ((log_level & G_LOG_LEVEL_CRITICAL) != 0 ||
      (log_level & G_LOG_LEVEL_ERROR) != 0)
    {
      goto out;
    }

  for (i = 0; i < G_N_ELEMENTS (allowed_patterns); ++i)
    {
      if (g_pattern_match_simple (allowed_patterns[i], message))
        return;
    }

out:

  default_log_func (log_domain, log_level, message, user_data);

  /* Make sure we abort for warnings */
  abort ();
}

/**
 * testing_engine_get_default:
 *
 * Returns: (transfer none):
 */
PeasEngine *
testing_engine_get_default (void)
{
  PeasEngine *engine;
  GError *error = NULL;
  static gboolean initialized = FALSE;

  if (initialized)
    return peas_engine_get_default ();

  /* Don't always abort on warnings */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

  default_log_func = g_log_set_default_handler (log_handler, NULL);

  g_irepository_prepend_search_path (BUILD_DIR "/libpeas");
#ifdef ENABLE_GTK
  g_irepository_prepend_search_path (BUILD_DIR "/libpeas-gtk");
#endif

  g_setenv ("PEAS_PLUGIN_LOADERS_DIR", BUILD_DIR "/loaders", TRUE);

  g_irepository_require (g_irepository_get_default (), "Peas", "1.0", 0, &error);
  g_assert_no_error (error);

#ifdef ENABLE_GTK
  g_irepository_require (g_irepository_get_default (), "PeasGtk", "1.0", 0, &error);
  g_assert_no_error (error);
#endif

  g_irepository_require_private (g_irepository_get_default (),
                                 BUILD_DIR "/tests/testing",
                                 "Testing", "1.0", 0, &error);
  g_assert_no_error (error);


  /* Must be after requiring typelibs */
  engine = peas_engine_get_default ();

  peas_engine_add_search_path (engine, BUILD_DIR "/tests/plugins", NULL);
  peas_engine_rescan_plugins (engine);

  initialized = TRUE;

  return engine;
}
