#ifndef __TESTING_LOADABLE_PLUGIN_H__
#define __TESTING_LOADABLE_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_LOADABLE_PLUGIN         (testing_loadable_plugin_get_type ())
#define TESTING_LOADABLE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_LOADABLE_PLUGIN, TestingLoadablePlugin))
#define TESTING_LOADABLE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_LOADABLE_PLUGIN, TestingLoadablePlugin))
#define TESTING_IS_LOADABLE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_LOADABLE_PLUGIN))
#define TESTING_IS_LOADABLE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_LOADABLE_PLUGIN))
#define TESTING_LOADABLE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_LOADABLE_PLUGIN, TestingLoadablePluginClass))

typedef struct _TestingLoadablePlugin         TestingLoadablePlugin;
typedef struct _TestingLoadablePluginClass    TestingLoadablePluginClass;
typedef struct _TestingLoadablePluginPrivate  TestingLoadablePluginPrivate;

struct _TestingLoadablePlugin {
  PeasExtensionBase parent_instance;

  TestingLoadablePluginPrivate *priv;
};

struct _TestingLoadablePluginClass {
  PeasExtensionBaseClass parent_class;
};

GType                 testing_loadable_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types              (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_LOADABLE_PLUGIN_H__ */
