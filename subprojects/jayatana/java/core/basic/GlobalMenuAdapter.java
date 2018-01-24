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
import java.util.logging.Level;
import java.util.logging.Logger;

import com.jarego.jayatana.swing.SwingGlobalMenuWindow;

/**
 * Global Menu adapter class that allows to encapsulate the window driver
 * along with the native menu controls.
 * 
 * @author Jared Gonzalez
 */
public abstract class GlobalMenuAdapter {
	private static final int SPINCOUNT = 200;
	private final GlobalMenuImp globalMenuImp;
	/**
        * Menu bar lock feature variable
        */
	private boolean lockedMenuBar = false;
	
	/**
         * Variable of specification of delay for construction wait
         * of menus.
	 */
	protected long approveRefreshWatcher = -1;
	/**
         * Window associated with the global menu
         */
        private Object window;
	/**
         * Window ID
	 */
	private long windowXID;
	
	/**
         * Start global menu controller based on a window object.
         *
         * @param window window
	 */
	public GlobalMenuAdapter(Window window) {
		this(window, GlobalMenu.getWindowXID((Window)window));
	}
	
	public GlobalMenuAdapter(Object window, long windowXID) {
		this.window = window;
		this.windowXID = windowXID;
		this.globalMenuImp = new GlobalMenuImp(this);
	}
	
	/**
         * Registers global menu bus viewer. In case the bus
         * exists the <code> register </code> method will be invoked.
	 */
	public void registerWatcher() {
		globalMenuImp.registerWatcher(windowXID);
	}
	/**
         * Removes the global menu bus viewer. In case the bus
         * existing, the <code> unregister </ code> method will be invoked.
	 */
	protected void unregisterWatcher() {
		globalMenuImp.unregisterWatcher(windowXID);
	}
	/**
         * This method regenerates the Bus viewer, and should be used if some
         * menu (level 0) added directly to the menu bar changes,
         * since Ubuntu has problems with the methods of adding or deleting these.
	 */
	protected void refreshWatcher() {
		globalMenuImp.refreshWatcher(windowXID);
	}
	
	/**
         * Add a new native submenu on the menu bar.
         *
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param visible visibulity status of the menu.
	 */
	protected void addMenu(int menuId, String label, char mnemonic, boolean enabled, boolean visible) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.addMenu(windowXID, -1, menuId, formatLabelString(label, mnemonic), lockedMenuBar ? false : enabled, visible);
	}
	/**
         * Add a new native submenu.
         *
         * @param menuParentId identifier of the parent menu, to specify a menu directly in the bar
         * from menu the parent identifier must be <code> -1 </code>.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param visible visibulity status of the menu.
	 */
	protected void addMenu(int menuParentId, int menuId, String label, char mnemonic, boolean enabled,
			boolean visible) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.addMenu(windowXID, menuParentId, menuId, formatLabelString(label, mnemonic), enabled, visible);
	}
	/**
         * Add a native menu item.
         *
         * @param menuParentId identifier of the parent menu.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param modifiers throttle modifier menu (CTRL, ALT or SHIFT).
         * @param keycode accelerator menu.
	 */
	protected void addMenuItem(int menuParentId, int menuId, String label, char mnemonic, boolean enabled,
			int modifiers, int keycode) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.addMenuItem(windowXID, menuParentId, menuId, formatLabelString(label, mnemonic), enabled, modifiers, keycode);
	}
        /**
         * Add a native check menu item.
         *
         * @param menuParentId identifier of the parent menu.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param modifiers throttle modifier menu (CTRL, ALT or SHIFT).
         * @param keycode accelerator menu.
         * @param selected menu selection status.
	 */
	protected void addMenuItemCheck(int menuParentId, int menuId, String label, char mnemonic, boolean enabled,
			int modifiers, int keycode, boolean selected) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.addMenuItemCheck(windowXID, menuParentId, menuId, formatLabelString(label, mnemonic), enabled,modifiers, keycode, selected);
	}
	/**
         * Add a native radio menu item.
         *
         * @param menuParentId identifier of the parent menu.
         * @param menuId menu identifier.
         * @param label menu label.
         * @param enabled menu enable status.
         * @param modifiers throttle modifier menu (CTRL, ALT or SHIFT).
         * @param keycode accelerator menu.
         * @param selected menu selection status.
	 */
	protected void addMenuItemRadio(int menuParentId, int menuId, String label, char mnemonic, boolean enabled,
			int modifiers, int keycode, boolean selected) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.addMenuItemRadio(windowXID, menuParentId, menuId, formatLabelString(label, mnemonic), enabled, modifiers, keycode, selected);
	}
	/**
         * Add a menu item of native separator.
         *
         * @param menuParentId identifier of the parent menu.
	 */
	protected void addSeparator(int menuParentId) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.addSeparator(windowXID, menuParentId);
	}
	/**
         * Status update of the native menu.
         *
         * @param menuId menu identifier
         * @param label new tag value
         * @param enabled new status value of menu enable.
         * @param visible new visibility status value of the menu.
	 */
	protected void updateMenu(int menuId, String label, char mnemonics, boolean enabled, boolean visible) {
		if (approveRefreshWatcher != -1)
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		globalMenuImp.updateMenu(windowXID, menuId, formatLabelString(label, mnemonics), enabled, visible);
	}
	
	/**
         * Get the window.
	 * 
         * @return window object.
	 */
	protected Object getWindow() {
		return window;
	}
	/**
         * Get the window identifier.
         *
         * @return window ID.
	 */
	protected long getWindowXID() {
		return windowXID;
	}
	
	/**
         * Format the text string to prevent errors by null and include mnemonics of
         * acceleration.
         *
         * @param text label text
         * @return text
	 */
	private static String formatLabelString(String text, char mnemonic) {
		if (text == null)
			return "";
		else {
			if (mnemonic > 0) {
				String output = "";
				boolean founded = false;
				for (char chr : text.toCharArray()) {
					if (!founded &&
							Character.toUpperCase(mnemonic) == Character.toUpperCase(chr)) {
						output += '_';
						founded = true;
					}
					output += chr;
				}
				return output;
			} else {
				return text;
			}
		}
	}
	
	/**
         * Regenerates menus directly in the menu bar.
	 */
	protected void refreshWatcherSafe() {
		if (approveRefreshWatcher == -1) {
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
			new Thread() {
				@Override
				public void run() {
					try {
						while (System.currentTimeMillis() < approveRefreshWatcher)
							Thread.sleep(100);
					} catch (InterruptedException e) {
						Logger.getLogger(SwingGlobalMenuWindow.class.getName()).log(
								Level.WARNING, "Can't wait approve rebuild", e);
					} finally {
						approveRefreshWatcher = -1;
						refreshWatcher();
					}
				}
			}.start();
		} else {
			approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
		}
	}
	
	/**
         * Lock the menu bar.
	 */
	public void lockMenuBar() {
		if (!lockedMenuBar) {
			lockedMenuBar = true;
			refreshWatcherSafe();
		}
	}
	
	/**
         * Unlock the menu bar.
	 */
	public void unlockMenuBar() {
		if (lockedMenuBar) {
			lockedMenuBar = false;
			refreshWatcherSafe();
		}
	}
	
	/**
         * Event log when it joins the Ubuntu Menus controller.
	 * 
	 * @param state <code>GlobalMenu.REGISTER_STATE_INITIAL</code> y
	 * <code>GlobalMenu.REGISTER_STATE_REFRESH</code>
	 */
	abstract protected void register(int state);
	/**
         * Event log when the connection to the Ubuntu Menus controller is terminated.
	 */
	abstract protected void unregister();
	/**
         * Event record when a menu is selected.
	 * 
         * @param parentMenuId Parent of the menu.
         * @param menuId Menu identifier.
	 */
	abstract protected void menuActivated(int parentMenuId, int menuId);
	/**
         * Event record when a menu is being selected before
         * show
	 * 
         * @param parentMenuId Parent of the menu.
         * @param menuId Menu identifier.
	 */
	abstract protected void menuAboutToShow(int parentMenuId, int menuId);
	/**
         * Event record when a menu is closed.
	 * 
         * @param parentMenuId Parent of the menu.
         * @param menuId Menu identifier.
	 */
	abstract protected void menuAfterClose(int parentMenuId, int menuId);
	
	/**
         * Menu adapter container to prevent access to internal methods
	 * 
	 * @author Jared Gonzalez
	 */
	private static class GlobalMenuImp extends GlobalMenu {
		private final GlobalMenuAdapter globalMenuAdapter;
		
		public GlobalMenuImp(GlobalMenuAdapter globalMenuAdapter) {
			this.globalMenuAdapter = globalMenuAdapter;
		}
		@Override
		protected void register(int state) {
			globalMenuAdapter.register(state);
		}
		@Override
		protected void unregister() {
			globalMenuAdapter.unregister();
		}
		@Override
		protected void menuActivated(int parentMenuId, int menuId) {
			globalMenuAdapter.menuActivated(parentMenuId, menuId);
		}
		@Override
		protected void menuAboutToShow(int parentMenuId, int menuId) {
			if (globalMenuAdapter.approveRefreshWatcher != -1)
				globalMenuAdapter.approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
			globalMenuAdapter.menuAboutToShow(parentMenuId, menuId);
		}
		@Override
		protected void menuAfterClose(int parentMenuId, int menuId) {
			if (globalMenuAdapter.approveRefreshWatcher != -1)
				globalMenuAdapter.approveRefreshWatcher = System.currentTimeMillis() + SPINCOUNT;
			globalMenuAdapter.menuAfterClose(parentMenuId, menuId);
		}
	}
}
