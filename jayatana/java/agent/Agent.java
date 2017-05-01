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
package com.jarego.jayatana;

import java.io.FileInputStream;
import java.lang.instrument.Instrumentation;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Este agente java permite evaluar si la arquitectura de las librerías nativas
 * de integración son de la misma arquitectura de la máquina virtual. En caso
 * de que conincidan inicia los procesos nativos de integración de lo contrario
 * solo se ignoran
 * 
 * @author Jared González
 */
public class Agent {
	/**
	 * Evalua si la arquitectura de la librerías nativas de Jayatana coninciden con
	 * la arquitectura de la máquia virutal de Java.
	 * 
	 * @param agentArgs argumentos de agente
	 * @param inst Control de la intrumentación de Java
	 */
	public static void premain(String agentArgs, Instrumentation inst) {
		try {
			// excluir versiones 1.5 y 1.4 pues no esta soportadas
			if (System.getProperty("java.version").startsWith("1.5") ||
					System.getProperty("java.version").startsWith("1.4"))
				return;
			
			// obtienen el archivo de la librería nativa de integración
			String libjayatanaag;
			try {
				if ((libjayatanaag = System.getenv("JAYATANA_LIBAGPATH")) == null) {
					libjayatanaag = "/usr/lib/jayatana/libjayatanaag.so";
				} else {
					System.err.println("JAYATANA_LIBAGPATH="+libjayatanaag);
				}
			} catch (Exception e) {
				libjayatanaag = "/usr/lib/jayatana/libjayatanaag.so";
			}
			
			try {
				// obtener arquitectura de la librería
				String libjayatanaagarch = "--";
				FileInputStream fis = new FileInputStream(libjayatanaag);
				try {
					for (int i=0;i<4;i++)
						fis.read();
					if (fis.read() == 2)
						libjayatanaagarch = "64";
					else
						libjayatanaagarch = "32";
				} finally {
					fis.close();
				}
				
				// verificar si la arquitectura conicide con la máquina virtual
				if (libjayatanaagarch.equals(System.getProperty("sun.arch.data.model")))
					System.load(libjayatanaag);
			} catch (Exception e) {}
		} catch (Exception e) {
			Logger.getLogger(Agent.class.getName())
				.log(Level.WARNING, "can't load jayatana agent", e);
		}
	}
}
