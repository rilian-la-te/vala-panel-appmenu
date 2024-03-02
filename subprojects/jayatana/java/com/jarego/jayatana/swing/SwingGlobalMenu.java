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

import java.awt.AWTEvent;
import java.awt.Dialog.ModalityType;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.AWTEventListener;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;

import com.jarego.jayatana.Feature;
import com.jarego.jayatana.FeatureManager;
import com.jarego.jayatana.basic.GlobalMenu;

/**
 * Feature class to deploy the integration with the global menu
 * from Ubuntu for Java Swing applications.
 *
 * @author Jared Gonzalez
 */
public class SwingGlobalMenu implements Feature, AWTEventListener {
	/**
         * Start feature deployment for global menu integration
         * from Ubuntu.
	 */
	@Override
	public void deploy() {
		GlobalMenu.nativeInitialize();
		Toolkit.getDefaultToolkit().addAWTEventListener(
				SwingGlobalMenu.this, AWTEvent.WINDOW_EVENT_MASK);
	}
	
	/**
         * Listen to window opening event to link integration
         * for each new window launched.
	 */
	@Override
	public void eventDispatched(AWTEvent event) {
		if (event.getID() == WindowEvent.WINDOW_OPENED) {
			if (event.getSource() instanceof JFrame)
				installOnWindow((JFrame)event.getSource());
			else if (event.getSource() instanceof JDialog)
				installOnWindow((JDialog)event.getSource());
		}
	}
	
	/**
         * Install over <code> JFrame </code> window.
         *
         * @param jframe window
	 */
	protected void installOnWindow(JFrame jframe) {
		JMenuBar menubar;
		if ((menubar = retriveMenuBar(jframe)) != null)
			tryInstallGlobalMenu(jframe, menubar);
	}
        /**
         * Install over <code> JDialog </code> window.
         *
         * @param jframe window
         */
	protected void installOnWindow(JDialog jdialog) {
		JMenuBar menubar;
		if ((menubar = retriveMenuBar(jdialog)) != null) {
			tryInstallGlobalMenu(jdialog, menubar);
		} else {
			if (jdialog.getModalityType() == ModalityType.APPLICATION_MODAL) {
				installLockParentGlobalMenu(jdialog.getOwner(), jdialog);
			}
		}
	}
	
	/**
         * Retrieve menu bar from the window.
         *
         * @param jframe window.
         * @return Menu bar.
	 */
	protected JMenuBar retriveMenuBar(JFrame jframe) {
		JMenuBar menuBar = null;
		if (jframe.getRootPane().getClientProperty("jayatana.menubar") instanceof JMenuBar)
			menuBar = (JMenuBar)jframe.getRootPane().getClientProperty("jayatana.menubar");
		else
			menuBar = jframe.getJMenuBar();
		return menuBar;
	}
	/**
         * Retrieve menu bar from the window.
         *
         * @param jdialog window.
         * @return Menu bar.
	 */
	protected JMenuBar retriveMenuBar(JDialog jdialog) {
		JMenuBar menuBar = null;
		if (jdialog.getRootPane().getClientProperty("jayatana.menubar") instanceof JMenuBar)
			menuBar = (JMenuBar)jdialog.getRootPane().getClientProperty("jayatana.menubar");
		else
			menuBar = jdialog.getJMenuBar();
		return menuBar;
	}
	
	/**
         * Try to install as long as the bus is running on the global menu.
         *
         * @param window window.
         * @param menubar menu bar.
	 */
	private void tryInstallGlobalMenu(Window window, JMenuBar menubar) {
		FeatureManager.deployOnce(FeatureManager.FEATURE_GMAINLOOP);
		SwingGlobalMenuWindow swingGlobalMenuWindow = new SwingGlobalMenuWindow(window, menubar);
		setSwingGlobalMenuWindowController(window, swingGlobalMenuWindow);
		swingGlobalMenuWindow.registerWatcher();
	}
	
	/**
         * Install the menu lock on the parent window.
         *
         * @param parent Parent window
         * @param child child window
	 */
	private void installLockParentGlobalMenu(Window parent, final Window child) {
		if (parent != null) {
			List<SwingGlobalMenuWindow> swingGlobalMenuWindowList = new ArrayList<SwingGlobalMenuWindow>();
			if (parent == JOptionPane.getRootFrame()) {
				for (Window w : Window.getOwnerlessWindows()) {
					SwingGlobalMenuWindow swingGlobalMenuWindow = getSwingGlobalMenuWindowController(w);
					if (swingGlobalMenuWindow != null) {
						swingGlobalMenuWindow.lockMenuBar();
						swingGlobalMenuWindowList.add(swingGlobalMenuWindow);
					}
				}
			} else {
				SwingGlobalMenuWindow swingGlobalMenuWindow = getSwingGlobalMenuWindowController(parent);
				if (swingGlobalMenuWindow != null) {
					swingGlobalMenuWindow.lockMenuBar();
					swingGlobalMenuWindowList.add(swingGlobalMenuWindow);
				}
			}
			ApplicationModalWindowListener listener =
					new ApplicationModalWindowListener(swingGlobalMenuWindowList);
			child.addWindowListener(listener);
			child.addComponentListener(listener);
		}
	}
	
	/**
         * Set the menu controller on the window.
         *
         * @param window window
         * @param sgm menu controller
	 */
	private void setSwingGlobalMenuWindowController(Window window, SwingGlobalMenuWindow sgm) {
		if (window instanceof JFrame)
			((JFrame)window)
				.getRootPane().putClientProperty("jayatana.globalmenu.controller", sgm);
		else if (window instanceof JDialog)
			((JDialog)window)
				.getRootPane().putClientProperty("jayatana.globalmenu.controller", sgm);
	}
	
        /**
         * Get the menu controller on the window.
         *
         * @param window window
         * @return menu controller
         */
	private SwingGlobalMenuWindow getSwingGlobalMenuWindowController(Window window) {
		SwingGlobalMenuWindow swingGlobalMenuWindow = null;
		if (window instanceof JFrame)
			swingGlobalMenuWindow = (SwingGlobalMenuWindow)((JFrame)window)
				.getRootPane().getClientProperty("jayatana.globalmenu.controller");
		else if (window instanceof JDialog)
			swingGlobalMenuWindow = (SwingGlobalMenuWindow)((JDialog)window)
				.getRootPane().getClientProperty("jayatana.globalmenu.controller");
		return swingGlobalMenuWindow;
	}
	
	/**
         * Class to release the menu block once the MODAL window is closed.
         *
	 * @author Jared Gonzalez
	 */
	private class ApplicationModalWindowListener implements WindowListener, ComponentListener {
		boolean locked;
		private List<SwingGlobalMenuWindow> swingGlobalMenuWindowList;
		
		public ApplicationModalWindowListener(List<SwingGlobalMenuWindow> swingGlobalMenuWindowList) {
			this.swingGlobalMenuWindowList = swingGlobalMenuWindowList;
			locked = true;
		}
		@Override
		public void componentHidden(ComponentEvent e) {
			if (locked) {
				for (SwingGlobalMenuWindow sgw : swingGlobalMenuWindowList)
					sgw.unlockMenuBar();
				locked = false;
			}
		}
		@Override
		public void componentShown(ComponentEvent e) {
			if (!locked) {
				for (SwingGlobalMenuWindow sgw : swingGlobalMenuWindowList)
					sgw.lockMenuBar();
				locked = true;
			}
		}
		@Override
		public void componentMoved(ComponentEvent e) {}
		@Override
		public void componentResized(ComponentEvent e) {}
		
		@Override
		public void windowActivated(WindowEvent e) {}
		@Override
		public void windowClosing(WindowEvent e) {}
		@Override
		public void windowDeactivated(WindowEvent e) {}
		@Override
		public void windowDeiconified(WindowEvent e) {}
		@Override
		public void windowIconified(WindowEvent e) {}
		@Override
		public void windowOpened(WindowEvent e) {}
		@Override
		public void windowClosed(WindowEvent e) {
			if (locked) {
				for (SwingGlobalMenuWindow sgw : swingGlobalMenuWindowList)
					sgw.unlockMenuBar();
				locked = false;
			}
			e.getWindow().removeWindowListener(this);
			e.getWindow().removeComponentListener(this);
		}
	}
}
