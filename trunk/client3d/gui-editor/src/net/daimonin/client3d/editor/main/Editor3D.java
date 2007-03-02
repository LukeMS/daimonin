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

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;

import javax.imageio.ImageIO;

import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.PropertiesConfiguration;
import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;

import net.daimonin.client3d.editor.object.ImageSetImage;
import net.daimonin.client3d.editor.util.FilenameFilterPNG;
import net.daimonin.client3d.editor.util.binpacker.BinPacker;
import net.daimonin.client3d.editor.util.binpacker.Rect;

/**
 * The editor for the Daimonin 3D-Client.
 *  
 * @author Rumbuff
 */
public class Editor3D {

	/**
	 * Current version number.
	 */
	private static final String VERSION = "0.1.6";
	
	/**
	 * Number of command line arguments.
	 */
	private static final int ARGSLENGTH = 2;
	
	/**
	 * Operation mode 'do all'.
	 */
	private static final String ARGSMODEALL = "all"; 
	
	/**
	 * Operation mode 'read XML'.
	 */
	private static final String ARGSMODEXML = "xml";

	/**
	 * ImageSetImages, all read PNGs.
	 */
	private List<ImageSetImage> images;
	
	/**
	 * @param args  Command line parameters.
	 */
	public static void main(final String[] args) {
		
		try {
			
			Editor3D app = new Editor3D();			
						
			// check command line args
			app.checkCommandlineArgs(args);
			
			// read config file and set config values
			final PropertiesConfiguration conf = app.readConfig(args[1]);
			final String imageset = conf.getString("imageset.filename");
			final String imagesetxml = conf.getString("imageset.filename.xml");
			final String startingDir = conf.getString("images.dir");
			int[] dimension = null;
			if (conf.getBoolean("imageset.restrictSize")) {
				dimension = new int[]{conf.getInt("imageset.maxWidth"), conf.getInt("imageset.maxHeight")};
			} else {
				dimension = new int[]{-1, -1};
			}
			
			// start work, depending on chosen operation mode
			System.out.println("Daimonin client3d gui-editor " + VERSION);
			if (ARGSMODEALL.equals(args[0])) {
				System.out.println("Running in all mode...");
				app.readImages(startingDir);				
				dimension = app.packImages(dimension);				
				app.writeXML(imageset, imagesetxml);
				app.writeImageSet(imageset, dimension);
			} else if (ARGSMODEXML.equals(args[0])) {
				System.out.println("Running in xml mode...");
				app.readXML(imagesetxml, startingDir);			
				if (dimension == null) {
					dimension = app.calcDimension();
				}
				app.writeImageSet(imageset, dimension);
			}
			System.out.println("Success!");
      		
		} catch (IOException e) {
			e.printStackTrace();
		} catch (DocumentException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Gets the editor's configuration.
	 * @param confFile	Name of the config file.
	 * @return 						The editor's configuration.
	 */
	private PropertiesConfiguration readConfig(String confFile) {
		try {
			if ("default".equals(confFile)) {
				confFile = "editor3d.config";
			}		
			
			// test if confFile exists
			if (! new File(confFile).exists()) {
				exit(1, "Configuration file '" + confFile + "' does not exist!");
			}
			
			return new PropertiesConfiguration(confFile);
		} catch (ConfigurationException e) {			
			e.printStackTrace();
			return null;
		}
	}
	
	/**
	 * Checks number of passed command line arguments.
	 * @param args	Command line arguments.
	 */
	private void checkCommandlineArgs(final String[] args) {
		if (! (args.length == ARGSLENGTH
				&& (ARGSMODEALL.equals(args[0]) || ARGSMODEXML.equals(args[0])))) {
			printUsage();
			System.exit(0);
		}
	}
	
	/**
	 * Print usage information to help the user with application commands.
	 */
	private void printUsage() {
		System.out.println("USAGE:\n\neditor3d_start <operation mode> <configuration file>");
		System.out.println("\n<operation mode>   Either 'all' or 'xml' (see below).");
		System.out.println("<configuration file> 'default' for default configuration (editor3d.config)"
			+ " or specify your own config file.");
		System.out.println("\nExamples:\neditor3d_start all default\neditor3d_start all myConfig.conf");
		System.out.println("\nOperation mode 'all':	All PNGs in starting dir are read recursively,"
			+ " packed into one rectangle, XML file with image meta data gets written, PNG containing all PNGs gets written.");
		System.out.println("\nOperation mode 'xml':	XML file with meta information is read, PNG containing all PNGs gets written."
			+ " Use this mode if you want to create or modify the XML by yourself.");
	}
	
	/**
	 * Builds a single PNG out of all ImageSetImages, considering their calculated coordinates. 
	 * @param fileNameImageSet	Name of resulting PNG.
	 * @param dimension					[width, height] of the resulting PNG.
	 * @throws IOException			IOException.
	 */
	private void writeImageSet(final String fileNameImageSet, final int[] dimension) throws IOException {
		
		BufferedImage bigImg = new BufferedImage(dimension[0], dimension[1], BufferedImage.TYPE_INT_ARGB);
		Graphics2D big = bigImg.createGraphics();
		for (int i = 0; i < images.size(); i++) {
			big.drawImage(images.get(i).getImage(), images.get(i).getPosX(),
					images.get(i).getPosY(), null);
		}		
		big.dispose();
		ImageIO.write(bigImg, "png", new File(fileNameImageSet));
	}

	/**
	 * Reads the XML and all PNGs in startingDir to create ImageSetImages.
	 * All former ImageSetImages get overwritten.
	 * @param fileNameXML					Name of the XML.
	 * @param startingDirImages		Dir where the PNGs are (read recursively).
	 * @throws IOException				IOException.
	 * @throws DocumentException	DocumentException.
	 */
	private void readXML(final String fileNameXML, final String startingDirImages)
	  throws IOException, DocumentException {
		// read PNGs
		List<File> gfxFiles = new ArrayList<File>();		
		readFilesRecursively(new File(startingDirImages), gfxFiles);
		images = new ArrayList<ImageSetImage>(gfxFiles.size());
		
		// read XML
		SAXReader reader = new SAXReader();
    Document document = reader.read(new File(fileNameXML));
    Element root = document.getRootElement();
    
    for (Iterator i = root.elementIterator(); i.hasNext();) {
      Element element1 = (Element) i.next();
      String name1 = null;
      for (Iterator k = element1.attributeIterator(); k.hasNext();) {
      	Attribute attr = (Attribute) k.next();
      	if ("name".equals(attr.getName())) {
      		name1 = attr.getValue();
      	}
      }
      for (Iterator j = element1.elementIterator(); j.hasNext();) {
      	Element element2 = (Element) j.next();      	
        int x = -1;
      	int y = -1;
      	String name2 = null;
        for (Iterator k = element2.attributeIterator(); k.hasNext();) {
        	Attribute attr = (Attribute) k.next();        	
        	if ("name".equals(attr.getName())) {
        		name2 = attr.getValue();
        	}
        	if ("posX".equals(attr.getName())) {
        		x = Integer.valueOf(attr.getValue());
        	}
        	if ("posY".equals(attr.getName())) {
        		y = Integer.valueOf(attr.getValue());
        	}        	
        }
        File imgFile = getFileByName(gfxFiles, name1 + "_" + name2 + ".png");
        ImageSetImage img = new ImageSetImage(imgFile);
        img.setPosX(x);
        img.setPosY(y);
        images.add(img);
			}      
    }
	}
	
	/**
	 * Searches in a list of files for a file with the given name.
	 * @param files		List of files to search in.
	 * @param name		Name of the wanted file.
	 * @return				The wanted file, or null if not found.
	 */
	private File getFileByName(final List<File> files, final String name) {
		for (int i = 0; i < files.size(); i++) {
			if (files.get(i).getName().toLowerCase().equals(name.toLowerCase())) {
				return files.get(i);
			}
		}
		return null;
	}
	
	/**
	 * Scans all ImageSetImages and calculates the dimension of the enclosing rectangle.
	 * @return  dimension of the enclosing rectangle.
	 */
	private int[] calcDimension() {
		int w = -1;
		int h = -1;
		for (int i = 0; i < images.size(); i++) {
			if (images.get(i).getPosX() + images.get(i).getWidth() > w) {
				w = images.get(i).getPosX() + images.get(i).getWidth();
			}
			if (images.get(i).getPosY() + images.get(i).getHeight() > h) {
				h = images.get(i).getPosY() + images.get(i).getHeight();
			}
		}		
		return new int[]{w, h};
	}
	
	/**
	 * Builds image groups/categories by file name, creates and writes the XML containing
	 * all images with their attributes.
	 * @param fileNameImageSet	Name of resulting PNG.
	 * @param fileNameXML				Name of the XML.
	 * @throws IOException			IOException.
	 */
	private void writeXML(final String fileNameImageSet, final String fileNameXML) throws IOException {
		
		// group ImageSetImages by name/category.
		// the left side of the image file name up to the '_' is taken as the category 
		Hashtable<String, List<ImageSetImage>> names = new Hashtable<String, List<ImageSetImage>>();
		for (int i = 0; i < images.size(); i++) {
			if (! names.containsKey(images.get(i).getName())) {
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
			if (! "fontextensions".equals(img2.get(0).getName())) {
				boolean hasDefault = false;
				for (int i = 0; i < img2.size(); i++) {
					if ("default".equals(img2.get(i).getState().toLowerCase())) {
						hasDefault = true;
					}
				}
				if (! hasDefault) {
					System.out.println("WARNING: image category '" + img2.get(0).getName()
						+ "' has no image with a default state!");
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
				category.addElement("State").addAttribute("name", fntex.get(i).getState())
				  .addAttribute("posX", String.valueOf(fntex.get(i).getPosX()))
				  .addAttribute("posY", String.valueOf(fntex.get(i).getPosY()))
				  .addAttribute("width", String.valueOf(fntex.get(i).getWidth()))
					.addAttribute("height", String.valueOf(fntex.get(i).getHeight()));					
			}
			names.remove("fontextensions");
		}
		
		List<ImageSetImage> mouse = names.get("mousecursor");
		if (mouse != null) {
			Element category = root.addElement("Image").addAttribute("name", mouse.get(0).getName())
			.addAttribute("width", String.valueOf(mouse.get(0).getImage().getWidth()))
			.addAttribute("height", String.valueOf(mouse.get(0).getImage().getHeight()))
			.addAttribute("alpha", mouse.get(0).getImage().getColorModel().hasAlpha()
					? String.valueOf(1) : String.valueOf(0));
		
			for (int i = 0; i < mouse.size(); i++) {
				checkImageSameDimension(mouse.get(0), mouse.get(i), "Images of same category have different dimension");
				checkImageSameAlpha(mouse.get(0), mouse.get(i), "Images of same category have different alpha");				
				category.addElement("State").addAttribute("name", mouse.get(i).getState())
				  .addAttribute("posX", String.valueOf(mouse.get(i).getPosX()))
				  .addAttribute("posY", String.valueOf(mouse.get(i).getPosY()));											
			}
			names.remove("mousecursor");
		}
		
		Enumeration<String> keys = names.keys();
		while (keys.hasMoreElements()) {
			List<ImageSetImage> img = names.get(keys.nextElement());
			Element category = root.addElement("Image").addAttribute("name", img.get(0).getName())
			.addAttribute("width", String.valueOf(img.get(0).getImage().getWidth()))
			.addAttribute("height", String.valueOf(img.get(0).getImage().getHeight()))
			.addAttribute("alpha", img.get(0).getImage().getColorModel().hasAlpha()
					? String.valueOf(1) : String.valueOf(0));
		
			for (int i = 0; i < img.size(); i++) {
				checkImageSameDimension(img.get(0), img.get(i), "Images of same category have different dimension");
				checkImageSameAlpha(img.get(0), img.get(i), "Images of same category have different alpha");
				category.addElement("State").addAttribute("name", img.get(i).getState())
				  .addAttribute("posX", String.valueOf(img.get(i).getPosX()))
				  .addAttribute("posY", String.valueOf(img.get(i).getPosY()));											
			}			
		}
		
		// write the XML
		OutputFormat format = OutputFormat.createPrettyPrint();
		XMLWriter writer = new XMLWriter(new FileWriter(fileNameXML), format);
		writer.write(document);
		writer.close();
	}

	/**
	 * Compares width and height of two images and throws a RuntimeException if they differ.
	 * @param img1			First image to check.
	 * @param img2			Second image to check.
	 * @param message		Prefix for the exception's message.
	 */
	private void checkImageSameDimension(final ImageSetImage img1, final ImageSetImage img2,
			final String message) {
		final int width1 = img1.getWidth();
		final int width2 = img2.getWidth();
		final int height1 = img1.getHeight();
		final int height2 = img2.getHeight();
		if (width1 != width2 || height1 != height2) {			
			exit(1, message + ": " 
					+ img1 + "-> width=" + width1 + " height=" + height1 + ", "
					+ img2 + "-> width=" + width2 + " height=" + height2);
		}		
	}
	
	/**
	 * Compares the alpha/transparency of two images and throws a RuntimeException if they differ.
	 * @param img1			First image to check.
	 * @param img2			Second image to check.
	 * @param message		Prefix for the exception's message.
	 */
	private void checkImageSameAlpha(final ImageSetImage img1, final ImageSetImage img2,
			final String message) {
		final boolean alpha1 = img1.getImage().getColorModel().hasAlpha();
		final boolean alpha2 = img2.getImage().getColorModel().hasAlpha();
		if (alpha1 != alpha2) {
			exit(1, message + ": " + img1 + "-> alpha=" + alpha1 + ", " + img2 + "-> alpha=" + alpha2);			
		}
	}
	
	/**
	 * Calculates optimum x and y coordinates of the PNGs using a 2D BinPacker algorithm.
	 * After packing these coordinates get stored in the ImageSetImages.
	 * @param dimension		Maximum dimension of the enclosing rectangle [width, height].
	 * @return  The dimension of the enclosing rectangle [width, height].
	 */
	private int[] packImages(final int[] dimension) {
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
		return dimension;
	}

	/**
	 * Reads all PNG files recursively from the starting dir and creates ImageSetImages. 
	 * @param startingDir		starting dir to read PNGs recursively.
	 */
	private void readImages(final String startingDir) {
		List<File> gfxFiles = new ArrayList<File>();		
		readFilesRecursively(new File(startingDir), gfxFiles);
		images = new ArrayList<ImageSetImage>(gfxFiles.size());
		
		// check the PNGs for correct file names		
		if (! ImageSetImage.checkFilenames(gfxFiles)) {
			exit(1, "Invalid format of image file name. I have to exit.");			
		}
		
		for (int i = 0; i < gfxFiles.size(); i++) {
			images.add(new ImageSetImage(gfxFiles.get(i)));
		}
	}
	
	/**
   * Fills a list of PNG files, recursively from a starting dir.
   * 
   * @param startingDir   starting dir.
   * @param fileList  		file list to be filled.
   */
  private void readFilesRecursively(final File startingDir, final List<File> fileList) {
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
   * @param img	ImageSetImage.
   * @return		BinPacker rectangle with the ImageSetImage's width and height.	
   */
  private Rect createRect(final ImageSetImage img) {
  	return new Rect(img.toString(), img.getWidth(), img.getHeight());
  }  
  
  /**
   * Terminates the application, does some cleaning before if necessary.
   * @param status  The exit status (0 = regular, 1 = error).
   * @param message	Message to print out.
   */
  private void exit(int status, String message) {
  	if (status == 0) {
  		System.out.println(message);
  		System.exit(0);
  	} else {
  		System.err.println(message);
  		System.exit(1);
  	}  	
  }
}
