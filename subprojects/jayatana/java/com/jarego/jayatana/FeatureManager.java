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
package com.jarego.jayatana;

import java.util.HashMap;
import java.util.Map;

/**
 * This class declares each of the characteristics and allows to manage them for
 * control the deployments of these. It allows to manage the features are
 * instantiate the classes until they are required.
 *
 * @author Jared Gonzalez
 */
public class FeatureManager {
	/**
         * GMainLoop feature.
	 */
	public static final String FEATURE_GMAINLOOP = "gMainLoop";
	/**
         * Menu edge correction feature for the Look and Feel of
	 * GTK.
	 */
	public static final String FEATURE_SWINGGTKFIX = "swingGtkFix";
	/**
         * Integration feature of global menu with Ubuntu.
	 */
	public static final String FEATURE_SWINGGMENU = "swingGMenu";
	/**
         * Startup Window Manager Class change feature using
         * the ambient varaible JAYATAN_WMCLASS
	 */
	public static final String FEATURE_SWINGWMCLASS = "swingWMClass";
	
	/**
         * Map of features
	 */
	private static Map<String, FeatureWrapper> features = new HashMap<String, FeatureWrapper>();
	
	static {
                // register integration features
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
         * Display all compatible features for Swing.
         */
	public static void deployForSwing() {
		// desplegar carcateristicas de integracion
		deployOnce(FEATURE_SWINGWMCLASS);
		deployOnce(FEATURE_SWINGGTKFIX);
		deployOnce(FEATURE_SWINGGMENU);
	}
	
        /**
         * Deploy a feature only once.
         *
         * @param featureId identifier of the feature.
         * @return Retrona <code> True </code> if the feature is
         * deployed or <code> False </code> if it was already deployed.
         */
	public static boolean deployOnce(String featureId) {
		return features.get(featureId).deployOnce();
	}
        /**
         * Check if a feature has already been deployed.
         *
         * @param featureId identifier of the feature.
         * @return Retrona <code> True </code> if the feature is
         * deployed or <code> False </code> if it was already deployed.
         */
	public static boolean isDeployed(String featureId) {
		return features.get(featureId).isDeployed();
	}
}
