using GLib;
using Appmenu;
using Xfce;

public class AppmenuPlugin : Xfce.PanelPlugin {

    public override void @construct() {
        GLib.Intl.setlocale(LocaleCategory.CTYPE,"");
        GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
        GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
        GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);
        widget = new AppMenuBar();
        add(widget);
        add_action_widget(widget);
        this.width_request = -1;
        widget.show_all();
    }
    private Gtk.Widget widget;
}

[ModuleInit]
public Type xfce_panel_module_init (TypeModule module) {
    return typeof (AppmenuPlugin);
}
