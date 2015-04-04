using GLib;
using Gtk;

namespace DBusMenu
{
    [DBus (use_string_marshalling = true)]
    public enum Status
    {
        [DBus (value = "normal")]
        NORMAL,
        [DBus (value = "notice")]
        NOTICE
    }
    [DBus (name = "com.canonical.dbusmenu")]
    public interface Iface : Object
    {
        public abstract uint version {get;}
        public abstract string text_direction {owned get;}
        [DBus (use_string_marshalling = true)]
        public abstract Status status {get;}
        public abstract string[] icon_theme_path {owned get;}
        /* layout signature is "(ia{sv}av)" */
        public abstract void get_layout(int parent_id,
                        int recursion_depth,
                        string[] property_names,
                        out uint revision,
                        [DBus (signature = "(ia{sv}av)")] out Variant layout) throws IOError;
        /* properties signature is "a(ia{sv})" */
        public abstract void get_group_properties(
                            int[] ids,
                            string[] property_names,
                            [DBus (signature = "a(ia{sv})")] out Variant properties) throws IOError;
        public abstract void get_property(int id, string name, out Variant value) throws IOError;
        public abstract void event(int id, string event_id, Variant? data, uint timestamp) throws IOError;
        /* events signature is a(isvu) */
        public abstract void event_group( [DBus (signature = "a(isvu)")] Variant events,
                                        out int[] id_errors) throws IOError;
        public abstract void about_to_show(int id, out bool need_update) throws IOError;
        public abstract void about_to_show_group(int[] ids, out int[] updates_needed, out int[] id_errors) throws IOError;
        /*updated properties signature is a(ia{sv}), removed is a(ias)*/
        public abstract signal void items_properties_updated(
                                [DBus (signature = "a(ia{sv})")] Variant updated_props,
                                [DBus (signature="a(ias)")] Variant removed_props);
        public abstract signal void layout_updated(uint revision, int parent);
        public abstract signal void item_activation_requested(int id, uint timestamp);
        public abstract signal void x_valapanel_item_value_changed(int id, uint timestamp);
    }

    public class PropertyStore : Object
    {
        VariantDict dict;
        HashTable<string,VariantType> checker;
        public Variant? get_prop(string name)
        {
            var type = checker.lookup(name);
            return (type != null) ? dict.lookup_value(name,type) : null;
        }
        public void set_prop(string name, Variant? val)
        {
            VariantType type = checker.lookup(name);
            if (val == null)
                dict.remove(name);
            else if (type != null && val.is_of_type(type))
                dict.insert_value(name,val);
            init_default();
        }
        construct
        {
            dict = new VariantDict();
            checker = new HashTable<string,VariantType>(str_hash,str_equal);
            checker.insert("visible", VariantType.BOOLEAN);
            checker.insert("enabled", VariantType.BOOLEAN);
            checker.insert("label", VariantType.STRING);
            checker.insert("type", VariantType.STRING);
            checker.insert("children-display", VariantType.STRING);
            checker.insert("toggle-type", VariantType.STRING);
            checker.insert("icon-name", VariantType.STRING);
            checker.insert("accessible-desc", VariantType.STRING);
            checker.insert("shortcut", new VariantType("aas"));
            checker.insert("toggle-state", VariantType.INT32);
            checker.insert("icon-data", new VariantType("ay"));
            checker.insert("disposition", VariantType.STRING);
            checker.insert("x-valapanel-secondary-icon-name", VariantType.STRING);
            checker.insert("x-valapanel-icon-size", VariantType.INT32);
            checker.insert("x-valapanel-min-value", VariantType.DOUBLE);
            checker.insert("x-valapanel-current-value", VariantType.DOUBLE);
            checker.insert("x-valapanel-max-value", VariantType.DOUBLE);
            checker.insert("x-valapanel-step-increment", VariantType.DOUBLE);
            checker.insert("x-valapanel-page-increment", VariantType.DOUBLE);
            checker.insert("x-valapanel-draw-value", VariantType.BOOLEAN);
            checker.insert("x-valapanel-format-value", VariantType.STRING);
        }
        public PropertyStore (Variant? props)
        {
            if (props != null)
            {
                VariantIter iter = props.iterator();
                string name;
                Variant v;
                while(iter.next ("{sv}", out name, out v))
                    this.set_prop(name,v);
            }
            init_default();
        }
        private void init_default()
        {
            if(!dict.contains("visible"))
                dict.insert_value("visible", new Variant.boolean(true));
            if(!dict.contains("enabled"))
                dict.insert_value("enabled", new Variant.boolean(true));
            if(!dict.contains("type"))
                dict.insert_value("type", new Variant.string("standard"));
            if(!dict.contains("label"))
                dict.insert_value("label", new Variant.string(""));
            if(!dict.contains("disposition"))
                dict.insert_value("disposition", new Variant.string("normal"));
        }
    }

    public class Item : Object
    {
        private Client client;
        private PropertyStore store;
        private List<int> children_ids;
        public int id
        {get; private set;}
        internal DateTime gc_tag;
        public signal void property_changed(string name, Variant? val);
        public signal void child_added(int id, Item item);
        public signal void child_removed(int id, Item item);
        public signal void child_moved(int oldpos, int newpos, Item item);
        public Item (int id, Client iface, Variant props, List<int> children_ids)
        {
            this.children_ids = children_ids.copy();
            this.client = iface;
            this.store = new PropertyStore(props);
            this.id = id;
        }
        public Variant get_variant_property(string name)
        {
            return store.get_prop(name);
        }
        public string get_string_property(string name)
        {
            return store.get_prop(name).get_string();
        }
        public bool get_bool_property(string name)
        {
            return (store.get_prop(name)!=null) ? store.get_prop(name).get_boolean() : false;
        }
        public int get_int_property(string name)
        {
            return (store.get_prop(name)!=null) ? store.get_prop(name).get_int32() : 0;
        }
        public List<int> get_children_ids()
        {
            return children_ids.copy();
        }
        public void set_variant_property(string name, Variant? val)
        {
            var old_value = this.store.get_prop(name);
            this.store.set_prop(name, val);
            var new_value = this.store.get_prop(name);
            if ((old_value ?? new_value) == null)
                return;
            if (new_value != null && old_value == null
                || old_value == null && new_value != null
                || !old_value.equal(new_value))
                this.property_changed(name,new_value);
        }
        public void add_child(int id, int pos)
        {
            children_ids.insert(id,pos);
            child_added(id,client.get_item(id));
        }
        public void remove_child(int id)
        {
            children_ids.remove(id);
            child_removed(id,client.get_item(id));
        }
        public void move_child(int id, int newpos)
        {
            var oldpos = children_ids.index(id);
            if (oldpos == newpos)
                return;
            children_ids.remove(id);
            children_ids.insert(id,newpos);
            child_moved(oldpos,newpos,client.get_item(id));
        }
        public List<Item> get_children()
        {
            List<Item> ret = new List<Item>();
            foreach (var id in children_ids)
                ret.append(client.get_item(id));
            return ret;
        }
        public int get_child_position(int child_id)
        {
            return children_ids.index(child_id);
        }
        public void handle_event(string event_id, Variant? data, uint timestamp)
        {
            try
            {
                client.iface.event(this.id,event_id,data ?? new Variant.int32(0),timestamp);
            } catch (Error e)
            {
                stderr.printf("%s\n",e.message);
            }
        }
        public void request_about_to_show()
        {
            client.request_about_to_show(this.id);
        }
    }
    public class Client : Object
    {
        private HashTable<int,Item> items;
        private bool layout_update_required;
        private bool layout_update_in_progress;
        private int[] requested_props_ids;
        private uint layout_revision;
        public Iface iface
        {get; private set;}
        construct
        {
            items = new HashTable<int,Item>(direct_hash, direct_equal);
        }
        public Client(string object_name, string object_path)
        {
            layout_revision = 0;
            try{
                this.iface = Bus.get_proxy_sync(BusType.SESSION, object_name, object_path);
            } catch (Error e) {stderr.printf("Cannot get menu! Error: %s",e.message);}
            VariantDict props = new VariantDict();
            props.insert("children-display","s","submenu");
            var item = new Item(0,this,props.end(),new List<int>());
            items.insert(0,item);
            request_layout_update();
            iface.layout_updated.connect((rev,parent)=>{request_layout_update();});
            iface.items_properties_updated.connect(props_updated_cb);
            iface.item_activation_requested.connect(request_activation_cb);
            iface.x_valapanel_item_value_changed.connect(request_value_cb);
            requested_props_ids = {};
        }
        public Item? get_root_item()
        {
            return items.lookup(0);
        }
        public Item? get_item(int id)
        {
            return items.lookup(id);
        }
        private void request_activation_cb(int id, uint timestamp)
        {
            get_item(id).handle_event("clicked",new Variant.int32(0),timestamp);
        }
        private void request_value_cb(int id, uint timestamp)
        {
            get_item(id).handle_event("value-changed",new Variant.double(get_item(id).get_variant_property("x-valapanel-current-value").get_double()),timestamp);
        }
        private void request_layout_update()
        {
            if(layout_update_in_progress)
                layout_update_required = true;
            else layout_update.begin();
        }
        /* the original implementation will only request partial layouts if somehow possible
        / we try to save us from multiple kinds of race conditions by always requesting a full layout */
        private async void layout_update()
        {
            /* Sanity check: Version can be 0 only if dbusmenu iface is not loaded yet*/
            if (iface.version < 1)
            {
                yield layout_update();
                return;
            }
            layout_update_required = false;
            layout_update_in_progress = true;
            string[] props = {"type", "children-display"};
            uint rev;
            Variant layout;
            try{
                iface.get_layout(0,-1,props,out rev, out layout);
            } catch (Error e) {
                debug("Cannot update layout. Error: %s\n Yielding another update...\n",e.message);
                return;
            }
            parse_layout(rev,layout);
            clean_items();
            if (layout_update_required)
                yield layout_update();
            else
                layout_update_in_progress = false;
        }
        private void parse_layout(uint rev, Variant layout)
        {
            /* Removed revision handling because of bug */
    //~         if (rev < layout_revision) return;
            /* layout signature must be "(ia{sv}av)" */
            int id = layout.get_child_value(0).get_int32();
            Variant props = layout.get_child_value(1);
            Variant children = layout.get_child_value(2);
            VariantIter chiter = children.iterator();
            List<int> children_ids = new List<int>();
            for(var child = chiter.next_value(); child != null; child = chiter.next_value())
            {
                child = child.get_variant();
                parse_layout(rev,child);
                int child_id = child.get_child_value(0).get_int32();
                children_ids.append(child_id);
            }
            if (id in items)
            {
                unowned Item item = items.lookup(id);
                VariantIter props_iter = props.iterator();
                string name;
                Variant val;
                while(props_iter.next("{sv}",out name, out val))
                    item.set_variant_property(name, val);
                /* make sure our children are all at the right place, and exist */
                var old_children_ids = item.get_children_ids();
                int i = 0;
                foreach(var new_id in children_ids)
                {
                    var old_child = -1;
                    foreach(var old_id in old_children_ids)
                        if (new_id == old_id)
                        {
                            old_child = old_id;
                            old_children_ids.remove(old_id);
                            break;
                        }
                    if (old_child < 0)
                        item.add_child(new_id,i);
                    else
                        item.move_child(old_child,i);
                    i++;
                }
                foreach (var old_id in old_children_ids)
                    item.remove_child(old_id);
            }
            else
            {
                items.insert(id, new Item(id,this,props,children_ids));
                request_properties.begin(id);
            }
            layout_revision = rev;
        }
        private void clean_items()
        {
        /* Traverses the list of cached menu items and removes everyone that is not in the list
        /  so we don't keep alive unused items */
        var tag = new DateTime.now_utc();
        List<int> traverse = new List<int>();
        traverse.append(0);
        while (traverse.length() > 0) {
            var item = this.get_item(traverse.data);
            traverse.delete_link(traverse);
            item.gc_tag = tag;
            traverse.concat(item.get_children_ids());
        }
        SList<int> remover = new SList<int>();
        items.foreach((k,v)=>{if (v.gc_tag != tag) remover.append(k);});
            foreach(var i in remover)
                items.remove(i);
        }
        /* we don't need to cache and burst-send that since it will not happen that frequently */
        public void request_about_to_show(int id)
        {
            var need_update = false;
            try
            {
                iface.about_to_show(id,out need_update);
            } catch (Error e)
            {
                stderr.printf("%s\n",e.message);
            }
            if (need_update)
                request_layout_update();
        }
        private async void request_properties(int id)
        {
            Variant props;
            string[] names = {};
            if (!(id in requested_props_ids))
                requested_props_ids += id;
            try{
                iface.get_group_properties(requested_props_ids,names,out props);
            } catch (GLib.Error e) {stderr.printf("%s\n",e.message);}
            requested_props_ids = {};
            parse_props(props);
        }
        private void props_updated_cb (Variant updated_props, Variant removed_props)
        {
            parse_props(updated_props);
            parse_props(removed_props);
        }
        private void parse_props(Variant props)
        {
            /*updated properties signature is a(ia{sv}), removed is a(ias)*/
            var iter = props.iterator();
            for (var props_req = iter.next_value(); props_req!=null; props_req = iter.next_value())
            {
                int req_id = props_req.get_child_value(0).get_int32();
                Variant props_id = props_req.get_child_value(1);
                var ch_iter = props_id.iterator();
                Variant? val;
                for (val = ch_iter.next_value(); val != null; val = ch_iter.next_value())
                {
                    if (val.get_type_string() == "{sv}")
                    {
                        var key = val.get_child_value(0).get_string();
                        var prop = val.get_child_value(1).get_variant();
                        if (items.lookup(req_id) != null)
                            items.lookup(req_id).set_variant_property(key,prop);
                    }
                    else if (val.get_type_string() == "s")
                    {
                        var key = val.get_string();
                        if (items.lookup(req_id) != null)
                            items.lookup(req_id).set_variant_property(key,null);
                    }
                }
            }
        }
    }
    public interface GtkItemIface : Object
    {
        public abstract Item item
        {get; protected set;}
        public static void parse_shortcut_variant(Variant shortcut, out uint key, out Gdk.ModifierType modifier)
        {
            print("%s\n",shortcut.print(false));
            key = 0;
            modifier = 0;
            VariantIter iter = shortcut.iterator();
            string str;
            while(iter.next("s", out str))
            {
                if (str == "Control") {
                    modifier |= Gdk.ModifierType.CONTROL_MASK;
                } else if (str == "Alt") {
                    modifier |= Gdk.ModifierType.MOD1_MASK;
                } else if (str == "Shift") {
                    modifier |= Gdk.ModifierType.SHIFT_MASK;
                } else if (str == "Super") {
                    modifier |= Gdk.ModifierType.SUPER_MASK;
                } else {
                    Gdk.ModifierType tempmod;
                    accelerator_parse(str, out key, out tempmod);
                }
            }
            return;
        }
    }
    public class GtkMainItem : CheckMenuItem, GtkItemIface
    {
        private static const string[] allowed_properties = {"visible","enabled","label","type",
                                                "children-display","toggle-type",
                                                "toggle-state","icon-name","icon-data","accessible-desc","x-valapanel-icon-size"};
        public Item item
        {get; protected set;}
        private bool has_indicator;
        private Box box;
        private Image image;
        private new AccelLabel label;
        private ulong activate_handler;
        private bool is_themed_icon;
        public GtkMainItem(Item item)
        {
            is_themed_icon = false;
            this.item = item;
            box = new Box(Orientation.HORIZONTAL, 5);
            image = new Image();
            label = new AccelLabel("");
            box.add(image);
            box.add(label);
            this.add(box);
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            item.child_added.connect(on_child_added_cb);
            item.child_removed.connect(on_child_removed_cb);
            item.child_moved.connect(on_child_moved_cb);
            activate_handler = this.activate.connect(on_toggled_cb);
            this.select.connect(on_select_cb);
            this.deselect.connect(on_deselect_cb);
            this.notify["visible"].connect(()=>{this.visible=item.get_bool_property("visible");});
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            if(activate_handler > 0)
                SignalHandler.block(this,activate_handler);
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
                case "label":
                    label.set_text_with_mnemonic(val.get_string());
                    break;
                case "children-display":
                    if (val != null && val.get_string() == "submenu")
                    {
                        this.submenu = new Gtk.Menu();
                        this.submenu.insert.connect(on_child_insert_cb);
                        foreach(var item in this.item.get_children())
                            submenu.add(GtkClient.new_item(item));
                    }
                    else
                        this.submenu = null;
                    break;
                case "toggle-type":
                    if (val == null)
                        this.set_toggle_type("normal");
                    else
                        this.set_toggle_type(val.get_string());
                    break;
                case "toggle-state":
                    if (val != null && val.get_int32()>0)
                        this.active = true;
                    else
                        this.active = false;
                    break;
                case "accessible-desc":
                    this.set_tooltip_text(val != null ? val.get_string() : null);
                    break;
                case "icon-name":
                case "icon-data":
                    update_icon(val);
                    break;
                case "shortcut":
                    update_shortcut(val);
                    break;
            }
            if(activate_handler > 0)
                SignalHandler.unblock(this,activate_handler);
        }
        private void set_toggle_type(string type)
        {
            if (type=="radio")
            {
                this.set_accessible_role(Atk.Role.RADIO_MENU_ITEM);
                this.has_indicator = true;
                this.draw_as_radio = true;
            }
            else if (type=="checkmark")
            {
                this.set_accessible_role(Atk.Role.CHECK_MENU_ITEM);
                this.has_indicator = true;
                this.draw_as_radio = false;
            }
            else
            {
                this.set_accessible_role(Atk.Role.MENU_ITEM);
                this.has_indicator = false;
            }
        }
        private void update_icon(Variant? val)
        {
            if (val == null)
            {
                var icon = image.gicon;
                if (!(icon != null && icon is ThemedIcon && is_themed_icon))
                    is_themed_icon = false;
                return;
            }
            Icon? icon = null;
            if (val.get_type_string() == "s")
            {
                is_themed_icon = true;
                icon = new ThemedIcon.with_default_fallbacks(val.get_string()+"-symbolic");
            }
            else if (!is_themed_icon && val.get_type_string() == "ay")
                icon = new BytesIcon(val.get_data_as_bytes());
            else return;
            image.set_from_gicon(icon,IconSize.MENU);
            image.set_pixel_size(16);
        }
        private void update_shortcut(Variant? val)
        {
            if (val == null)
                return;
            uint key;
            Gdk.ModifierType mod;
            parse_shortcut_variant(val, out key, out mod);
            this.label.set_accel(key,mod);
        }
        private void on_child_added_cb(int id,Item item)
        {
            if (this.submenu != null)
                this.submenu.append (GtkClient.new_item(item));
            else
            {
                debug("Adding new item to item without submenu! Creating new submenu...\n");
                this.submenu = new Gtk.Menu();
                this.submenu.append (GtkClient.new_item(item));
            }
        }
        private void on_child_removed_cb(int id, Item item)
        {
            if (this.submenu != null)
                foreach(var ch in this.submenu.get_children())
                    if ((ch as GtkItemIface).item == item)
                        ch.destroy();
            else
                debug("Cannot remove a child from item without submenu!\n");
        }
        private void on_child_moved_cb(int oldpos, int newpos, Item item)
        {
            if (this.submenu != null)
                foreach(var ch in this.submenu.get_children())
                    if ((ch as GtkItemIface).item == item)
                        this.submenu.reorder_child(ch,newpos);
            else
                debug("Cannot move a child of item with has no children!\n");
        }
        private void on_toggled_cb()
        {
            item.handle_event("clicked",new Variant.int32(0),get_current_event_time());
        }
        private void on_select_cb()
        {
            if (this.submenu != null)
            {
                item.handle_event("opened",null,0);
                item.request_about_to_show();
            }
        }
        private void on_deselect_cb()
        {
            if (this.submenu != null)
                item.handle_event("closed",null,0);
        }
        private void on_child_insert_cb(Widget w, int pos)
        {
            var ch = w as GtkItemIface;
            this.submenu.reorder_child(w,item.get_child_position(ch.item.id));
            this.submenu.queue_resize();
        }
        protected override void draw_indicator(Cairo.Context cr)
        {
            if (has_indicator)
                base.draw_indicator(cr);
        }
    }
    public class GtkSeparatorItem: SeparatorMenuItem, GtkItemIface
    {
        private static const string[] allowed_properties = {"visible","enabled"};
        public Item item
        {get; protected set;}
        public GtkSeparatorItem(Item item)
        {
            this.item = item;
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
            }
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
    }
    public class GtkSliderItem: Gtk.MenuItem, GtkItemIface
    {
        private static const string[] allowed_properties = {"visible","enabled","icon-name",
                                                            "x-valapanel-min-value","x-valapanel-current-value","x-valapanel-max-value",
                                                            "x-valapanel-step-increment","x-valapanel-page-increment","x-valapanel-draw-value",
                                                            "x-valapanel-format-value"};
        public Item item
        {get; protected set;}
        private Box box;
        private Image primary;
        private Scale slider;
        private Adjustment adj;
        private string item_format;
        private bool grabbed;
        public GtkSliderItem(Item item)
        {
            this.item = item;
            box = new Box(Orientation.HORIZONTAL,5);
            primary = new Image();
            adj = new Adjustment(0,0,double.MAX,0,0,0);
            slider = new Scale(Orientation.HORIZONTAL,adj);
            slider.hexpand = true;
            box.add(primary);
            box.add(slider);
            this.add(box);
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            adj.value_changed.connect(on_value_changed_cb);
            slider.format_value.connect(on_value_format_cb);
            slider.value_pos = PositionType.RIGHT;
            this.add_events (Gdk.EventMask.SCROLL_MASK
                            |Gdk.EventMask.POINTER_MOTION_MASK
                            |Gdk.EventMask.BUTTON_MOTION_MASK);
            this.set_size_request(200,-1);
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
                case "icon-name":
                    primary.set_from_gicon (icon_from_name(val),IconSize.MENU);
                    break;
                case "x-valapanel-min-value":
                    adj.lower = val.get_double();
                    break;
                case "x-valapanel-current-value":
                    adj.value = val.get_double();
                    break;
                case "x-valapanel-max-value":
                    adj.upper = val.get_double();
                    break;
                case "x-valapanel-step-increment":
                    adj.step_increment = val.get_double();
                    break;
                case "x-valapanel-page-increment":
                    adj.page_increment = val.get_double();
                    break;
                case "x-valapanel-draw-value":
                    slider.draw_value = val.get_boolean();
                    break;
                case "x-valapanel-format-value":
                    this.item_format = val.get_string();
                    break;
            }
        }
        private Icon icon_from_name(Variant? namev)
        {
            if (namev != null)
            {
                return new ThemedIcon.with_default_fallbacks(namev.get_string() + "-symbolic");
            }
            return new ThemedIcon.with_default_fallbacks("image-missing-symbolic");
        }
        private void on_value_changed_cb()
        {
            item.handle_event("value-changed",new Variant.double(adj.value),get_current_event_time());
        }
        private string on_value_format_cb(double val)
        {
            return item_format.printf(val);
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
        protected override bool button_press_event(Gdk.EventButton event)
        {
            slider.event(event);
            if (!grabbed)
                grabbed = true;
            return true;
        }
        protected override bool button_release_event(Gdk.EventButton event)
        {
            slider.event (event);
            if (grabbed)
            {
                grabbed = false;
                this.grab_broken_event (null);
            }
            return true;
        }
        protected override bool motion_notify_event(Gdk.EventMotion event)
        {
            slider.event (event);
            return true;
        }
        protected override bool scroll_event(Gdk.EventScroll event)
        {
            slider.event (event);
            return true;
        }
    }
    public class GtkMenuBarItem : Gtk.MenuItem, GtkItemIface
    {
        private static const string[] allowed_properties = {"visible","enabled","label","type",
                                                "children-display", "x-valapanel-icon-size",
                                                "icon-name","icon-data","accessible-desc"};
        public Item item
        {get; protected set;}
        private Box box;
        private Image image;
        private new AccelLabel label;
        private ulong activate_handler;
        private bool is_themed_icon;
        public GtkMenuBarItem(Item item)
        {
            is_themed_icon = false;
            this.item = item;
            box = new Box(Orientation.HORIZONTAL, 5);
            image = new Image();
            label = new AccelLabel("");
            box.add(image);
            box.add(label);
            this.add(box);
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            item.child_added.connect(on_child_added_cb);
            item.child_removed.connect(on_child_removed_cb);
            item.child_moved.connect(on_child_moved_cb);
            activate_handler = this.activate.connect(on_toggled_cb);
            this.select.connect(on_select_cb);
            this.deselect.connect(on_deselect_cb);
            this.set_accessible_role(Atk.Role.MENU_ITEM);
            this.notify["visible"].connect(()=>{this.visible=item.get_bool_property("visible");});
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            if(activate_handler > 0)
                SignalHandler.block(this,activate_handler);
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
                case "label":
                    label.set_text_with_mnemonic(val.get_string());
                    break;
                case "children-display":
                    if (val != null && val.get_string() == "submenu")
                    {
                        this.submenu = new Gtk.Menu();
                        this.submenu.insert.connect(on_child_insert_cb);
                        foreach(var item in this.item.get_children())
                            submenu.add(GtkClient.new_item(item));
                    }
                    else
                        this.submenu = null;
                    break;
                case "accessible-desc":
                    this.set_tooltip_text(val != null ? val.get_string() : null);
                    break;
                case "icon-name":
                case "icon-data":
                    update_icon(val);
                    break;
                case "shortcut":
                    update_shortcut(val);
                    break;
                case "x-valapanel-icon-size":
                    image.set_pixel_size(val != null ? val.get_int32() : 16);
                    break;
            }
            if(activate_handler > 0)
                SignalHandler.unblock(this,activate_handler);
        }
        private void update_icon(Variant? val)
        {
            if (val == null)
            {
                var icon = image.gicon;
                if (icon == null)
                    image.hide();
                else if (!(icon is ThemedIcon && is_themed_icon))
                    is_themed_icon = false;
                return;
            }
            Icon? icon = null;
            if (val.get_type_string() == "s")
            {
                is_themed_icon = true;
                icon = new ThemedIcon.with_default_fallbacks(val.get_string()+"-symbolic");
            }
            else if (!is_themed_icon && val.get_type_string() == "ay")
                icon = new BytesIcon(val.get_data_as_bytes());
            else
                return;
            image.set_from_gicon(icon,IconSize.MENU);
            image.show();
        }
        private void update_shortcut(Variant? val)
        {
            if (val == null)
                return;
            uint key;
            Gdk.ModifierType mod;
            parse_shortcut_variant(val, out key, out mod);
            this.label.set_accel(key,mod);
        }
        private void on_child_added_cb(int id,Item item)
        {
            if (this.submenu != null)
                this.submenu.append (GtkClient.new_item(item));
            else
            {
                debug("Adding new item to item without submenu! Creating new submenu...\n");
                this.submenu = new Gtk.Menu();
                this.submenu.append (GtkClient.new_item(item));
            }
        }
        private void on_child_removed_cb(int id, Item item)
        {
            if (this.submenu != null)
                foreach(var ch in this.submenu.get_children())
                    if ((ch as GtkItemIface).item == item)
                        ch.destroy();
            else
               debug("Cannot remove a child from item without submenu!\n");
        }
        private void on_child_moved_cb(int oldpos, int newpos, Item item)
        {
            if (this.submenu != null)
                foreach(var ch in this.submenu.get_children())
                    if ((ch as GtkItemIface).item == item)
                        this.submenu.reorder_child(ch,newpos);
            else
                debug("Cannot move a child of item with has no children!\n");
        }
        private void on_toggled_cb()
        {
            item.handle_event("clicked",new Variant.int32(0),get_current_event_time());
        }
        private void on_select_cb()
        {
            if (this.submenu != null)
            {
                item.handle_event("opened",null,0);
                item.request_about_to_show();
            }
        }
        private void on_deselect_cb()
        {
            if (this.submenu != null)
                item.handle_event("closed",null,0);
        }
        private void on_child_insert_cb(Widget w, int pos)
        {
            var ch = w as GtkItemIface;
            this.submenu.reorder_child(w,item.get_child_position(ch.item.id));
            this.submenu.queue_resize();
        }
    }
    public class GtkClient : Client
    {
        private Gtk.MenuShell root_menu;
        public static Gtk.MenuItem new_item(Item item)
        {
            if (item.get_string_property("type") == "separator")
                return new GtkSeparatorItem(item);
            else if (item.get_string_property("type") == "slider")
                return new GtkSliderItem(item);
            return new GtkMainItem(item);
        }
        private static Gtk.MenuItem new_menubar_item(Item item)
        {
            if (item.get_string_property("type") == "separator")
                return new GtkSeparatorItem(item);
            return new GtkMenuBarItem(item);
        }
        public GtkClient(string object_name, string object_path)
        {
            base(object_name,object_path);
            this.root_menu = null;
        }
        public void attach_to_menu(Gtk.MenuShell menu)
        {
            foreach (var path in iface.icon_theme_path)
                IconTheme.get_default().prepend_search_path(path);
            root_menu = menu;
            root_menu.foreach((c)=>{menu.remove(c);});
            root_menu.realize.connect(open_cb);
            root_menu.unrealize.connect(close_cb);
            get_root_item().child_added.connect(on_child_added_cb);
            get_root_item().child_moved.connect(on_child_moved_cb);
            get_root_item().child_removed.connect(on_child_removed_cb);
            foreach(var ch in get_root_item().get_children())
                on_child_added_cb(ch.id,ch);
            foreach(var path in iface.icon_theme_path)
                IconTheme.get_default().append_search_path(path);
            root_menu.show();
        }
        private void open_cb()
        {
            get_root_item().handle_event("opened",null,0);
            get_root_item().request_about_to_show();
            root_menu.queue_resize();
        }
        private void close_cb()
        {
            get_root_item().handle_event("closed",null,0);
        }
        private void on_child_added_cb(int id, Item item)
        {
            Gtk.MenuItem menuitem;
            if (this.root_menu is Gtk.MenuBar)
                menuitem = new_menubar_item(item);
            else
                menuitem = new_item(item);
            root_menu.insert(menuitem,get_root_item().get_child_position(item.id));
        }
        private void on_child_moved_cb(int oldpos, int newpos, Item item)
        {
            foreach(var ch in root_menu.get_children())
                if ((ch as GtkItemIface).item == item)
                {
                    ch.ref();
                    root_menu.remove(ch);
                    root_menu.insert(ch,newpos);
                    ch.unref();
                }
        }
        private void on_child_removed_cb(int id, Item item)
        {
            foreach(var ch in root_menu.get_children())
                if ((ch as GtkItemIface).item == item)
                    ch.destroy();
        }
        public static bool check (string bus_name, string object_path)
        {
            try
            {
                Iface iface = Bus.get_proxy_sync(BusType.SESSION,bus_name,object_path);
                if (iface.version < 2) return false;
                else return true;
            } catch (Error e){}
            return false;
        }
    }
}
