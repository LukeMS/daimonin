/*
 * This source file is part of Daimonin (http://daimonin.sourceforge.net)
 * Copyright (c) 2007 The Daimonin Team
 * Also see acknowledgements in Readme.html
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * In addition, as a special exception, the copyright holders of client3d give
 * you permission to combine the client3d program with lgpl libraries of your
 * choice and/or with the fmod libraries.
 * You may copy and distribute such a system following the terms of the GNU GPL
 * for client3d and the licenses of the other code concerned.
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/licenses/licenses.html
 */

package net.daimonin.client3d.editor.ui;

import javax.swing.JTextArea;

import net.daimonin.client3d.editor.main.Editor3D;

/**
 * Main frame which contain all other GUI containers and the menu bar.
 * 
 * @author Rumbuff
 */
public class MainFrame extends javax.swing.JFrame implements java.awt.event.ActionListener {

	/** Creates new form MainFrame */
	public MainFrame() {
		initComponents();
	}

	/**
   * This method is called from within the constructor to initialize the form.
   * WARNING: Do NOT modify this code. The content of this method is always
   * regenerated by the Form Editor.
   */
	// <editor-fold defaultstate="collapsed" desc=" Generated Code ">
	private void initComponents() {
		java.awt.GridBagConstraints gridBagConstraints;

		jPanel = new javax.swing.JPanel();
		jScrollPaneMain = new javax.swing.JScrollPane();
		jScrollPaneInfo = new javax.swing.JScrollPane();
		jTextAreaInfo = new javax.swing.JTextArea();
		menuBar = new javax.swing.JMenuBar();
		menuFile = new javax.swing.JMenu();
		menuItemFileExit = new javax.swing.JMenuItem();
		menuImageset = new javax.swing.JMenu();
		menuItemISAll = new javax.swing.JMenuItem();
		menuOptions = new javax.swing.JMenu();
		menuItemOptionsPrefs = new javax.swing.JMenuItem();
		menuHelp = new javax.swing.JMenu();
		menuItemHelpAbout = new javax.swing.JMenuItem();

		setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
		setTitle("Daimonin 3D GUI-editor");
		jPanel.setLayout(new java.awt.GridBagLayout());

		jPanel.setMinimumSize(new java.awt.Dimension(550, 10));
		jPanel.setPreferredSize(new java.awt.Dimension(600, 300));
		jScrollPaneMain.setMinimumSize(new java.awt.Dimension(560, 320));
		jScrollPaneMain.setPreferredSize(new java.awt.Dimension(560, 320));
		jPanel.add(jScrollPaneMain, new java.awt.GridBagConstraints());

		jScrollPaneInfo.setMinimumSize(new java.awt.Dimension(560, 126));
		jScrollPaneInfo.setPreferredSize(new java.awt.Dimension(560, 126));
		jTextAreaInfo.setColumns(20);
		jTextAreaInfo.setEditable(false);
		jTextAreaInfo.setRows(5);
		jScrollPaneInfo.setViewportView(jTextAreaInfo);

		gridBagConstraints = new java.awt.GridBagConstraints();
		gridBagConstraints.gridx = 0;
		gridBagConstraints.gridy = 2;
		gridBagConstraints.anchor = java.awt.GridBagConstraints.SOUTH;
		jPanel.add(jScrollPaneInfo, gridBagConstraints);

		menuBar.setName("Daimonin 3D GUI-Editor");
		menuFile.setText("File");
		menuItemFileExit.setText("Exit");
		menuItemFileExit.addActionListener(this);

		menuFile.add(menuItemFileExit);

		menuBar.add(menuFile);

		menuImageset.setText("Imageset");
		menuItemISAll.setText("Generate");
		menuItemISAll.addActionListener(this);

		menuImageset.add(menuItemISAll);

		menuBar.add(menuImageset);

		menuOptions.setText("Options");
		menuItemOptionsPrefs.setText("Preferences");
		menuItemOptionsPrefs.addActionListener(this);

		menuOptions.add(menuItemOptionsPrefs);

		menuBar.add(menuOptions);

		menuHelp.setText("Help");
		menuItemHelpAbout.setText("About");
		menuItemHelpAbout.addActionListener(this);

		menuHelp.add(menuItemHelpAbout);

		menuBar.add(menuHelp);

		setJMenuBar(menuBar);

		javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
		getContentPane().setLayout(layout);
		layout.setHorizontalGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING).addComponent(
				jPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE));
		layout.setVerticalGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING).addGroup(
				layout.createSequentialGroup().addComponent(jPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 464,
						javax.swing.GroupLayout.PREFERRED_SIZE).addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE,
						Short.MAX_VALUE)));
		pack();
	}

	// Code for dispatching events from components to event handlers.

	public void actionPerformed(java.awt.event.ActionEvent evt) {
		if (evt.getSource() == menuItemFileExit) {
			MainFrame.this.menuItemFileExitActionPerformed(evt);
		} else if (evt.getSource() == menuItemISAll) {
			MainFrame.this.menuItemISAllActionPerformed(evt);
		} else if (evt.getSource() == menuItemOptionsPrefs) {
			MainFrame.this.menuItemOptionsPrefsActionPerformed(evt);
		} else if (evt.getSource() == menuItemHelpAbout) {
			MainFrame.this.menuItemHelpAboutActionPerformed(evt);
		}
	}

	private void menuItemHelpAboutActionPerformed(java.awt.event.ActionEvent evt) {
		JTextArea text = new JTextArea();
		text.setEditable(false);
		text.append("This is the DAIMONIN gui-editor version " + Editor3D.VERSION + " for the new 3D client.\n");
		text.append("See http://www.daimonin.net for details about the game.\n\n");
		text.append("This software is distributed under the terms of the GPL 2, see file LICENSE for details.\n");
		jScrollPaneMain.setViewportView(text);
	}

	private void menuItemOptionsPrefsActionPerformed(java.awt.event.ActionEvent evt) {
		allPanel = null;
		prefPanel = new PreferencesPanel();
		jScrollPaneMain.setViewportView(prefPanel);
	}

	private void menuItemFileExitActionPerformed(final java.awt.event.ActionEvent evt) {
		System.exit(0);
	}

	private void menuItemISAllActionPerformed(final java.awt.event.ActionEvent evt) {
		prefPanel = null;
		allPanel = new GeneratePanel();
		jScrollPaneMain.setViewportView(allPanel);
	}

	/** preferences panel * */
	public PreferencesPanel prefPanel;

	/** panel for imageset generation * */
	public GeneratePanel allPanel;

	// Variables declaration - do not modify//GEN-BEGIN:variables
	private javax.swing.JPanel jPanel;

	private javax.swing.JScrollPane jScrollPaneInfo;

	public javax.swing.JScrollPane jScrollPaneMain;

	public javax.swing.JTextArea jTextAreaInfo;

	private javax.swing.JMenuBar menuBar;

	private javax.swing.JMenu menuFile;

	private javax.swing.JMenu menuHelp;

	private javax.swing.JMenu menuImageset;

	private javax.swing.JMenuItem menuItemFileExit;

	private javax.swing.JMenuItem menuItemHelpAbout;

	private javax.swing.JMenuItem menuItemISAll;

	private javax.swing.JMenuItem menuItemOptionsPrefs;

	private javax.swing.JMenu menuOptions;
	// End of variables declaration//GEN-END:variables

}
