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

import java.util.HashMap;
import java.util.Map;

/**
 * Esta clase declara cada una de las características y permite gestionarlas para
 * controlar los desplieges de estas. Permite gestionar las características son
 * instanciar las clases hasta que estas son requeridas.
 * 
 * @author Jared González
 */
public class FeatureManager {
	/**
	 * Característica de GMainLoop.
	 */
	public static final String FEATURE_GMAINLOOP = "gMainLoop";
	/**
	 * Característica de correción de bordes de menus para el LookAndFeel de
	 * GTK.
	 */
	public static final String FEATURE_SWINGGTKFIX = "swingGtkFix";
	/**
	 * Característica de integración de menu global con Ubuntu.
	 */
	public static final String FEATURE_SWINGGMENU = "swingGMenu";
	/**
	 * Característica de cambio de Startup Window Manager Class usando
	 * la varaible de ambiente JAYATAN_WMCLASS
	 */
	public static final String FEATURE_SWINGWMCLASS = "swingWMClass";
	
	/**
	 * Tabla hash de características
	 */
	private static Map<String, FeatureWrapper> features = new HashMap<String, FeatureWrapper>();
	
	static {
		// registrar carcateristicas de integración
		FeatureWrapper basicNativeLibraries = new FeatureWrapper(
				"com.jarego.jayatana.basic.NativeLibraries");
		
		features.put(FEATURE_GMAINLOOP,
				new FeatureWrapper("com.jarego.jayatana.basic.GMainLoop",
						basicNativeLibraries));
		features.put(FEATURE_SWINGGTKFIX,
				new FeatureWrapper("com.jarego.jayatana.swing.SwingGTKFixed"));
		features.put(FEATURE_SWINGGMENU,
				new FeatureWrapper("com.jarego.jayatana.swing.SwingGlobalMenu",
						basicNativeLibraries));
		features.put(FEATURE_SWINGWMCLASS,
				new FeatureWrapper("com.jarego.jayatana.swing.SwingWMClass"));
		features.put(FEATURE_SWINGWMCLASS,
				new FeatureWrapper("com.jarego.jayatana.swing.SwingWMClass"));
	}
	
	/**
	 * Despliega todas características compatibles para Swing.
	 */
	public static void deployForSwing() {
		// desplegar carcateristicas de integración
		deployOnce(FEATURE_SWINGWMCLASS);
		deployOnce(FEATURE_SWINGGTKFIX);
		deployOnce(FEATURE_SWINGGMENU);
	}
	
	/**
	 * Despliege una característica solo una vez.
	 * 
	 * @param featureId identificador de la característica.
	 * @return Retrona <code>True</code> si la característica es
	 * desplegada o <code>False</code> si ya estaba desplegada.
	 */
	public static boolean deployOnce(String featureId) {
		return features.get(featureId).deployOnce();
	}
	/**
	 * Verifica si una característica ya ha sido desplegada.
	 * 
	 * @param featureId identificador de la característica.
	 * @return Retrona <code>True</code> si la característica es
	 * desplegada o <code>False</code> si ya estaba desplegada.
	 */
	public static boolean isDeployed(String featureId) {
		return features.get(featureId).isDeployed();
	}
}
