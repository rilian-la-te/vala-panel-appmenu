using GLib;
using Appmenu;
using Xfce;

public class AppmenuPlugin : Xfce.PanelPlugin {

    public override void @construct() {
        widget = new ActiveMenu();
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
