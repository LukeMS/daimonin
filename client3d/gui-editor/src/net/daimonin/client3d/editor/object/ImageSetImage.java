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
package net.daimonin.client3d.editor.object;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.List;

import javax.imageio.ImageIO;

/**
 * Simple Object to collect informations about a single ImageSet image (PNG file).
 * 
 * @author Rumbuff
 */
public class ImageSetImage {
	
	/**
	 * File.
	 */
	private File file;
	
	/**
	 * Width.
	 */
	private int width;
	
	/**
	 * Height.
	 */
	private int height;
	
	/**
	 * Name.
	 */
	private String name;
	
	/**
	 * State.
	 */
	private String state;
	
	/**
	 * Position x.
	 */
	private int posX;
	
	/**
	 * Position y.
	 */
	private int posY;
	
	/**
	 * Alpha.
	 */
	private boolean alpha;
	
	/**
	 * Image.
	 */
	private BufferedImage image;
	
	/**
	 * Separates the name from the state in the filename.
	 */
	public static final char SEPARATOR = '_';
	
	
	/**
	 * Constructs an ImageSet Image. All info about this image is collected automatically
	 * during construction.
	 * @param aFile The image.
	 */
	public ImageSetImage(final File aFile) {		
		file = aFile;
		collectImageInfo();
	}
	
	/**
	 * Check the the given files for correct file names.
	 * @param files		The files to check.
	 * @return				If all file names are correct.
	 */
	public static final boolean checkFilenames(final List<File> files) {
		boolean correct = true;
		for (int i = 0; i < files.size(); i++) {
			File aFile = files.get(i);
			if (aFile.getName().indexOf(SEPARATOR) < 1
					|| aFile.getName().indexOf(SEPARATOR) != aFile.getName().lastIndexOf(SEPARATOR)) {
				System.err.println("Invalid image name: '"
					+ aFile.getName() + "'. Must be \"<name>_<state>.png\".");
				correct = false;	
			}
		}
		return correct;
	}

	/**
	 * Reads the image and determines all attributes.
	 */
	private void collectImageInfo() {
		
		try {			
			String fileName = file.getName();
			name = fileName.substring(0, fileName.indexOf(SEPARATOR)).toLowerCase();
			state = fileName.substring(fileName.indexOf(SEPARATOR) + 1, fileName.indexOf(".png")).toLowerCase();
			
			image = ImageIO.read(file);
			
//			image = JAI.create("fileload", file.getAbsolutePath());
			width = image.getWidth();
			height = image.getHeight();

		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * @return the posX.
	 */
	public final int getPosX() {
		return posX;
	}

	/**
	 * @param aPosX the posX to set.
	 */
	public final void setPosX(final int aPosX) {
		posX = aPosX;
	}

	/**
	 * @return the posY.
	 */
	public final int getPosY() {
		return posY;
	}

	/**
	 * @param aPosY the posY to set.
	 */
	public final void setPosY(final int aPosY) {
		posY = aPosY;
	}

	/**
	 * @return the height.
	 */
	public final int getHeight() {
		return height;
	}

	/**
	 * @return the name.
	 */
	public final String getName() {
		return name;
	}

	/**
	 * @return the state.
	 */
	public final String getState() {
		return state;
	}

	/**
	 * @return the width.
	 */
	public final int getWidth() {
		return width;
	}
	
	/**
	 * @return the alpha.
	 */
	public final boolean isAlpha() {
		return alpha;
	}

	/**
	 * @param aAlpha the alpha to set.
	 */
	public final void setAlpha(final boolean aAlpha) {
		alpha = aAlpha;
	}

	/**
	 * @return the image.
	 */
	public final BufferedImage getImage() {
		return image;
	}

	/**
	 * Gets the name/filename.
	 * @return  the name/filename.
	 */
	public final String toString() {
		return name + SEPARATOR + state + ".png";
	}
}
