#ifndef REGISTRARMAIN_H
#define REGISTRARMAIN_H

#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(RegistrarApplication, registrar_application, REGISTRAR, APPLICATION,
                     GApplication)

RegistrarApplication *registrar_application_new();

G_END_DECLS

#endif // REGISTRARMAIN_H
