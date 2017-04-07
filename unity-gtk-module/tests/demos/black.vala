class BlackBox : Gtk.Box {
	protected override bool draw (Cairo.Context cr) {
		cr.set_source_rgb (0.0, 0.0, 0.0);
		cr.paint ();
		return false;
	}
}

void main (string[] args) {
	Gtk.init (ref args);

	Bus.own_name (BusType.SESSION, "demo.black", 0, null, null, null);

	var window = new Gtk.Window (Gtk.WindowType.TOPLEVEL);
	window.set_default_size (400, 300);
    var box = new Gtk.Box (Gtk.Orientation.VERTICAL, 0);
	window.add (box);

	var menubar = new Gtk.MenuBar ();

	var file_item = new Gtk.MenuItem.with_label ("File");
	var file = new Gtk.Menu ();
	file_item.set_submenu (file);
	menubar.add (file_item);
	file.add (new Gtk.MenuItem.with_label ("New"));
	file.add (new Gtk.MenuItem.with_label ("Open"));
	file.add (new Gtk.MenuItem.with_label ("Save"));
	file.add (new Gtk.MenuItem.with_label ("Quit"));

	var edit_item = new Gtk.MenuItem.with_label ("Edit");
	var edit = new Gtk.Menu ();
	edit_item.set_submenu (edit);
	menubar.add (edit_item);
	edit.add (new Gtk.MenuItem.with_label ("Cut"));
	edit.add (new Gtk.MenuItem.with_label ("Copy"));
	edit.add (new Gtk.MenuItem.with_label ("Paste"));

	var view_item = new Gtk.MenuItem.with_label ("View");
	var view = new Gtk.Menu ();
	view_item.set_submenu (view);
	menubar.add (view_item);
	view.add (new Gtk.CheckMenuItem.with_label ("Fullscreen"));

	var help_item = new Gtk.MenuItem.with_label ("Help");
	var help = new Gtk.Menu ();
	help_item.set_submenu (help);
	menubar.add (help_item);
	help.add (new Gtk.MenuItem.with_label ("Contents"));
	help.add (new Gtk.MenuItem.with_label ("About"));

	box.pack_start (menubar, false, false, 0);
	box.pack_start (new BlackBox (), true, true, 0);

	window.show_all ();

	Gtk.main ();
}
