/*
 * Copyright (c) 2014 Jared González
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
 * Esta clase es un característica para modificar el valor del STARTUP WINDOW MANAGER CLASS
 * el cual puede permite integrarse a un archivo lanzador *.desktop para integración con la
 * barra de menús de Ubuntu.
 * 
 * Este valor se puede modificar con el parámetro de máquina virtal:
 * 		jayatana.wmclass
 * O con la variable de ambiente:
 * 		JAYATANA_WMCLASS
 * 
 * Este valor puede usarse en convinación con el parámetro StartupWMClass del archivo .desktop
 * Para permitir integrar con la barra de lanzamiento unity.
 * 
 * @author Jared González.
 */
public class SwingWMClass implements Feature {
	/**
	 * Despliege de característica para permitir integración con el lanzador de Unity.
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
