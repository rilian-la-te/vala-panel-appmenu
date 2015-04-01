using Gtk;
using GLib;
using ValaPanel;
using Appmenu;

public class AppmenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new MenuApplet(toplevel,settings,number);
    }
}
public class MenuApplet: Applet
{
	Appmenu.ActiveMenu layout;
	public MenuApplet (Toplevel top, GLib.Settings? settings, uint number)
	{
		base(top,settings,number);
	}
	public override void create()
	{
		layout = new Appmenu.ActiveMenu();
		this.add(layout);
		show_all();
	}
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(AppmenuApplet));
}
