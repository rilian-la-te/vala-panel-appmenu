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

import java.util.logging.Level;
import java.util.logging.Logger;

import com.jarego.jayatana.Feature;

/**
 * This class starts a GMainLoop for integration with the linux bus.
 *
 * @author Jared Gonzalez
 */
public class GMainLoop implements Feature {
	/**
         * Start the GMainLoop process.
	 */
	native private static void installGMainLoop();
	/**
         * Stops the GMain Loop process.
	 */
	native private static void uninstallGMainLoop();
	
	@Override
	public void deploy() {
                // register the output thread of the application for
                // stop the GMainLoop
		Runtime.getRuntime().addShutdownHook(new Thread() {
			{
				setDaemon(true);
				setName("JAyatana GMainLoop Shutdown");
			}
			@Override
			public void run() {
                                // in case the global menu thread is
                                // active wait for the closure of this.
				if (GlobalMenu.shutdownThread != null)
					try {
						GlobalMenu.shutdownThread.join();
					} catch (InterruptedException e) {
						Logger.getLogger(GMainLoop.class.getName())
							.log(Level.WARNING, "can't wait Global Menu end", e);
					}
                                // finish the GMainLoop
                                uninstallGMainLoop();
			}
		});
                // start the GMainLoop
                installGMainLoop();
	}
}
