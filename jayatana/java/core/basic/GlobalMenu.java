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

import java.awt.Window;

/**
 * Clase de integración de menu global Global basado en DBUS, permite interactuar con el
 * menu global de Ubuntu con invocaciones nativas.
 * 
 * @author Jared González
 */
public abstract class GlobalMenu {
	/**
	 * El menu se registro por primera vez.
	 */
	public static final int REGISTER_STATE_INITIAL = 0;
	/**
	 * El menu se actualizo por un cambio en la estructura de la barra de menu.
	 */
	public static final int REGISTER_STATE_REFRESH = 1;
	
	/**
	 * Inicializa el las estructuras nativas para la integración con el menu global.
	 */
	native private static void initialize();
	/**
	 * Destruye las estructuras nativas para la integración con el menu global.
	 */
	native private static void uninitialize();
	/**
	 * Hilo del control de cierre de la aplicación.
	 */
	public static Thread shutdownThread = null;
	
	/**
	 * Inicializa las estructuras nativas además de registrar el hilo de cierre de aplicación
	 * para destruir las estructuras nativas.
	 */
	public static void nativeInitialize() {
		Runtime.getRuntime().addShutdownHook(shutdownThread = new Thread() {
			{
				setName("JAyatana GlobalMenu Shutdown");
			}
			@Override
			public void run() {
				GlobalMenu.uninitialize();
			}
		});
		GlobalMenu.initialize();
	}
	
	/**
	 * Obtiene el identificador de ventana de una clase Window de Java.
	 * 
	 * @param window objecto ventana
	 * @return Regresa el identificador de ventana
	 */
	native public static long getWindowXID(Window window);
	
	/**
	 * Registra visualizador de bus de menu global. En caso de que el bus
	 * exista se invocará el método <code>register</code>.
	 * 
	 * @param windowXID identificador de ventana
	 */
	native synchronized public void registerWatcher(long windowXID);
	/**
	 * Elimina el visualizador de bus de menu global. En cas de que el bus
	 * exista se invocará el método <code>unregister</code>.
	 * 
	 * @param windowXID identificador de ventana
	 */
	native synchronized public void unregisterWatcher(long windowXID);
	/**
	 * Este método regenera el visualizador de Bus, y debe ser usado si algun
	 * menu (de nivel 0) agregado directamente a la barra de menus cambia,
	 * puesto que Ubuntu tiene problemas con los métodos de agregar o eliminar estos.
	 * 
	 * @param windowXID identificador de ventana
	 */
	native synchronized public void refreshWatcher(long windowXID);
	
	/**
	 * Este método es invocado cuando se registra el bus del menu global, si el menu global
	 * no esta o se excluye por algun otro método como la variable de ambiente UBUNTU_MENUPROXY
	 * este método nunca será invocado.
	 * 
	 * @param state estado de registro, cauando es la primera vez regresa el valor de
	 * <code>REGISTER_STATE_INITIAL</code> si se esta registrando por un refresh regresa el
	 * valor de <code>REGISTER_STATE_REFRESH</code>.
	 */
	abstract protected void register(int state);
	/**
	 * Este método es invocado cuando se elimina el visualizador de Bus o si el bus es cerrado
	 * externamente.
	 */
	abstract protected void unregister();
	
	/**
	 * Agrega un nuevo menú de folder nativo.
	 * 
	 * @param windowXID identificador de ventana.
	 * @param menuParentId identificador del menu padre, para especificar un menu directamente en la barra
	 * de menú el identificador del padre debe ser <code>-1</code>.
	 * @param menuId identificador del menú.
	 * @param label etiqueta del menú.
	 * @param enabled estado de habilitación del menú.
	 * @param visible estado de visibulidad del menú.
	 */
	native public void addMenu(long windowXID, int menuParentId, int menuId,
			String label, boolean enabled, boolean visible);
	/**
	 * Agrega un elemento de menú nativo.
	 * 
	 * @param windowXID identificador de ventana.
	 * @param menuParentId identificador del menú padre.
	 * @param menuId identificador del menú.
	 * @param label etiqueta del menú.
	 * @param enabled estado de habilitación del menú.
	 * @param modifiers modificador del acelerador del menú (CTRL, ALT o SHIFT).
	 * @param keycode acelerador del menú.
	 */
	native public void addMenuItem(long windowXID, int menuParentId, int menuId,
			String label, boolean enabled, int modifiers, int keycode);
	/**
	 * Agrega un elemento de menú radio nativo.
	 * 
	 * @param windowXID identificador de ventana.
	 * @param menuParentId identificador del menú padre.
	 * @param menuId identificador del menú.
	 * @param label etiqueta del menú.
	 * @param enabled estado de habilitación del menú.
	 * @param modifiers modificador del acelerador del menú (CTRL, ALT o SHIFT).
	 * @param keycode acelerador del menú.
	 * @param selected estado de selección del menú.
	 */
	native public void addMenuItemRadio(long windowXID, int menuParentId, int menuId,
			String label, boolean enabled, int modifiers, int keycode, boolean selected);
	/**
	 * Agrega un elemento de menú check nativo.
	 * 
	 * @param windowXID identificador de ventana.
	 * @param menuParentId identificador del menú padre.
	 * @param menuId identificador del menú.
	 * @param label etiqueta del menú.
	 * @param enabled estado de habilitación del menú.
	 * @param modifiers modificador del acelerador del menú (CTRL, ALT o SHIFT).
	 * @param keycode acelerador del menú.
	 * @param selected estado de selección del menú.
	 */
	native public void addMenuItemCheck(long windowXID, int menuParentId, int menuId,
			String label, boolean enabled, int modifiers, int keycode, boolean selected);
	/**
	 * Agrega un elmemento de menú de separador nativo.
	 * 
	 * @param windowXID identificador del ventana.
	 * @param menuParentId identificador del menú padre.
	 */
	native public void addSeparator(long windowXID, int menuParentId);
	/**
	 * Actualización de estado del menú nativo.
	 * 
	 * @param windowXID identificador de ventana
	 * @param menuId identificador de menu
	 * @param label nuevo valor de etiqueta
	 * @param enabled nuevo valor de estado de habilitación del menú.
	 * @param visible nuevo valor de estado de visibilidad del menú.
	 */
	native public void updateMenu(long windowXID, int menuId, String label,
			boolean enabled, boolean visible);
	
	/**
	 * Este método es invocado cuando un menú nativo es seleccionado.
	 * 
	 * @param parentMenuId identificador del pardre del menú.
	 * @param menuId identificador del menú seleccionado.
	 */
	abstract protected void menuActivated(int parentMenuId, int menuId);
	/**
	 * Este método es invocado cuando un menú folder es abierto.
	 * 
	 * @param parentMenuId identificador del padre del menú, si el menú esta
	 * directamente a la barra de menu el valor del padre es <code>-1</code>
	 * @param menuId identificador del menú.
	 */
	abstract protected void menuAboutToShow(int parentMenuId, int menuId);
	/**
	 * Este método es invocado cuando un menú folder es cerrado.
	 * 
	 * @param parentMenuId identificador del padre del menú, si el menú esta
	 * directamente a la barra de menu el valor del padre es <code>-1</code>
	 * @param menuId identificador del menú.
	 */
	abstract protected void menuAfterClose(int parentMenuId, int menuId);
}
