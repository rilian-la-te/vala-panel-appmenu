import autopilot.emulators.X11
import autopilot.introspection.gtk
import os
import pyatspi.registry
import pyatspi.utils
import time
import unity.tests

def print_accessible(root, level=0):
    print level * ' ', root

    for node in root:
        print_accessible(node, level + 1)

def get_panel_accessible(root):
    is_panel = lambda a: a.name == 'unity-panel-service' and a.get_role_name() == 'application'
    return pyatspi.utils.findDescendant(root, is_panel, True)

def get_app_menu_accessible(root):
    is_app_menu = lambda a: len(a) > 0 and a[0].name == 'File' and a[0].get_role_name() == 'label'
    return pyatspi.utils.findDescendant(root, is_app_menu, True)

class GeditTestCase(unity.tests.UnityTestCase, autopilot.introspection.gtk.GtkIntrospectionTestMixin):

    def setUp(self):
        super(GeditTestCase, self).setUp()

        registry = pyatspi.registry.Registry()
        self.desktop = registry.getDesktop(0)

        module_name = 'unity-gtk-module'

        if os.path.isfile(module_name):
            modules = [module for module in os.getenv('GTK_MODULES', '').split(':') if module]
            modules = [module for module in modules if module != 'unity-gtk-module']
            modules.append(module_name)

            self.patch_environment('GTK_MODULES', ':'.join(modules))

        # This is needed on systems other than the EN locale
        os.putenv("LC_ALL", "C")
        self.addCleanup(os.unsetenv, "LC_ALL")

    def test_file_new(self):
        """Test if menu item insertion works."""
        self.app = self.launch_test_application('gedit')
        time.sleep(0.2)

        # Open and close the Documents menu
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that Untitled Document 1 is checked
        panel = get_panel_accessible(self.desktop)
        app_menu = get_app_menu_accessible(panel)
        documents_item = app_menu[5]
        documents_menu = documents_item[0]
        untitled_document_1_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))

        # Activate File > New
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_File')
        menu.mouse_click()
        self.keyboard.press_and_release('Down')
        self.keyboard.press_and_release('Enter')

        # Open and close the Documents menu
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that two documents are open
        tabs = self.app.select_many('GeditTab')
        self.assertTrue(len(tabs) == 2)
        self.assertTrue(tabs[0].name == 'Untitled Document 1')
        self.assertTrue(tabs[1].name == 'Untitled Document 2')

        # Assert that Untitled Document 2 is checked
        untitled_document_1_item = documents_menu[-2]
        untitled_document_2_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_2_item.name == 'Untitled Document 2')
        self.assertFalse(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))
        self.assertTrue(untitled_document_2_item.get_state_set().contains(pyatspi.STATE_CHECKED))

    def test_file_close(self):
        """Test if menu item removal works."""
        self.app = self.launch_test_application('gedit')
        time.sleep(0.2)

        # Open and close the Documents menu
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that Untitled Document 1 is checked
        panel = get_panel_accessible(self.desktop)
        app_menu = get_app_menu_accessible(panel)
        documents_item = app_menu[5]
        documents_menu = documents_item[0]
        untitled_document_1_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))

        # Activate File > Close
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_File')
        menu.mouse_click()
        self.keyboard.press_and_release('Up')
        self.keyboard.press_and_release('Up')
        self.keyboard.press_and_release('Enter')

        # Open and close the Documents menu
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that no documents are open
        tabs = self.app.select_many('GeditTab')
        self.assertFalse(tabs)

        # Assert that Untitled Document 1 was removed
        move_to_new_window_item = documents_menu[-1]
        self.assertTrue(move_to_new_window_item.name == 'Move to New Window')

    def test_file_quit(self):
        """Test if menu item activation works."""
        self.app = self.launch_test_application('gedit')
        time.sleep(0.2)

        # Activate File > Quit
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_File')
        menu.mouse_click()
        self.keyboard.press_and_release('Up')
        self.keyboard.press_and_release('Enter')

        # Assert that the application quit
        self.assertFalse(self.app_is_running('Text Editor'))

    def test_edit_undo(self):
        """Test if menu item sensitivity works."""
        self.app = self.start_app_window('Text Editor')
        time.sleep(2.2)

        # Hi!
        self.keyboard.type('hi')

        # Assert that Undo is sensitive
        panel = get_panel_accessible(self.desktop)
        app_menu = get_app_menu_accessible(panel)
        edit_item = app_menu[1]
        edit_menu = edit_item[0]
        undo_item = edit_menu[0]
        self.assertTrue(undo_item.name == 'Undo')
        self.assertTrue(undo_item.get_state_set().contains(pyatspi.STATE_SENSITIVE))

        # Activate Edit > Undo
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_Edit')
        menu.mouse_click()
        self.keyboard.press_and_release('Down')
        self.keyboard.press_and_release('Enter')

        # Open and close the Edit menu
        menu.mouse_click()
        menu.mouse_click()

        # Assert that Undo is insensitive
        self.assertFalse(undo_item.get_state_set().contains(pyatspi.STATE_SENSITIVE))

    def test_view_toolbar(self):
        """Test if check menu item activation works."""
        self.app = self.launch_test_application('gedit')
        time.sleep(0.2)

        # Assert that View > Toolbar matches the visibility of the tool bar
        panel = get_panel_accessible(self.desktop)
        app_menu = get_app_menu_accessible(panel)
        view_item = app_menu[2]
        view_menu = view_item[0]
        toolbar_item = view_menu[0]
        checked = toolbar_item.get_state_set().contains(pyatspi.STATE_CHECKED)
        toolbar = self.app.select_many('GtkToolbar')[0]
        visible = toolbar.visible
        self.assertTrue(checked == visible)

        # Activate View > Toolbar
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_View')
        menu.mouse_click()
        self.keyboard.press_and_release('Down')
        self.keyboard.press_and_release('Enter')

        # Open and close the View menu
        menu.mouse_click()
        menu.mouse_click()

        # Assert that the visibility changed
        self.assertTrue(checked == visible)
        self.assertFalse(toolbar.visible == visible)
        self.assertFalse(toolbar_item.get_state_set().contains(pyatspi.STATE_CHECKED) == checked)

        # Activate View > Toolbar
        menu.mouse_click()
        self.keyboard.press_and_release('Down')
        self.keyboard.press_and_release('Enter')

        # Open and close the View menu
        menu.mouse_click()
        menu.mouse_click()

        # Assert that the visibility is restored
        self.assertTrue(checked == visible)
        self.assertTrue(toolbar.visible == visible)
        self.assertTrue(toolbar_item.get_state_set().contains(pyatspi.STATE_CHECKED) == checked)

    def test_documents_untitled_document(self):
        """Test if radio menu item activation works."""
        self.app = self.launch_test_application('gedit')
        time.sleep(0.2)

        # Open and close the Documents menu
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that Untitled Document 1 is checked
        panel = get_panel_accessible(self.desktop)
        app_menu = get_app_menu_accessible(panel)
        documents_item = app_menu[5]
        documents_menu = documents_item[0]
        untitled_document_1_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))

        # Activate File > New
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_File')
        menu.mouse_click()
        self.keyboard.press_and_release('Down')
        self.keyboard.press_and_release('Enter')

        # Open and close the Documents menu
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that two documents are open
        tabs = self.app.select_many('GeditTab')
        self.assertTrue(len(tabs) == 2)
        self.assertTrue(tabs[0].name == 'Untitled Document 1')
        self.assertTrue(tabs[1].name == 'Untitled Document 2')

        # Assert that Untitled Document 2 is checked
        untitled_document_1_item = documents_menu[-2]
        untitled_document_2_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_2_item.name == 'Untitled Document 2')
        self.assertFalse(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))
        self.assertTrue(untitled_document_2_item.get_state_set().contains(pyatspi.STATE_CHECKED))

        # Activate Documents > Untitled Document 1
        menu.mouse_click()
        self.keyboard.press_and_release('Up')
        self.keyboard.press_and_release('Up')
        self.keyboard.press_and_release('Enter')

        # Open and close the Documents menu
        menu.mouse_click()
        menu.mouse_click()

        # Assert that Untitled Document 1 is checked
        self.assertTrue(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))
        self.assertFalse(untitled_document_2_item.get_state_set().contains(pyatspi.STATE_CHECKED))

    def test_ctrl_n(self):
        """Test if menu item insertion works."""
        self.app = self.launch_test_application('gedit')
        time.sleep(0.2)

        # Open and close the Documents menu
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that Untitled Document 1 is checked
        panel = get_panel_accessible(self.desktop)
        app_menu = get_app_menu_accessible(panel)
        documents_item = app_menu[5]
        documents_menu = documents_item[0]
        untitled_document_1_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))

        # Activate File > New
        self.keyboard.press_and_release('Ctrl+n')

        # Open and close the Documents menu
        panel = self.unity.panels.get_active_panel()
        menu = panel.menus.get_menu_by_label('_Documents')
        menu.mouse_click()
        menu.mouse_click()

        # Assert that two documents are open
        tabs = self.app.select_many('GeditTab')
        self.assertTrue(len(tabs) == 2)
        self.assertTrue(tabs[0].name == 'Untitled Document 1')
        self.assertTrue(tabs[1].name == 'Untitled Document 2')

        # Assert that Untitled Document 2 is checked
        untitled_document_1_item = documents_menu[-2]
        untitled_document_2_item = documents_menu[-1]
        self.assertTrue(untitled_document_1_item.name == 'Untitled Document 1')
        self.assertTrue(untitled_document_2_item.name == 'Untitled Document 2')
        self.assertFalse(untitled_document_1_item.get_state_set().contains(pyatspi.STATE_CHECKED))
        self.assertTrue(untitled_document_2_item.get_state_set().contains(pyatspi.STATE_CHECKED))
