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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Feature wrapper class to manage the number of deployments and
 * error handling during this deployment.
 * 
 * @author Jared Gonzalez
 */
public class FeatureWrapper {
	private static List<FeatureWrapper> toFeatureWrapperArray(List<String> classNameList) {
		List<FeatureWrapper> fwList = new ArrayList<FeatureWrapper>();
		for (String cn : classNameList)
			fwList.add(new FeatureWrapper(cn));
		return fwList;
	}
	
	private final List<FeatureWrapper> dependencies;
	private final String className;
	private Feature featrue = null;
	private boolean deployed = false;
	
	/**
         * Creates a feature wrap based on the name of the implemented class of the
         * interface <code> com.jarego.jayatana.Feature </code>.
         *
         * @param className implemented class of the interface <code> com.jarego.jayatana.Feature </code>
	 */
	public FeatureWrapper(String className) {
		this(className, (List<FeatureWrapper>)null);
	}
	public FeatureWrapper(String className, String ...dependencies) {
		this(className, toFeatureWrapperArray(Arrays.asList(dependencies)));
	}
	public FeatureWrapper(String className, FeatureWrapper ...dependencies) {
		this(className, Arrays.asList(dependencies));
	}
	public FeatureWrapper(String className, List<FeatureWrapper> dependencies) {
		this.className = className;
		this.dependencies = dependencies;
	}
	
	/**
         * Gets the feature instance, is instantiated for the first time when invoking the method.
         *
         * @return returns the feature interface
	 * @throws ClassNotFoundException 
	 * @throws IllegalAccessException 
	 * @throws InstantiationException 
	 */
	private Feature getInstance() throws InstantiationException,
			IllegalAccessException, ClassNotFoundException {
		if (featrue == null)
			featrue = (Feature)Class.forName(className).newInstance();
		return featrue;
	}
	
        /**
         * Check if the feature has already been deployed.
         *
         * @return Return <code> True </code> if it has been deployed from
         * opposite returns <code> False </code>.
	 */
	public synchronized boolean isDeployed() {
		return deployed;
	}
	
	/**
         * Perform the one time feature deployment
         *
         * @return Return <code> True </code> if it is deployed for the first time
         * otherwise return <code> False </code>.
	 */
	public synchronized boolean deployOnce() {
		if (!deployed) {
			try {
				boolean ok = true;
				if (dependencies != null) {
					for (FeatureWrapper fm : dependencies) {
						if (!fm.isDeployed())
							fm.deployOnce();
						ok = ok && fm.isDeployed();
						if (!ok)
							throw new Exception("failed dependency: "+fm.className);
					}
				}
				Feature feature;
				if ((feature = getInstance()) != null) {
						feature.deploy();
					deployed = true;
					return true;
				}
			} catch (Exception e) {
				Logger.getLogger(FeatureWrapper.class.getName())
					.log(Level.WARNING, "can't deploy feature: "+className, e);
			}
		}
		return false;
	}
}
