#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "unavailable-plugin.h"

struct _TestingUnavailablePluginPrivate {
  GObject *object;
};

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingUnavailablePlugin,
                                testing_unavailable_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_unavailable_plugin_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  TestingUnavailablePlugin *plugin = TESTING_UNAVAILABLE_PLUGIN (object);

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
testing_unavailable_plugin_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  TestingUnavailablePlugin *plugin = TESTING_UNAVAILABLE_PLUGIN (object);

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
testing_unavailable_plugin_init (TestingUnavailablePlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              TESTING_TYPE_UNAVAILABLE_PLUGIN,
                                              TestingUnavailablePluginPrivate);
}

static void
testing_unavailable_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_unavailable_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_unavailable_plugin_class_init (TestingUnavailablePluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_unavailable_plugin_set_property;
  object_class->get_property = testing_unavailable_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (TestingUnavailablePluginPrivate));
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_unavailable_plugin_activate;
  iface->deactivate = testing_unavailable_plugin_deactivate;
}

static void
testing_unavailable_plugin_class_finalize (TestingUnavailablePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_unavailable_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_UNAVAILABLE_PLUGIN);
}
