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
package net.daimonin.client3d.editor.main;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.awt.image.renderable.ParameterBlock;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;

import javax.imageio.ImageIO;
import javax.media.jai.BorderExtenderConstant;
import javax.media.jai.JAI;

import net.daimonin.client3d.editor.object.ImageSetImage;
import net.daimonin.client3d.editor.ui.GeneratePanel;
import net.daimonin.client3d.editor.ui.MainFrame;
import net.daimonin.client3d.editor.ui.PreferencesPanel;
import net.daimonin.client3d.editor.util.FilenameFilterPNG;
import net.daimonin.client3d.editor.util.binpacker.BinPacker;
import net.daimonin.client3d.editor.util.binpacker.Rect;

import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.PropertiesConfiguration;
import org.dom4j.Document;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;

/**
 * The editor for the Daimonin 3D-Client.
 * 
 * @author Rumbuff
 */
public class Editor3D {

	private static final String CONF_RESTRICT_SIZE_HEIGHT = "imageset.maxHeight";

	private static final String CONF_RESTRICT_SIZE_WIDTH = "imageset.maxWidth";

	private static final String CONF_RESTRICT_SIZE = "imageset.restrictSize";

	private static final String CONF_FILENAME_IMAGESET_XML = "imageset.filename.xml";

	private static final String CONF_FILENAME_IMAGESET = "imageset.filename";

	private static final String CONF_BORDER_COLOR_BLUE = "imageset.border.color.blue";

	private static final String CONF_BORDER_COLOR_GREEN = "imageset.border.color.green";

	private static final String CONF_BORDER_COLOR_RED = "imageset.border.color.red";

	private static final String CONF_BORDER_SIZE = "imageset.border.size";

	private static final String CONF_IMAGES_DIR = "images.dir";

	/**
   * Current version number.
   */
	public static final String VERSION = "0.2.0";

	/**
   * Maximum border size.
   */
	private static final int BORDERSIZEMAX = 3;

	/**
   * Maximum border color.
   */
	private static final int BORDERCOLORMAX = 255;

	/**
   * Storage for configuration.
   */
	private static PropertiesConfiguration conf;

	/*
   * Configuration properties.
   */
	/** Name of the Imageset.PNG * */
	public static String imageset;

	/** Name of the GUI_ImageSet.xml * */
	public static String imagesetxml;

	/** Starting directory to search for PNGs * */
	public static String startingDir;

	/** Border size for all PNGs * */
	public static String borderSize;

	/** Border color for all PNGs, red * */
	public static String borderColorR;

	/** Border color for all PNGs, green * */
	public static String borderColorG;

	/** Border color for all PNGs, blue * */
	public static String borderColorB;

	/** If the generated Imageset should be restricted in size * */
	public static boolean restrictImageSize;

	/** Max. width, if size restricted * */
	public static int maxWidth;

	/** Max. height, if size restricted * */
	public static int maxHeight;

	/** Dimension of the packed imageset * */
	private static int[] dimension;

	/**
   * ImageSetImages, all read PNGs.
   */
	private static List<ImageSetImage> images;

	/**
   * Main window.
   */
	private static MainFrame frame;

	/**
   * @param args Command line parameters.
   */
	public static void main(final String[] args) {

		// read config file and set config values
		conf = Editor3D.readConfig();
		imageset = conf.getString(CONF_FILENAME_IMAGESET);
		imagesetxml = conf.getString(CONF_FILENAME_IMAGESET_XML);
		startingDir = conf.getString(CONF_IMAGES_DIR);
		borderSize = conf.getString(CONF_BORDER_SIZE);
		borderColorR = conf.getString(CONF_BORDER_COLOR_RED);
		borderColorG = conf.getString(CONF_BORDER_COLOR_GREEN);
		borderColorB = conf.getString(CONF_BORDER_COLOR_BLUE);
		restrictImageSize = conf.getBoolean(CONF_RESTRICT_SIZE);
		maxWidth = conf.getInt(CONF_RESTRICT_SIZE_WIDTH);
		maxHeight = conf.getInt(CONF_RESTRICT_SIZE_HEIGHT);

		Editor3D.frame = new MainFrame();
		Editor3D.frame.setVisible(true);
		printInfo("Welcome to Daimonin 3D GUI-editor " + VERSION);
	}

	/**
   * Gets the editor's configuration.
   * 
   * @return The editor's configuration.
   */
	private static PropertiesConfiguration readConfig() {
		try {
			final String config = "editor3d.config";
			// test if confFile exists
			if (!new File(config).exists()) {
				exit(1, "Configuration file '" + config + "' does not exist!");
			}
			return new PropertiesConfiguration(config);
		} catch (ConfigurationException e) {
			e.printStackTrace();
			printError(e.getMessage());
			return null;
		}
	}

	/**
   * Builds a single PNG out of all ImageSetImages, considering their calculated
   * coordinates.
   * 
   * @param fileNameImageSet Name of resulting PNG.
   * @param dimension [width, height] of the resulting PNG. where 0 is maximum
   *          compression, 1 is no compression at all.
   * @throws IOException IOException.
   */
	private static void writeImageSet(final String fileNameImageSet, final int[] dimension) throws IOException {

		BufferedImage bigImg = new BufferedImage(dimension[0], dimension[1], BufferedImage.TYPE_INT_ARGB);
		Graphics2D big = bigImg.createGraphics();
		for (int i = 0; i < images.size(); i++) {
			if (images.get(i).getBorderSize() > 0) {
				ParameterBlock params = new ParameterBlock();
				params.addSource(images.get(i).getImage());
				params.add(images.get(i).getBorderSize()); // left pad
				params.add(images.get(i).getBorderSize()); // right pad
				params.add(images.get(i).getBorderSize()); // top pad
				params.add(images.get(i).getBorderSize()); // bottom pad
				params.add(new BorderExtenderConstant(new double[] { images.get(i).getBorderColor().getRed(),
						images.get(i).getBorderColor().getGreen(), images.get(i).getBorderColor().getBlue(), BORDERCOLORMAX }));

				big.drawImage(JAI.create("border", params).getAsBufferedImage(), images.get(i).getPosX(), images.get(i)
						.getPosY(), null);

			} else {
				big.drawImage(images.get(i).getImage(), images.get(i).getPosX(), images.get(i).getPosY(), null);
			}
		}

		big.dispose();
		ImageIO.write(bigImg, "png", new File(fileNameImageSet));
		printInfo(System.getProperty("user.dir") + "/" + imageset + " created");
	}

	/**
   * Builds image groups/categories by file name, creates and writes the XML
   * containing all images with their attributes.
   * 
   * @param fileNameImageSet Name of resulting PNG.
   * @param fileNameXML Name of the XML.
   * @throws IOException IOException.
   */
	private static void writeXML(final String fileNameImageSet, final String fileNameXML) throws IOException {

		// group ImageSetImages by name/category.
		// the left side of the image file name up to the '_' is taken as the
		// category
		Hashtable<String, List<ImageSetImage>> names = new Hashtable<String, List<ImageSetImage>>();
		for (int i = 0; i < images.size(); i++) {
			if (!names.containsKey(images.get(i).getName())) {
				List<ImageSetImage> values = new ArrayList<ImageSetImage>();
				values.add(images.get(i));
				names.put(images.get(i).getName(), values);
			} else {
				names.get(images.get(i).getName()).add(images.get(i));
			}
		}

		// check if every category has a default state (except FontExtensions)
		Enumeration<String> keys2 = names.keys();
		while (keys2.hasMoreElements()) {
			List<ImageSetImage> img2 = names.get(keys2.nextElement());
			if (!"fontextensions".equals(img2.get(0).getName())) {
				boolean hasDefault = false;
				for (int i = 0; i < img2.size(); i++) {
					if ("default".equals(img2.get(i).getState().toLowerCase())) {
						hasDefault = true;
					}
				}
				if (!hasDefault) {
					printError("WARNING: image category '" + img2.get(0).getName() + "' has no image with a default state!");
				}
			}
		}

		// create the XML structure
		Document document = DocumentHelper.createDocument();
		Element root = document.addElement("ImageSet").addAttribute("file", fileNameImageSet);

		List<ImageSetImage> fntex = names.get("fontextensions");
		if (fntex != null) {
			Element category = root.addElement("ImageFntExt").addAttribute("name", fntex.get(0).getName());
			for (int i = 0; i < fntex.size(); i++) {
				category.addElement("State").addAttribute("name", fntex.get(i).getState()).addAttribute("posX",
						String.valueOf(fntex.get(i).getPosX() + fntex.get(i).getBorderSize())).addAttribute("posY",
						String.valueOf(fntex.get(i).getPosY() + fntex.get(i).getBorderSize())).addAttribute("width",
						String.valueOf(fntex.get(i).getWidth())).addAttribute("height", String.valueOf(fntex.get(i).getHeight()));
			}
			names.remove("fontextensions");
		}

		List<ImageSetImage> mouse = names.get("mousecursor");
		if (mouse != null) {
			Element category = root.addElement("Image").addAttribute("name", mouse.get(0).getName()).addAttribute("width",
					String.valueOf(mouse.get(0).getImage().getWidth())).addAttribute("height",
					String.valueOf(mouse.get(0).getImage().getHeight())).addAttribute("alpha",
					mouse.get(0).getImage().getColorModel().hasAlpha() ? String.valueOf(1) : String.valueOf(0));

			for (int i = 0; i < mouse.size(); i++) {
				checkImageSameDimension(mouse.get(0), mouse.get(i), "Images of same category have different dimension");
				checkImageSameAlpha(mouse.get(0), mouse.get(i), "Images of same category have different alpha");
				category.addElement("State").addAttribute("name", mouse.get(i).getState()).addAttribute("posX",
						String.valueOf(mouse.get(i).getPosX() + mouse.get(i).getBorderSize())).addAttribute("posY",
						String.valueOf(mouse.get(i).getPosY() + mouse.get(i).getBorderSize()));
			}
			names.remove("mousecursor");
		}

		Enumeration<String> keys = names.keys();
		while (keys.hasMoreElements()) {
			List<ImageSetImage> img = names.get(keys.nextElement());
			Element category = root.addElement("Image").addAttribute("name", img.get(0).getName()).addAttribute("width",
					String.valueOf(img.get(0).getImage().getWidth())).addAttribute("height",
					String.valueOf(img.get(0).getImage().getHeight())).addAttribute("alpha",
					img.get(0).getImage().getColorModel().hasAlpha() ? String.valueOf(1) : String.valueOf(0));

			for (int i = 0; i < img.size(); i++) {
				checkImageSameDimension(img.get(0), img.get(i), "Images of same category have different dimension");
				checkImageSameAlpha(img.get(0), img.get(i), "Images of same category have different alpha");
				category.addElement("State").addAttribute("name", img.get(i).getState()).addAttribute("posX",
						String.valueOf(img.get(i).getPosX() + img.get(i).getBorderSize())).addAttribute("posY",
						String.valueOf(img.get(i).getPosY() + img.get(i).getBorderSize()));
			}
		}

		// write the XML
		OutputFormat format = OutputFormat.createPrettyPrint();
		XMLWriter writer = new XMLWriter(new FileWriter(fileNameXML), format);
		writer.write(document);
		writer.close();
		printInfo(System.getProperty("user.dir") + "/" + imagesetxml + " created");
	}

	/**
   * Compares width and height of two images and throws a RuntimeException if
   * they differ.
   * 
   * @param img1 First image to check.
   * @param img2 Second image to check.
   * @param message Prefix for the exception's message.
   */
	private static void checkImageSameDimension(final ImageSetImage img1, final ImageSetImage img2, final String message) {
		final int width1 = img1.getWidth();
		final int width2 = img2.getWidth();
		final int height1 = img1.getHeight();
		final int height2 = img2.getHeight();
		if (width1 != width2 || height1 != height2) {
			exit(1, message + ": " + img1 + "-> width=" + width1 + " height=" + height1 + ", " + img2 + "-> width=" + width2
					+ " height=" + height2);
		}
	}

	/**
   * Compares the alpha/transparency of two images and throws a RuntimeException
   * if they differ.
   * 
   * @param img1 First image to check.
   * @param img2 Second image to check.
   * @param message Prefix for the exception's message.
   */
	private static void checkImageSameAlpha(final ImageSetImage img1, final ImageSetImage img2, final String message) {
		final boolean alpha1 = img1.getImage().getColorModel().hasAlpha();
		final boolean alpha2 = img2.getImage().getColorModel().hasAlpha();
		if (alpha1 != alpha2) {
			exit(1, message + ": " + img1 + "-> alpha=" + alpha1 + ", " + img2 + "-> alpha=" + alpha2);
		}
	}

	/**
   * Calculates optimum x and y coordinates of the PNGs using a 2D BinPacker
   * algorithm. After packing these coordinates get stored in the
   * ImageSetImages.
   * 
   * @param dimension Maximum dimension of the enclosing rectangle [width,
   *          height].
   * @return The dimension of the enclosing rectangle [width, height].
   */
	private static int[] packImages(final int[] dimension) {
		// transform ImageSetImages into BinPacker rectangles
		List<Rect> rects = new ArrayList<Rect>(images.size());
		for (int i = 0; i < images.size(); i++) {
			rects.add(createRect(images.get(i)));
		}

		// pack
		BinPacker binPacker = new BinPacker();
		List<Rect> packedRects = binPacker.packOptimized(rects, false, dimension);

		assert (images.size() == packedRects.size());

		// store the coordinates
		for (int i = 0; i < images.size(); i++) {
			for (int j = 0; j < packedRects.size(); j++) {
				if ((images.get(i).toString()).equals(packedRects.get(j).getId())) {
					images.get(i).setPosX(packedRects.get(j).getX());
					images.get(i).setPosY(packedRects.get(j).getY());
				}
			}
		}
		// printInfo("PNG files packed");
		return dimension;
	}

	/**
   * Reads all PNG files recursively from the starting dir and creates
   * ImageSetImages.
   * 
   * @param startingDir starting dir to read PNGs recursively.
   * @param borderSize border size, 0 for no border.
   * @param borderColorR border color, Red value.
   * @param borderColorG border color, Green value.
   * @param borderColorB border color, Blue value.
   */
	private static void readImages(final String startingDir, final String borderSize, final String borderColorR,
			final String borderColorG, final String borderColorB) {
		List<File> gfxFiles = new ArrayList<File>();
		readFilesRecursively(new File(startingDir), gfxFiles);

		// check if there are files to process
		if (gfxFiles.size() == 0) {
			exit(1, "No PNG files found in directory '" + startingDir + "'. Nothing to do.");
		}
		images = new ArrayList<ImageSetImage>(gfxFiles.size());

		// check the PNGs for correct file names
		if (!ImageSetImage.checkFilenames(gfxFiles)) {
			exit(1, "Invalid format of image file name. I have to exit.");
		}

		// check border size/color values
		int lBorderSize = Integer.valueOf(borderSize);
		if (lBorderSize < 0) {
			lBorderSize = 0;
		} else if (lBorderSize > BORDERSIZEMAX) {
			lBorderSize = BORDERSIZEMAX;
		}
		int lBorderColorR = Integer.valueOf(borderColorR);
		int lBorderColorG = Integer.valueOf(borderColorG);
		int lBorderColorB = Integer.valueOf(borderColorB);
		if (lBorderColorR < 0) {
			lBorderColorR = 0;
		} else if (lBorderColorR > BORDERCOLORMAX) {
			lBorderColorR = BORDERCOLORMAX;
		}
		if (lBorderColorG < 0) {
			lBorderColorG = 0;
		} else if (lBorderColorG > BORDERCOLORMAX) {
			lBorderColorG = BORDERCOLORMAX;
		}
		if (lBorderColorB < 0) {
			lBorderColorB = 0;
		} else if (lBorderColorB > BORDERCOLORMAX) {
			lBorderColorB = BORDERCOLORMAX;
		}

		for (int i = 0; i < gfxFiles.size(); i++) {
			ImageSetImage image = new ImageSetImage(gfxFiles.get(i));
			if (lBorderSize > 0) {
				image.setBorderSize(lBorderSize);
				image.setBorderColor(new Color(lBorderColorR, lBorderColorG, lBorderColorB));
			}
			images.add(image);
		}
		// printInfo("PNG files read");
	}

	/**
   * Fills a list of PNG files, recursively from a starting dir.
   * 
   * @param startingDir starting dir.
   * @param fileList file list to be filled.
   */
	private static void readFilesRecursively(final File startingDir, final List<File> fileList) {
		if (startingDir.isFile()) {
			fileList.add(startingDir);
		}

		File[] children = startingDir.listFiles(new FilenameFilterPNG());
		if (children != null) {
			for (int i = 0; i < children.length; i++) {
				readFilesRecursively(children[i], fileList);
			}
		}
	}

	/**
   * Creates a BinPacker rectangle out of an ImageSetImage.
   * 
   * @param img ImageSetImage.
   * @return BinPacker rectangle with the ImageSetImage's width and height.
   */
	private static Rect createRect(final ImageSetImage img) {
		return new Rect(img.toString(), img.getWidth() + img.getBorderSize() * 2, img.getHeight() + img.getBorderSize() * 2);
	}

	/**
   * Terminates the application, does some cleaning before if necessary.
   * 
   * @param status The exit status (0 = regular, 1 = error).
   * @param message Message to print out.
   */
	private static void exit(final int status, final String message) {
		if (status == 0) {
			printInfo(message);
			System.exit(0);
		} else {
			printError(message);
		}
	}

	/**
   * Main method for generation of the imageset.
   * 
   * @param allPanel allPanel
   */
	public static void generate(GeneratePanel allPanel) {
		try {
			saveConfig(allPanel);
			if (restrictImageSize) {
				dimension = new int[] { maxWidth, maxHeight };
			} else {
				dimension = new int[] { -1, -1 };
			}
			readImages(startingDir, borderSize, borderColorR, borderColorG, borderColorB);
			dimension = packImages(dimension);
			writeXML(imageset, imagesetxml);
			writeImageSet(imageset, dimension);
			printSuccess("Success.\n");
		} catch (IOException e) {
			e.printStackTrace();
			printError(e.getMessage());
		}
	}

	/**
   * Saves preferences if they were changed.
   * 
   * @param prefPanel prefPanel
   */
	public static void savePreferences(PreferencesPanel prefPanel) {
		boolean prefsChanged = false;

		final String filenamePNG = prefPanel.jTextFieldFilenamePNG.getText();
		if (filenamePNG != null && !filenamePNG.equals(conf.getString(CONF_FILENAME_IMAGESET))) {
			prefsChanged = true;
			conf.setProperty(CONF_FILENAME_IMAGESET, filenamePNG);
			imageset = filenamePNG;
		}

		final String filenameXML = prefPanel.jTextFieldFilenameXML.getText();
		if (filenameXML != null && !filenameXML.equals(conf.getString(CONF_FILENAME_IMAGESET_XML))) {
			prefsChanged = true;
			conf.setProperty(CONF_FILENAME_IMAGESET_XML, filenameXML);
			imagesetxml = filenameXML;
		}

		final boolean restrictSize = prefPanel.jCheckBoxRestrictPNGSize.isSelected();
		if (!String.valueOf(restrictSize).equals(conf.getString(CONF_RESTRICT_SIZE))) {
			prefsChanged = true;
			conf.setProperty(CONF_RESTRICT_SIZE, String.valueOf(restrictSize));
			restrictImageSize = restrictSize;
		}

		final String restrictedWidth = prefPanel.jTextFieldRestrictedWidth.getText();
		if (restrictedWidth != null && !restrictedWidth.equals(conf.getString(CONF_RESTRICT_SIZE_WIDTH))) {
			prefsChanged = true;
			conf.setProperty(CONF_RESTRICT_SIZE_WIDTH, restrictedWidth);
			maxWidth = Integer.valueOf(restrictedWidth);
		}

		final String restrictedHeight = prefPanel.jTextFieldRestrictedHeight.getText();
		if (restrictedHeight != null && !restrictedHeight.equals(conf.getString(CONF_RESTRICT_SIZE_HEIGHT))) {
			prefsChanged = true;
			conf.setProperty(CONF_RESTRICT_SIZE_HEIGHT, restrictedHeight);
			maxHeight = Integer.valueOf(restrictedHeight);
		}

		try {
			if (prefsChanged) {
				conf.save(conf.getFile());
				printInfo("Preferences saved");
			}
		} catch (ConfigurationException e) {
			e.printStackTrace();
			printError(e.getMessage());
		}

		// clean up
		frame.jScrollPaneMain.setViewportView(null);
		frame.prefPanel = null;
	}

	/**
   * Cleaning up after leaving the preferences.
   */
	public static void cancelPreferences() {
		// clean up
		frame.jScrollPaneMain.setViewportView(null);
		frame.prefPanel = null;
	}

	private static void saveConfig(GeneratePanel allPanel) {
		boolean confChanged = false;
		if (allPanel.jCheckBoxStartingDir.isSelected() && !startingDir.equals(conf.getString(CONF_IMAGES_DIR))) {
			confChanged = true;
			conf.setProperty(CONF_IMAGES_DIR, startingDir);
		}
		if (allPanel.jCheckBoxBorderSize.isSelected() && !borderSize.equals(conf.getString(CONF_BORDER_SIZE))) {
			confChanged = true;
			conf.setProperty(CONF_BORDER_SIZE, borderSize);
		}
		if (allPanel.jCheckBoxBorderColor.isSelected() && !borderColorR.equals(conf.getString(CONF_BORDER_COLOR_RED))) {
			confChanged = true;
			conf.setProperty(CONF_BORDER_COLOR_RED, borderColorR);
		}
		if (allPanel.jCheckBoxBorderColor.isSelected() && !borderColorG.equals(conf.getString(CONF_BORDER_COLOR_GREEN))) {
			confChanged = true;
			conf.setProperty(CONF_BORDER_COLOR_GREEN, borderColorG);
		}
		if (allPanel.jCheckBoxBorderColor.isSelected() && !borderColorB.equals(conf.getString(CONF_BORDER_COLOR_BLUE))) {
			confChanged = true;
			conf.setProperty(CONF_BORDER_COLOR_BLUE, borderColorB);
		}

		try {
			if (confChanged) {
				conf.save(conf.getFile());
				printInfo("Configuration saved");
			}
		} catch (ConfigurationException e) {
			e.printStackTrace();
			printError(e.getMessage());
		}
	}

	private static void printInfo(String msg) {
		// TODO: change Text color to black
		frame.jTextAreaInfo.append(msg + "\n");
		frame.jTextAreaInfo.setCaretPosition(frame.jTextAreaInfo.getText().length());
	}

	private static void printError(String msg) {
		// TODO: change Text color to red
		frame.jTextAreaInfo.append(msg + "\n");
		frame.jTextAreaInfo.setCaretPosition(frame.jTextAreaInfo.getText().length());
	}

	private static void printSuccess(String msg) {
		// TODO: change Text color to green
		frame.jTextAreaInfo.append(msg + "\n");
		frame.jTextAreaInfo.setCaretPosition(frame.jTextAreaInfo.getText().length());
	}
}
