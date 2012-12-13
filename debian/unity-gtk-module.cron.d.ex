#
# Regular cron jobs for the unity-gtk-module package
#
0 4	* * *	root	[ -x /usr/bin/unity-gtk-module_maintenance ] && /usr/bin/unity-gtk-module_maintenance
