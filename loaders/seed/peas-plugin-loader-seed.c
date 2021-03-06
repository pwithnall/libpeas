/*
 * peas-plugin-loader-seed.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009 - Steve Frécinaux
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

#include <seed.h>
/* FIXME: Seed Bug 624560 */
SeedValue seed_value_from_gvalue (SeedContext    ctx,
                                  GValue        *gval,
                                  SeedException *exception);

#include "peas-plugin-loader-seed.h"
#include "peas-extension-seed.h"

G_DEFINE_TYPE (PeasPluginLoaderSeed, peas_plugin_loader_seed, PEAS_TYPE_PLUGIN_LOADER);

typedef struct {
  SeedContext context;
  SeedObject extensions;
} SeedInfo;

static SeedEngine *seed = NULL;

static void
peas_plugin_loader_seed_add_module_directory (PeasPluginLoader *loader,
                                              const gchar      *module_dir)
{
  gchar **sp = seed_engine_get_search_path (seed);

  if (sp)
    {
      gchar *orig_sp = g_strjoinv (":", sp);
      gchar *new_sp = g_strconcat (module_dir, ":", orig_sp, NULL);

      seed_engine_set_search_path (seed, new_sp);

      g_free (new_sp);
      g_free (orig_sp);
    }
  else
    {
      seed_engine_set_search_path (seed, module_dir);
    }
}

static gchar *
get_script_for_plugin_info (PeasPluginInfo   *info,
                            SeedContext       context)
{
  gchar *basename;
  gchar *filename;
  gchar *script = NULL;

  basename = g_strconcat (peas_plugin_info_get_module_name (info), ".js", NULL);
  filename = g_build_filename (peas_plugin_info_get_module_dir (info), basename, NULL);

  g_debug ("Seed script filename is '%s'", filename);

  g_file_get_contents (filename, &script, NULL, NULL);

  g_free (basename);
  g_free (filename);

  return script;
}

static gboolean
peas_plugin_loader_seed_load (PeasPluginLoader *loader,
                              PeasPluginInfo   *info)
{
  PeasPluginLoaderSeed *sloader = PEAS_PLUGIN_LOADER_SEED (loader);
  SeedContext context;
  gchar *script;
  SeedException exc = NULL;
  SeedObject global, extensions;
  SeedInfo *sinfo;

  context = seed_context_create (seed->group, NULL);

  seed_prepare_global_context (context);
  script = get_script_for_plugin_info (info, context);

  seed_simple_evaluate (context, script, &exc);
  g_free (script);

  if (exc)
    {
      gchar *exc_string = seed_exception_to_string (context, exc);
      g_warning ("Seed Exception: %s", exc_string);
      g_free (exc_string);
      seed_context_unref (context);
      return FALSE;
    }
  else
    {
      global = seed_context_get_global_object (context);
      extensions = seed_object_get_property (context, global, "extensions");

      if (seed_value_is_object (context, extensions))
        {
          sinfo = (SeedInfo *) g_slice_new (SeedInfo);
          sinfo->context = context;
          sinfo->extensions = extensions;
          seed_context_ref (context);
          seed_value_protect (context, extensions);

          g_hash_table_insert (sloader->loaded_plugins, info, sinfo);
        }
    }

  seed_context_unref (context);

  return TRUE;
}

static gboolean
peas_plugin_loader_seed_provides_extension  (PeasPluginLoader *loader,
                                             PeasPluginInfo   *info,
                                             GType             exten_type)
{
  PeasPluginLoaderSeed *sloader = PEAS_PLUGIN_LOADER_SEED (loader);
  SeedInfo *sinfo;
  SeedValue extension;

  sinfo = (SeedInfo *) g_hash_table_lookup (sloader->loaded_plugins, info);
  if (!sinfo)
    return FALSE;

  extension = seed_object_get_property (sinfo->context,
                                        sinfo->extensions,
                                        g_type_name (exten_type));
  return extension && seed_value_is_object (sinfo->context, extension);
}

static PeasExtension *
peas_plugin_loader_seed_create_extension (PeasPluginLoader *loader,
                                          PeasPluginInfo   *info,
                                          GType             exten_type,
                                          guint             n_parameters,
                                          GParameter       *parameters)
{
  PeasPluginLoaderSeed *sloader = PEAS_PLUGIN_LOADER_SEED (loader);
  SeedInfo *sinfo;
  SeedValue extension_methods, extension;
  gchar **property_names;
  guint i, j;

  sinfo = (SeedInfo *) g_hash_table_lookup (sloader->loaded_plugins, info);
  if (!sinfo)
    return NULL;

  /* FIXME: instantiate new object and pass the parameters */
  extension_methods = seed_object_get_property (sinfo->context,
                                                sinfo->extensions,
                                                g_type_name (exten_type));
  if (!extension_methods || seed_value_is_undefined (sinfo->context, extension_methods) || seed_value_is_null (sinfo->context, extension_methods))
    return NULL;

  if (!seed_value_is_object (sinfo->context, extension_methods))
    {
      g_warning ("Extension '%s' in plugin '%s' is not a Seed object",
                 g_type_name (exten_type), peas_plugin_info_get_module_name (info));
      return NULL;
    }

  /* Copy the original extension_methods object to a new specific object. */
  extension = seed_make_object (sinfo->context, NULL, NULL);
  property_names = seed_object_copy_property_names (sinfo->context,
                                                    extension_methods);
  for (i = 0; property_names[i] != NULL; i++)
    {
      SeedValue value;

      value = seed_object_get_property (sinfo->context,
                                        extension_methods,
                                        property_names[i]);
      seed_object_set_property (sinfo->context,
                                extension,
                                property_names[i],
                                value);
    }

  /* Set the properties as well */
  for (i = 0; i < n_parameters; i++)
    {
      gchar *key;
      SeedValue value;

      /* We want to normalize the property names to have a '_' instead of the
       * conventional '-', to make them accessible through this.property_name */
      key = g_strdup (parameters[i].name);
      for (j = 0; key[j] != '\0'; j++)
        if (key[j] == '-')
          key[j] = '_';

      value = seed_value_from_gvalue (sinfo->context,
                                      &parameters[i].value,
                                      NULL);
      seed_object_set_property (sinfo->context,
                                extension,
                                key,
                                value);

      g_free (key);
    }

  g_strfreev (property_names);

  return peas_extension_seed_new (exten_type, sinfo->context, extension);
}

static void
peas_plugin_loader_seed_unload (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
  PeasPluginLoaderSeed *sloader = PEAS_PLUGIN_LOADER_SEED (loader);
  SeedInfo *sinfo;

  sinfo = (SeedInfo *) g_hash_table_lookup (sloader->loaded_plugins, info);
  if (!sinfo)
    return;

  g_hash_table_remove (sloader->loaded_plugins, info);
  seed_value_unprotect (sinfo->context, sinfo->extensions);
  seed_context_unref (sinfo->context);
  g_slice_free (SeedInfo, sinfo);
}

static void
peas_plugin_loader_seed_garbage_collect (PeasPluginLoader *loader)
{
  seed_context_collect (seed->context);
}

static void
peas_plugin_loader_seed_init (PeasPluginLoaderSeed *sloader)
{
  /* This is somewhat buggy as the seed engine cannot be reinitialized
   * and is shared among instances (esp wrt module paths), but apparently there
   * is no way to avoid having it shared... */
  if (!seed)
    seed = seed_init (NULL, NULL);

  sloader->loaded_plugins = g_hash_table_new (g_direct_hash, g_direct_equal);
}

static void
peas_plugin_loader_seed_class_init (PeasPluginLoaderSeedClass *klass)
{
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  loader_class->add_module_directory = peas_plugin_loader_seed_add_module_directory;
  loader_class->load = peas_plugin_loader_seed_load;
  loader_class->provides_extension = peas_plugin_loader_seed_provides_extension;
  loader_class->create_extension = peas_plugin_loader_seed_create_extension;
  loader_class->unload = peas_plugin_loader_seed_unload;
  loader_class->garbage_collect = peas_plugin_loader_seed_garbage_collect;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              PEAS_TYPE_PLUGIN_LOADER_SEED);
}
