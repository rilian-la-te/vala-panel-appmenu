/*
 * Copyright (c) 2014 Jared Gonzalez
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
package com.jarego.jayatana.swing;

import java.lang.String;
import java.util.Set;
import java.util.HashSet;

import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.AWTEventListener;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.ContainerEvent;
import java.awt.event.ContainerListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.FocusManager;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JSeparator;
import javax.swing.KeyStroke;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

import com.jarego.jayatana.basic.GlobalMenu;
import com.jarego.jayatana.basic.GlobalMenuAdapter;

/**
 * This class allows you to control the menus associated with a Java Swing window.
 *
 * @author Jared Gonzalez
 */
public class SwingGlobalMenuWindow extends GlobalMenuAdapter implements WindowListener,
		AWTEventListener, ContainerListener, PropertyChangeListener, ComponentListener {
	private JMenuBar menubar;
        private Window window;
	private boolean netbeansPlatform;
        private boolean ideaWindow;
        private Set<JMenuItem> approved_checkboxes;
	private boolean fullscreen = false;
        private final String NETBEANS_PLATFORM = "org.openide.awt.MenuBar";
        private final String IDEA_SUBSTRING = "com.intellij";
	
	/**
         * Instance class of menu driver for java Swing windows.
         *
         * @param window window.
         * @param menubar menu bar.
	 */
	public SwingGlobalMenuWindow(Window window, JMenuBar menubar) {
		super(window);
                this.window = window;
		this.menubar = menubar;
                this.approved_checkboxes = new HashSet<JMenuItem>();
	}


        /**
         * Check for MenuBar is Netbeans one.
         *
         * @return is Netbeans menubar
         */
        public boolean checkIsNetbeans() {
            return NETBEANS_PLATFORM.equals(menubar.getClass().getName());
        }

        /**
         * Check for Environment is JetBrains one.
         *
         * @return is JetBrains environment
         */
        public boolean checkIsJetBrains() {
            boolean by_menubar = menubar.getClass().getName().contains(IDEA_SUBSTRING);
            boolean by_window = window.getClass().getName().contains(IDEA_SUBSTRING);
            return by_menubar || by_window;
        }

	/**
         * Register Java Swing menu.
	 */
	@Override
	protected void register(final int state) {
		try {
			EventQueue.invokeAndWait(new Runnable() {
				@Override
				public void run() {
					if (state == GlobalMenu.REGISTER_STATE_INITIAL) {
                                                //set menubar visibility
						menubar.setVisible(false);
						
                                                // Correction for Netbeans
                                                netbeansPlatform = checkIsNetbeans();
                                                ideaWindow = checkIsJetBrains();
						// -----------------------
						
                                                // register listeners of component changes
						for (Component comp : menubar.getComponents()) {
							if (comp instanceof JMenu) {
								((JMenu)comp).addPropertyChangeListener(SwingGlobalMenuWindow.this);
								((JMenu)comp).addComponentListener(SwingGlobalMenuWindow.this);
							}
						}
						menubar.addContainerListener(SwingGlobalMenuWindow.this);
                                                // register keyboard accelerator listeners
						Toolkit.getDefaultToolkit().addAWTEventListener(
								SwingGlobalMenuWindow.this, KeyEvent.KEY_EVENT_MASK);
						((Window)getWindow()).addWindowListener(SwingGlobalMenuWindow.this);
						((Window)getWindow()).addComponentListener(SwingGlobalMenuWindow.this);
					}
					createMenuBarMenus();
				}
			});
		} catch (Exception e) {
			Logger.getLogger(SwingGlobalMenuWindow.class.getName())
					.log(Level.WARNING, e.getMessage(), e);
		}
	}
	/**
         * Deregister listeners, when the global menu bus closes.
         */
	@Override
	protected void unregister() {
                // remove listeners from events
		((Window)getWindow()).removeWindowListener(SwingGlobalMenuWindow.this);
		((Window)getWindow()).addComponentListener(SwingGlobalMenuWindow.this);
		Toolkit.getDefaultToolkit().removeAWTEventListener(SwingGlobalMenuWindow.this);
		for (Component comp : menubar.getComponents()) {
			if (comp instanceof JMenu) {
				((JMenu)comp).removePropertyChangeListener(SwingGlobalMenuWindow.this);
				((JMenu)comp).removeComponentListener(SwingGlobalMenuWindow.this);
			}
		}
		menubar.removeContainerListener(SwingGlobalMenuWindow.this);
                // make menubar visible again
		menubar.setVisible(true);
	}
	
	/**
         * Create level 1 menus, directly to the menu bar.
         */
	private void createMenuBarMenus() {
		for (Component comp : menubar.getComponents()) {
			if (comp instanceof JMenu) {
				addMenu(null, (JMenu)comp);
			}
		}
	}
	
	/**
         * Add submenu.
         *
         * @param parent parent menu
         * @param menu
	 */
	private void addMenu(JMenu parent, JMenu menu) {
		if (parent == null)
			addMenu(menu.hashCode(), menu.getText(), (char)menu.getMnemonic(), menu.isEnabled(), menu.isVisible());
		else
			addMenu(parent.hashCode(), menu.hashCode(), menu.getText(), (char)menu.getMnemonic(), menu.isEnabled(), menu.isVisible());
	}
	
	/**
         * Add menu item
         * @param parent parent menu
         * @param menuitem menu item
	 */
	private void addMenuItem(JMenu parent, JMenuItem menuitem) {
		Dimension size = menuitem.getPreferredSize();
		if (size.height < 2)
			return;
		
		int modifiers = -1;
		int keycode = -1;
		
		if (menuitem.getAccelerator() != null) {
			modifiers = menuitem.getAccelerator().getModifiers();
			keycode = menuitem.getAccelerator().getKeyCode();
		}


                if (menuitem instanceof JRadioButtonMenuItem) {
			addMenuItemRadio(parent.hashCode(), menuitem.hashCode(),
					menuitem.getText(), (char)menuitem.getMnemonic(), menuitem.isEnabled(), modifiers,
					keycode, menuitem.isSelected());
		} else if (menuitem instanceof JCheckBoxMenuItem) {
                        if(ideaWindow && !(menuitem.isSelected()) && !approved_checkboxes.contains(menuitem))
                                addMenuItem(parent.hashCode(), menuitem.hashCode(), menuitem.getText(),
                                                (char)menuitem.getMnemonic(), menuitem.isEnabled(), modifiers, keycode);
                        else {
                                addMenuItemCheck(parent.hashCode(), menuitem.hashCode(),
                                                menuitem.getText(), (char)menuitem.getMnemonic(), menuitem.isEnabled(), modifiers,
                                                keycode, menuitem.isSelected());
                                if(ideaWindow && !approved_checkboxes.contains(menuitem))
                                        approved_checkboxes.add(menuitem);
                        }
		} else {
			addMenuItem(parent.hashCode(), menuitem.hashCode(), menuitem.getText(),
					(char)menuitem.getMnemonic(), menuitem.isEnabled(), modifiers, keycode);
		}
	}
	
	/**
         * Get the menu based on this hashcode.
         *
         * @param hashcode menu identifier.
         * @return Returns the found menu in case it is not found
         * returns <code> NULL </code>.
	 */
	private JMenuItem getJMenuItem(int hashcode) {
		for (Component comp : menubar.getComponents())
			if (comp instanceof JMenuItem) {
				JMenuItem item;
				if ((item = getJMenuItem((JMenuItem) comp, hashcode)) != null)
					return item;
			}
		return null;
	}
	/**
         * Get the menu item based on this hashcode.
         *
         * @param menu parent menu
         * @param hashcode menu identifier.
         * @return Returns the found menuitem in case it is not found
         * returns <code> NULL </code>.
	 */
	private JMenuItem getJMenuItem(JMenuItem menu, int hashcode) {
		if (menu.hashCode() == hashcode) {
			return menu;
		} else if (menu instanceof JMenu) {
			for (Component comp : ((JMenu) menu).getMenuComponents())
				if (comp instanceof JMenuItem) {
					JMenuItem item;
					if ((item = getJMenuItem((JMenuItem) comp, hashcode)) != null)
						return item;
				}
		}
		return null;
	}
	
	/**
         * Activate menu.
	 */
	@Override
	protected void menuActivated(int parentMenuId, int menuId) {
		final JMenuItem menuitem = getJMenuItem(menuId);
		if (menuitem != null && menuitem.isEnabled() && menuitem.isVisible()) {
			EventQueue.invokeLater(new Runnable() {
				@Override
				public void run() {
					menuitem.getModel().setArmed(true);
					menuitem.getModel().setPressed(true);
					menuitem.getModel().setPressed(false);
					menuitem.getModel().setArmed(false);
				}
			});
		}
	}
	
	/**
         * Verify if one class is an instance of another based on
         * Java Refelection
         *
         * @param cls Class to compare.
         * @param scls Class or comparison interface.
         * @return If it is an inherited or implemented instance, it returns
         * <code> True </code> otherwise <code> False </code>.
	 */
	private boolean isInstanceReflection(Class<?> cls, String scls) {
		if ("java.lang.Object".equals(cls.getName())) {
			return false;
		} else if (scls.equals(cls.getName())) {
			return true;
		} else {
			for (Class<?> clsInterface : cls.getInterfaces()) {
				if (scls.equals(clsInterface.getName()))
					return true;
			}
			return isInstanceReflection(cls.getSuperclass(), scls);
		}
	}
	
	/**
         * Invocation of Netbeans platform menus, allows to invoke own constructors of the
         * platform to get the menus.
         *
         * @param menu Netbeans platform menu.
	 */
	private void menuAboutToShowForNetbeansPlatform(JMenu menu) {
		if (isInstanceReflection(menu.getClass(), "org.openide.awt.MenuBar$LazyMenu")) {
			try {
				Method methodDoInitialize = menu.getClass().getDeclaredMethod(
						"doInitialize", new Class<?>[] {});
				if (!methodDoInitialize.isAccessible())
					methodDoInitialize.setAccessible(true);
				
				Field fieldDynaModel = menu.getClass().getDeclaredField("dynaModel");
				if (!fieldDynaModel.isAccessible())
						fieldDynaModel.setAccessible(true);

				Class<?> classDynaModel = fieldDynaModel.getType();
				Method methodDynaModel = classDynaModel.getMethod("checkSubmenu",
						new Class<?>[] {JMenu.class});
				if (!methodDynaModel.isAccessible())
						methodDynaModel.setAccessible(true);

				methodDoInitialize.invoke(menu, new Object[] {});
				Object objectDynaModel = fieldDynaModel.get(menu);
				methodDynaModel.invoke(objectDynaModel, menu);
			} catch (Exception e) {
				Logger.getLogger(SwingGlobalMenuWindow.class.getName())
						.log(Level.SEVERE, e.getMessage(), e);
			}
		}
		
		if (isInstanceReflection(menu.getClass(), "org.openide.awt.DynamicMenuContent")) {
			try {
				Method methodSynchMenu = menu.getClass().getDeclaredMethod(
						"synchMenuPresenters", new Class<?>[] {JComponent[].class});
				if (!methodSynchMenu.isAccessible())
					methodSynchMenu.setAccessible(true);
				methodSynchMenu.invoke(menu, new Object[] {null});
			} catch (Exception e) {
				Logger.getLogger(SwingGlobalMenuWindow.class.getName())
						.log(Level.WARNING, e.getMessage(), e);
			}
		}
	}
	// ----------------------
	
	/**
         * About to show a submenu.
	 */
	@Override
	protected void menuAboutToShow(int parentMenuId, final int menuId) {
		try {
			EventQueue.invokeAndWait(new Runnable() {
				@Override
				public void run() {
					final JMenu menu = (JMenu)getJMenuItem(menuId);
					int items = 0;
					if (menu != null && menu.isEnabled() && menu.isVisible()) {
						menu.getModel().setSelected(true);
						
						JPopupMenu popupMenu = menu.getPopupMenu();
						PopupMenuEvent pevent = new PopupMenuEvent(popupMenu);
						for (PopupMenuListener pl : popupMenu.getPopupMenuListeners())
							if (pl != null) pl.popupMenuWillBecomeVisible(pevent);
						
                                                // Correction for netbeans
						if (netbeansPlatform)
							menuAboutToShowForNetbeansPlatform(menu);
						// -----------------------
						
						for (Component comp : popupMenu.getComponents()) {
							if (comp instanceof JMenu) {
								addMenu(menu, (JMenu)comp);
								items++;
							} else if (comp instanceof JMenuItem && comp.isVisible()) {
								addMenuItem(menu, (JMenuItem)comp);
								items++;
							} else if (comp instanceof JSeparator && comp.isVisible()) {
								addSeparator(menu.hashCode());
								items++;
							}
						}
					}
					if (items == 0 && menu != null)
						addMenuItem(menu.hashCode(), -1, "(...)", (char)0, false, -1, -1);
				}
			});
		} catch (Exception e) {
			Logger.getLogger(SwingGlobalMenuWindow.class.getName())
				.log(Level.SEVERE, e.getMessage(), e);
		}
	}
	/**
         * After closing submenu.
	 */
	@Override
	protected void menuAfterClose(int parentMenuId, final int menuId) {
		try {
			EventQueue.invokeAndWait(new Runnable() {
				@Override
				public void run() {
					final JMenu menu = (JMenu)getJMenuItem(menuId);
					if (menu != null && menu.isEnabled() && menu.isVisible()) {
						JPopupMenu popupMenu = menu.getPopupMenu();
						if (popupMenu != null) {
							PopupMenuEvent pevent = new PopupMenuEvent(popupMenu);
							for (PopupMenuListener pl : popupMenu.getPopupMenuListeners())
								if (pl != null) pl.popupMenuWillBecomeInvisible(pevent);
						}
						menu.getModel().setSelected(false);
					}
				}
			});
		} catch (Exception e) {
			Logger.getLogger(SwingGlobalMenuWindow.class.getName())
				.log(Level.WARNING, e.getMessage(), e);
		}
	}

	@Override
	public void windowOpened(WindowEvent e) {}
	@Override
	public void windowClosing(WindowEvent e) {}
	/**
         * The window is closed.
	 */
	@Override
	public void windowClosed(WindowEvent e) {
		unregisterWatcher();
	}
	@Override
	public void windowIconified(WindowEvent e) {}
	@Override
	public void windowDeiconified(WindowEvent e) {}
	@Override
	public void windowActivated(WindowEvent e) {}
	@Override
	public void windowDeactivated(WindowEvent e) {}
	
	/**
         * Get the window to which a component belongs.
         *
         * @param comp component
         * @return If the component has an associated window, it returns
         * the window otherwise returns <code> NULL </ code>.
	 */
	private Window getWindow(Component comp) {
		if (comp == null)
			return null;
		else if (comp instanceof JFrame)
			return (Window)comp;
		else if (comp instanceof JDialog)
			return (Window)comp;
		else
			return getWindow(comp.getParent());
	}
	
	/**
         * Processing keyboard accelerator events.
	 */
	@Override
	public void eventDispatched(AWTEvent event) {
		KeyEvent e = (KeyEvent) event;
		if (e.getID() == KeyEvent.KEY_PRESSED && !e.isConsumed()) {
			if (e.getKeyCode() != KeyEvent.VK_ALT
					&& e.getKeyCode() != KeyEvent.VK_SHIFT
					&& e.getKeyCode() != KeyEvent.VK_CONTROL
					&& e.getKeyCode() != KeyEvent.VK_META
					&& e.getKeyCode() != KeyEvent.VK_ALT_GRAPH) {
				if (getWindow((Component)e.getSource()) == getWindow()) {
					try {
						KeyStroke acelerator = KeyStroke.getKeyStroke(e.getKeyCode(), e.getModifiers());
						if (FocusManager.getCurrentManager().getFocusOwner() instanceof JComponent) {
							JComponent jcomp = (JComponent)FocusManager.getCurrentManager().getFocusOwner();
							if (jcomp.getActionForKeyStroke(acelerator) == null) {
								Method methodProcessKeyBinding = JMenuBar.class.getDeclaredMethod(
									"processKeyBinding", new Class<?>[] {
										KeyStroke.class, KeyEvent.class, int.class, boolean.class
								});
								if (!methodProcessKeyBinding.isAccessible())
									methodProcessKeyBinding.setAccessible(true);
								Object result = methodProcessKeyBinding.invoke(menubar, new Object[] {
									KeyStroke.getKeyStroke(e.getKeyCode(), e.getModifiers()), e, 	
									JComponent.WHEN_IN_FOCUSED_WINDOW, true
								});
								if (Boolean.TRUE.equals(result))
									e.consume();
							}
						}
					} catch (Exception err) {
						Logger.getLogger(SwingGlobalMenuWindow.class.getName())
								.log(Level.WARNING, err.getMessage(), err);
					}
				}
			}
		}
	}

	/**
         *A new menu is added to the menu bar.
	 */
	@Override
	public void componentAdded(ContainerEvent e) {
		if (e.getChild() instanceof JMenu) {
			((JMenu)e.getChild()).addPropertyChangeListener(this);
			((JMenu)e.getChild()).addComponentListener(this);
			refreshWatcherSafe();
		}
	}
	/**
         * A menu is removed from the menu bar.
	 */
	@Override
	public void componentRemoved(ContainerEvent e) {
		if (e.getChild() instanceof JMenu) {
			((JMenu)e.getChild()).removePropertyChangeListener(this);
			((JMenu)e.getChild()).removeComponentListener(this);
			refreshWatcherSafe();
		}
	}
	/**
         * Change the attribute of a menu in the menu bar.
	 */
	@Override
	public void propertyChange(PropertyChangeEvent evt) {
		if ("enabled".equals(evt.getPropertyName())) {
			JMenu menu = (JMenu)evt.getSource();
			updateMenu(menu.hashCode(), menu.getText(), (char)menu.getMnemonic(), menu.isEnabled(), menu.isVisible());
		}
	}
	/**
         * A menu bar menu is hidden.
	 */
	@Override
	public void componentHidden(ComponentEvent e) {
		if (e.getSource() instanceof JMenu) {
			JMenu menu = (JMenu)e.getSource();
			updateMenu(menu.hashCode(), menu.getText(), (char)menu.getMnemonic(), menu.isEnabled(), menu.isVisible());
		}
	}
	/**
         * A menu bar menu becomes visible.
	 */
	@Override
	public void componentShown(ComponentEvent e) {
		if (e.getSource() instanceof JMenu) {
			JMenu menu = (JMenu)e.getSource();
			updateMenu(menu.hashCode(), menu.getText(), (char)menu.getMnemonic(), menu.isEnabled(), menu.isVisible());
		} else if (e.getSource() instanceof Window) {
			refreshWatcherSafe();
		}
	}
	@Override
	public void componentMoved(ComponentEvent e) {}
	@Override
	public void componentResized(ComponentEvent e) {
		if (e.getSource() instanceof Window) {
			Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
			Point rootPaneLoc = null;
			Dimension rootPaneSize = null;
			if (getWindow() instanceof JFrame) {
				rootPaneLoc = ((JFrame)getWindow()).getRootPane().getLocationOnScreen();
				rootPaneSize = ((JFrame)getWindow()).getRootPane().getSize();
			} else if (getWindow() instanceof JDialog) {
				rootPaneLoc = ((JDialog)getWindow()).getRootPane().getLocationOnScreen();
				rootPaneSize = ((JDialog)getWindow()).getRootPane().getSize();
			}
			if (rootPaneLoc != null && rootPaneSize != null) {
				if (rootPaneSize.height >= screenSize.height &&
						rootPaneSize.width >= screenSize.width &&
						rootPaneLoc.x == 0 && rootPaneLoc.y == 0 &&
						!fullscreen) {
					fullscreen = true;
					unregisterWatcher();
					for (Component comp : menubar.getComponents())
						if (comp instanceof JMenu)
							((JMenu)comp).getModel().setSelected(false);
				} else if ((rootPaneSize.height < screenSize.height ||
						rootPaneSize.width < screenSize.width) &&
						rootPaneLoc.x >= 0 && rootPaneLoc.y >= 0 &&
						fullscreen) {
					fullscreen = false;
					registerWatcher();
				}
			}
		}
	}
}
