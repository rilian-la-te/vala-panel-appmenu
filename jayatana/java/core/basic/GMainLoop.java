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
package com.jarego.jayatana.basic;

import java.util.logging.Level;
import java.util.logging.Logger;

import com.jarego.jayatana.Feature;

/**
 * Esta clase inicia un GMainLoop para integración con el bus de linux.
 * 
 * @author Jared González
 */
public class GMainLoop implements Feature {
	/**
	 * Inicia el proceso de GMainLoop.
	 */
	native private static void installGMainLoop();
	/**
	 * Detiene el proceso de GMain Loop.
	 */
	native private static void uninstallGMainLoop();
	
	@Override
	public void deploy() {
		// registra el hilo de salida de la aplicación para 
		// detener el GMainLoop
		Runtime.getRuntime().addShutdownHook(new Thread() {
			{
				setDaemon(true);
				setName("JAyatana GMainLoop Shutdown");
			}
			@Override
			public void run() {
				// en caso de que el hilo del menu global este
				// activo esperar el cierre de este.
				if (GlobalMenu.shutdownThread != null)
					try {
						GlobalMenu.shutdownThread.join();
					} catch (InterruptedException e) {
						Logger.getLogger(GMainLoop.class.getName())
							.log(Level.WARNING, "can't wait Global Menu end", e);
					}
				// terminar el GMainLoop
				uninstallGMainLoop();
			}
		});
		// iniciar el GMainLoop
		installGMainLoop();
	}
}
