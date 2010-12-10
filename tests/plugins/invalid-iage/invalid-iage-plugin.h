#ifndef __TESTING_INVALID_IAGE_PLUGIN_H__
#define __TESTING_INVALID_IAGE_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_INVALID_IAGE_PLUGIN         (testing_invalid_iage_plugin_get_type ())
#define TESTING_INVALID_IAGE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_INVALID_IAGE_PLUGIN, TestingInvalidIAgePlugin))
#define TESTING_INVALID_IAGE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_INVALID_IAGE_PLUGIN, TestingInvalidIAgePlugin))
#define TESTING_IS_INVALID_IAGE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_INVALID_IAGE_PLUGIN))
#define TESTING_IS_INVALID_IAGE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_INVALID_IAGE_PLUGIN))
#define TESTING_INVALID_IAGE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_INVALID_IAGE_PLUGIN, TestingInvalidIAgePluginClass))

typedef struct _TestingInvalidIAgePlugin         TestingInvalidIAgePlugin;
typedef struct _TestingInvalidIAgePluginClass    TestingInvalidIAgePluginClass;
typedef struct _TestingInvalidIAgePluginPrivate  TestingInvalidIAgePluginPrivate;

struct _TestingInvalidIAgePlugin {
  PeasExtensionBase parent_instance;

  TestingInvalidIAgePluginPrivate *priv;
};

struct _TestingInvalidIAgePluginClass {
  PeasExtensionBaseClass parent_class;
};

GType                 testing_invalid_iage_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types                  (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_INVALID_IAGE_PLUGIN_H__ */
