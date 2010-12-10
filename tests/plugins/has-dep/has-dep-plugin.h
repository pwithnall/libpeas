#ifndef __TESTING_HAS_DEP_PLUGIN_H__
#define __TESTING_HAS_DEP_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_HAS_DEP_PLUGIN         (testing_has_dep_plugin_get_type ())
#define TESTING_HAS_DEP_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_HAS_DEP_PLUGIN, TestingHasDepPlugin))
#define TESTING_HAS_DEP_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_HAS_DEP_PLUGIN, TestingHasDepPlugin))
#define TESTING_IS_HAS_DEP_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_HAS_DEP_PLUGIN))
#define TESTING_IS_HAS_DEP_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_HAS_DEP_PLUGIN))
#define TESTING_HAS_DEP_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_HAS_DEP_PLUGIN, TestingHasDepPluginClass))

typedef struct _TestingHasDepPlugin         TestingHasDepPlugin;
typedef struct _TestingHasDepPluginClass    TestingHasDepPluginClass;
typedef struct _TestingHasDepPluginPrivate  TestingHasDepPluginPrivate;

struct _TestingHasDepPlugin {
  PeasExtensionBase parent_instance;

  TestingHasDepPluginPrivate *priv;
};

struct _TestingHasDepPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType                 testing_has_dep_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types             (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_HAS_DEP_PLUGIN_H__ */
