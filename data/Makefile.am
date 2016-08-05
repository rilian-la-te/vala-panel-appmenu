gsettings_SCHEMAS = com.canonical.unity-gtk-module.gschema.xml
@GSETTINGS_RULES@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = unity-gtk$(GTK_VERSION)-parser.pc

upstart_jobdir = $(datadir)/upstart/sessions/
dist_upstart_job_DATA = unity-gtk-module.conf

systemd_userunitdir = $(SYSTEMD_USERUNITDIR)
systemd_userunit_DATA = unity-gtk-module.service

upstart_systemd_overridedir = $(datadir)/upstart/systemd-session/upstart/
upstart_systemd_override_DATA = unity-gtk-module.override

EXTRA_DIST = $(dist_upstart_job_DATA)
