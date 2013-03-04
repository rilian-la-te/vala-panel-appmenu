import autopilot.emulators.X11
import autopilot.introspection.gtk
import pyatspi.registry
import pyatspi.utils
import time
import unity.tests

def introspect(node, depth):
    print '%s"%s" - %s' % (depth * ' ', node.name, node.get_role_name())

    for child in node:
        introspect(child, depth + 2)

class GeditTestCase(unity.tests.UnityTestCase, autopilot.introspection.gtk.GtkIntrospectionTestMixin):

    def setUp(self):
        super(GeditTestCase, self).setUp()

        self.app = self.launch_test_application('gedit')
        registry = pyatspi.registry.Registry()
        self.desktop = registry.getDesktop(0)

        time.sleep(0.2)

    def test_file_new(self):
        """Test if menu item insertion works."""
        panel = pyatspi.utils.findAllDescendants(self.desktop, lambda a: a.name == 'unity-panel-service')[0]
        menubar = panel[0]
        documents_item = menubar[7]
        documents_menu = documents_item[0]

        panel = self.unity.panels.get_active_panel()
        time.sleep(0.2)
        menu = panel.menus.get_menu_by_label('_File')
        time.sleep(0.2)
        menu.mouse_click()
        time.sleep(0.2)

        untitled_document_1_item = documents_menu[-3]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))

        self.keyboard.press_and_release('Down')
        time.sleep(0.2)
        self.keyboard.press_and_release('Enter')
        time.sleep(0.2)

        menu = panel.menus.get_menu_by_label('_Documents')
        time.sleep(0.2)
        menu.mouse_click()
        time.sleep(0.2)
        menu.mouse_click()
        time.sleep(0.2)

        tabs = self.app.select_many('GeditTab')
        self.assertTrue(len(tabs) == 2)
        self.assertTrue(tabs[0].name == 'Untitled Document 1')
        self.assertTrue(tabs[1].name == 'Untitled Document 2')

        untitled_document_2_item = documents_menu[-3]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        # self.assertTrue(untitled_document_2_item.name == 'Untitled Document 2')
        # self.assertFalse(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))
        # self.assertTrue(untitled_document_2_item.get_state_set().contains(pyatspi.STATE_CHECKED))

    def test_file_quit(self):
        """Test if menu item activation works."""
        panel = self.unity.panels.get_active_panel()
        time.sleep(0.2)
        menu = panel.menus.get_menu_by_label('_File')
        time.sleep(0.2)
        menu.mouse_click()
        time.sleep(0.2)

        self.keyboard.press_and_release('Up')
        time.sleep(0.2)
        self.keyboard.press_and_release('Enter')
        time.sleep(0.2)

        self.assertFalse(self.app_is_running('Text Editor'))

    def test_view_toolbar(self):
        """Test if check menu item activation works."""
        panel = pyatspi.utils.findAllDescendants(self.desktop, lambda a: a.name == 'unity-panel-service')[0]
        menubar = panel[0]
        view_item = menubar[2]
        view_menu = view_item[0]
        toolbar_item = view_menu[1]
        checked = toolbar_item.get_state_set().contains(pyatspi.STATE_CHECKED)

        toolbar = self.app.select_many('GtkToolbar')[0]
        visible = toolbar.visible

        self.assertTrue(checked == visible)

        panel = self.unity.panels.get_active_panel()
        time.sleep(0.2)
        menu = panel.menus.get_menu_by_label('_View')
        time.sleep(0.2)
        menu.mouse_click()
        time.sleep(0.2)

        self.keyboard.press_and_release('Down')
        time.sleep(0.2)
        self.keyboard.press_and_release('Enter')
        time.sleep(0.2)

        self.assertTrue(checked == visible)
        self.assertFalse(toolbar.visible == visible)
        self.assertFalse(toolbar_item.get_state_set().contains(pyatspi.STATE_CHECKED) == checked)

        menu.mouse_click()
        time.sleep(0.2)

        self.keyboard.press_and_release('Down')
        time.sleep(0.2)
        self.keyboard.press_and_release('Enter')
        time.sleep(0.2)

        menu.mouse_click()
        time.sleep(0.2)

        self.assertTrue(checked == visible)
        self.assertTrue(toolbar.visible == visible)
        self.assertTrue(toolbar_item.get_state_set().contains(pyatspi.STATE_CHECKED) == checked)
