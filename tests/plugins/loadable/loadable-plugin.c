#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "loadable-plugin.h"

struct _TestingLoadablePluginPrivate {
  GObject *object;
};

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingLoadablePlugin,
                                testing_loadable_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_loadable_plugin_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  TestingLoadablePlugin *plugin = TESTING_LOADABLE_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      plugin->priv->object = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_loadable_plugin_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  TestingLoadablePlugin *plugin = TESTING_LOADABLE_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, plugin->priv->object);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


static void
testing_loadable_plugin_init (TestingLoadablePlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              TESTING_TYPE_LOADABLE_PLUGIN,
                                              TestingLoadablePluginPrivate);
}

static void
testing_loadable_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_loadable_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_loadable_plugin_class_init (TestingLoadablePluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_loadable_plugin_set_property;
  object_class->get_property = testing_loadable_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (TestingLoadablePluginPrivate));
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_loadable_plugin_activate;
  iface->deactivate = testing_loadable_plugin_deactivate;
}

static void
testing_loadable_plugin_class_finalize (TestingLoadablePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_loadable_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_LOADABLE_PLUGIN);
}
