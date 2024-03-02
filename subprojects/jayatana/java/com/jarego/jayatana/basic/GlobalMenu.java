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
package com.jarego.jayatana.basic;

import java.awt.Window;

/**
 * Integration class of Global global menu based on DBUS, allows to interact with the
 * Ubuntu global menu with native invocations.
 *
 * @author Jared Gonzalez
 */
public abstract class GlobalMenu {
	/**
          * The menu was registered for the first time.
	 */
	public static final int REGISTER_STATE_INITIAL = 0;
	/**
          * The menu was updated by a change in the structure of the menu bar.
	 */
	public static final int REGISTER_STATE_REFRESH = 1;
	
	/**
          * Initialize the native structures for integration with the global menu.
	 */
	native private static void initialize();
	/**
          * Destroy the native structures for integration with the global menu.
	 */
	native private static void uninitialize();
	/**
          * Thread of the closing control of the application.
	 */
	public static Thread shutdownThread = null;
	
	/**
          * Initializes the native structures in addition to registering the application closing thread
          * to destroy the native structures.
	 */
	public static void nativeInitialize() {
		Runtime.getRuntime().addShutdownHook(shutdownThread = new Thread() {
			{
				setName("JAyatana GlobalMenu Shutdown");
			}
			@Override
			public void run() {
				GlobalMenu.uninitialize();
			}
		});
		GlobalMenu.initialize();
	}
	
        /**
          * Gets the window handle of a Java Window class.
          *
          * @param window object window
          * @return Returns the window handle
          */
	native public static long getWindowXID(Window window);
	
        /**
          * Registers global menu bus viewer. In case the bus
          * exists the <code>register</code> method will be invoked.
          *
          * @param windowXID window identifier
          */
	native synchronized public void registerWatcher(long windowXID);
        /**
          * Removes the global menu bus viewer. In case the bus
          * existing, the <code> unregister </code> method will be invoked.
          *
          * @param windowXID window identifier
          */
	native synchronized public void unregisterWatcher(long windowXID);
        /**
          * This method regenerates the Bus viewer, and should be used if some
          * menu (level 0) added directly to the menu bar changes,
          * since Ubuntu has problems with the methods of adding or deleting these.
          *
          * @param windowXID window identifier
          */
	native synchronized public void refreshWatcher(long windowXID);
	
        /**
          * This method is invoked when the global menu bus is registered, if the global menu
          * is not or is excluded by some other method such as the environment variable UBUNTU_MENUPROXY
          * this method will never be invoked.
          *
          * @param state registration status, when it is the first time returns the value of
          * <code> REGISTER_STATE_INITIAL </code> if you are registering for a refresh returns the
          * value of <code> REGISTER_STATE_REFRESH </code>.
          */
	abstract protected void register(int state);
        /**
          * This method is invoked when the bus viewer is deleted or if the bus is closed
          * externally
          */
	abstract protected void unregister();
	
        /**
         * Add a new native menu.
         *
         * @param windowXID window identifier.
         * @param menuParentId identifier of the parent menu, to specify a menu directly in the bar
         * from menu the parent identifier must be <code> -1 </code>.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param visible visibulity status of the menu.
         */
        native public void addMenu (long windowXID, int menuParentId, int menuId,
                                    String label, boolean enabled, boolean visible);
        /**
         * Add a native menu item.
         *
         * @param windowXID window identifier.
         * @param menuParentId identifier of the parent menu.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param modifiers throttle modifier menu (CTRL, ALT or SHIFT).
         * @param keycode accelerator menu.
         */
        native public void addMenuItem (long windowXID, int menuParentId, int menuId,
                                        String label, boolean enabled, int modifiers, int keycode);
        /**
         * Add a native radio menu item.
         *
         * @param windowXID window identifier.
         * @param menuParentId identifier of the parent menu.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param modifiers throttle modifier menu (CTRL, ALT or SHIFT).
         * @param keycode accelerator menu.
         * @param selected menu selection status.
         */
        native public void addMenuItemRadio (long windowXID, int menuParentId, int menuId,
                                             String label, boolean enabled, int modifiers, int keycode, boolean selected);
        /**
         * Add a native check menu item.
         *
         * @param windowXID window identifier.
         * @param menuParentId identifier of the parent menu.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param modifiers throttle modifier menu (CTRL, ALT or SHIFT).
         * @param keycode accelerator menu.
         * @param selected menu selection status.
         */
        native public void addMenuItemCheck (long windowXID, int menuParentId, int menuId,
                                             String label, boolean enabled, int modifiers, int keycode, boolean selected);
        /**
         * Add a menu item of native separator.
         *
         * @param windowXID window identifier.
         * @param menuParentId identifier of the parent menu.
         */
        native public void addSeparator (long windowXID, int menuParentId);
        /**
         * Status update of the native menu.
         *
         * @param windowXID window identifier
         * @param menuId menu identifier
         * @param label new tag value
         * @param enabled new status value of menu enable.
         * @param visible new visibility status value of the menu.
         */
        native public void updateMenu (long windowXID, int menuId, String label,
                                       boolean enabled, boolean visible);

        /**
         * This method is invoked when a native menu is selected.
         *
         * @param parentMenuId identifier of the menu parent.
         * @param menuId identifier of the selected menu.
         */
        abstract protected void menuActivated (int parentMenuId, int menuId);
        /**
         * This method is invoked when a menu is opened.
         *
         * @param parentMenuId identifier of the parent of the menu, if the menu is
         * directly to the menu bar the value of the parent is <code> -1 </code>
         * @param menuId menu identifier.
         */
        abstract protected void menuAboutToShow (int parentMenuId, int menuId);
        /**
         * This method is invoked when a menu is closed.
         *
         * @param parentMenuId identifier of the parent of the menu, if the menu is
         * directly to the menu bar the value of the parent is <code> -1 </code>
         * @param menuId menu identifier.
         */
        abstract protected void menuAfterClose (int parentMenuId, int menuId);
}
