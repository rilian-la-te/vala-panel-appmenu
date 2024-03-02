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

import java.awt.Toolkit;
import java.lang.reflect.Field;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.jarego.jayatana.Feature;

/**
 * This class is a feature to modify the value of the STARTUP WINDOW MANAGER CLASS
 * which can be integrated into a * .desktop launcher file for integration with the
 * Ubuntu menu bar.
 *
 * This value can be modified with the virtual machine parameter:
 * jayatana.wmclass
 * Or with the environment variable:
 * JAYATANA_WMCLASS
 *
 * This value can be used in conjunction with the StartupWMClass parameter in the .desktop file
 * To allow to integrate with the Unity launch bar.
 *
 * @author Jared Gonzalez.
 */
public class SwingWMClass implements Feature {
	/**
         * Feature display to allow integration with the Unity launcher.
	 */
	@Override
	public void deploy() {
		String startupWMClass = null;
		
		if (System.getProperty("jayatana.startupWMClass") != null)
			startupWMClass = System.getProperty("jayatana.startupWMClass");
		else if (System.getProperty("jayatana.wmclass") != null)
			startupWMClass = System.getProperty("jayatana.wmclass");
		
		else if (System.getProperty("JAYATANA_STARTUPWMCLASS") != null)
			startupWMClass = System.getProperty("JAYATANA_STARTUPWMCLASS");
		else if (System.getProperty("JAYATANA_WMCLASS") != null)
			startupWMClass = System.getProperty("JAYATANA_WMCLASS");
		
		if (startupWMClass != null) {
			try {
				System.setProperty("java.awt.WM_CLASS", startupWMClass);
				Toolkit xToolkit = Toolkit.getDefaultToolkit();
				Field awtAppClassNameField = xToolkit.getClass()
						.getDeclaredField("awtAppClassName");
				awtAppClassNameField.setAccessible(true);
				awtAppClassNameField.set(xToolkit, startupWMClass);
				awtAppClassNameField.setAccessible(false);
			} catch (Exception e) {
				Logger.getLogger(SwingWMClass.class.getName())
						.log(Level.WARNING, e.getMessage(), e);
			}
		}
	}
}
