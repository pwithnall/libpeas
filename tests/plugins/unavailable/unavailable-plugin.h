#ifndef __TESTING_UNAVAILABLE_PLUGIN_H__
#define __TESTING_UNAVAILABLE_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_UNAVAILABLE_PLUGIN         (testing_unavailable_plugin_get_type ())
#define TESTING_UNAVAILABLE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_UNAVAILABLE_PLUGIN, TestingUnavailablePlugin))
#define TESTING_UNAVAILABLE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_UNAVAILABLE_PLUGIN, TestingUnavailablePlugin))
#define TESTING_IS_UNAVAILABLE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_UNAVAILABLE_PLUGIN))
#define TESTING_IS_UNAVAILABLE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_UNAVAILABLE_PLUGIN))
#define TESTING_UNAVAILABLE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_UNAVAILABLE_PLUGIN, TestingUnavailablePluginClass))

typedef struct _TestingUnavailablePlugin         TestingUnavailablePlugin;
typedef struct _TestingUnavailablePluginClass    TestingUnavailablePluginClass;
typedef struct _TestingUnavailablePluginPrivate  TestingUnavailablePluginPrivate;

struct _TestingUnavailablePlugin {
  PeasExtensionBase parent_instance;

  TestingUnavailablePluginPrivate *priv;
};

struct _TestingUnavailablePluginClass {
  PeasExtensionBaseClass parent_class;
};

GType                 testing_unavailable_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types                 (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_UNAVAILABLE_PLUGIN_H__ */
