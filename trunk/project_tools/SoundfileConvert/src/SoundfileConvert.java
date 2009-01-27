/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package soundfileconvert;

/**
 *
 * @author Jeff
 */
import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.*;


public class SoundfileConvert
{
    public boolean SAXException = false;
    private PrintWriter out;

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        String inFileStr = new String("");
        String outFileStr = new String("");
        
        // If we have a command line input, do what is necessary
        if (args.length > 0)
        {
            inFileStr = args[0];
            if (args.length > 1)
                outFileStr = args[1];
            else
            {
                System.out.println("Usage: java -jar soundfileconvert.jar infile.xml outfile.dat");
                System.exit(0);
            }
        }
        
        new SoundfileConvert(inFileStr, outFileStr);
    }

    public SoundfileConvert(String inFileStr, String outFileStr)
    {
        File outF;
        Document doc;
  
        // Check we have valid command line input
        if ((inFileStr.length() == 0) || (outFileStr.length() == 0))
        {
            System.out.println("Usage: java -jar soundfileconvert.jar infile.xml outfile.dat");
            System.exit(0);
        }

        doc = getDocument(inFileStr);

        // If we got SAX errors, doc will be null.
        if (doc == null)
        {
            System.out.println("Conversion terminated on XML parse failure.");
            return;
        }
        
        // Traverse the document
        Element root = doc.getDocumentElement();
        Element soundtype; 
        Element sound;
        int soundtypeId;
        int soundId;
        int errors = 0;
        int id1 = 0;
        int id2 = 0;
        int nodeCount;
        String s1;
        String s2;
        boolean dup = false;
        String type;
        String name;
        String file;
        String prefix;
        Vector<Integer> soundtypeIds = new Vector<Integer>();
        Vector<String> soundTypes = new Vector<String>();
        Vector<Integer> soundIds = new Vector<Integer>();
        Vector<String> soundNames = new Vector<String>();
        Vector<Integer> idErrs = new Vector<Integer>();
        Vector<String> nErrs = new Vector<String>();
        
        // Build vectors of ids
        soundtype = (Element)root.getFirstChild();
        while (soundtype != null)
        {
            soundtypeId = Integer.parseInt(soundtype.getAttribute("id"));
            soundtypeIds.add(soundtypeId);
            soundTypes.add(soundtype.getAttribute("type"));
            sound = (Element)soundtype.getFirstChild();
            while (sound != null)
            {
                soundId = Integer.parseInt(sound.getAttribute("id"));
                soundIds.add(soundId);
                soundNames.add(sound.getAttribute("name"));
                sound = (Element)sound.getNextSibling();
            }
            
            // Check for duplicate sound ids
            dup = false;
            for (int i=0; i < soundIds.size()-1; i++)
            {
                id1 = soundIds.get(i);
                for (int j=i+1; j < soundIds.size(); j++)
                {
                    id2 = soundIds.get(j);
                    if (id2 == id1)
                    {
                        dup = true;
                        idErrs.add(id1);
                        errors++;
                    }
                }
            }
            if (dup)
            {
                for (int i=0; i < idErrs.size(); i++)
                    System.out.println("Duplicate sound id " +
                            idErrs.get(i) + " for soundtype id " +
                            soundtypeId + " (" +
                            soundTypes.get(soundtypeId) + ").");
                idErrs.clear();
            }
            soundIds.clear();

            // Check for duplicate sound names
            dup = false;
            for (int i=0; i < soundNames.size()-1; i++)
            {
                s1 = soundNames.get(i);
                for (int j=i+1; j < soundNames.size(); j++)
                {
                    if (soundNames.get(j).equalsIgnoreCase(s1))
                    {
                        dup = true;
                        nErrs.add(s1);
                        errors++;
                    }
                }
            }
            if (dup)
            {
                for (int i=0; i < nErrs.size(); i++)
                    System.out.println("Duplicate sound name '" +
                            nErrs.get(i) + "' for soundtype id " +
                            soundtypeId + " (" +
                            soundTypes.get(soundtypeId) + ").");
                nErrs.clear();
            }
            soundNames.clear();
            soundtype = (Element)soundtype.getNextSibling();
        }
        
        if (errors > 0)
        {
            System.out.println(errors + " errors in file " + inFileStr + ".");
            return;
        }
        
        // Input file successfully validated and checked for duplicates.
        // Generate the output file.
        outF = new File(outFileStr);
        if (outF.isDirectory())
        {
            System.out.println("Output file cannot be a directory.");
            return;
        }
        else if (outF.exists())
        {
            outF.delete();
            System.out.println("Existing output file was deleted.");
        }
        try
        {
            if (outF.createNewFile())
            {
                out = new PrintWriter(new BufferedWriter(new FileWriter(outF)));
                SimpleDateFormat dateFormatter =  new SimpleDateFormat("yyyy.MM.dd kk:mm:ss");
                String date = dateFormatter.format(new Date());
                out.println("# This file is automatically generated. Do not edit.");
                out.println("# Sound data file created on " + date);
                out.println("# Created from " + inFileStr);
                
                // Traverse the document, formatting and outputting lines
                nodeCount = root.getChildNodes().getLength();
                s1 = String.format("*start|%d", nodeCount);
                out.println(s1);
                soundtype = (Element)root.getFirstChild();
                while (soundtype != null)
                {
                    nodeCount = soundtype.getChildNodes().getLength();
                    soundtypeId = Integer.parseInt(soundtype.getAttribute("id"));
                    type = soundtype.getAttribute("type").toUpperCase();
                    prefix = soundtype.getAttribute("prefix").toUpperCase();
                    s1 = String.format("*%d|%s|%s|%d", soundtypeId,
                            type, prefix, nodeCount);
                    out.println(s1);
                    sound = (Element)soundtype.getFirstChild();
                    while (sound != null)
                    {
                        soundId = Integer.parseInt(sound.getAttribute("id"));
                        name = sound.getAttribute("name").toUpperCase();
                        file = sound.getAttribute("file");
                        s1 = String.format("+%d|%s|%s", soundId, name, file);
                        out.println(s1);

                        sound = (Element)sound.getNextSibling();
                    }
                    soundtype = (Element)soundtype.getNextSibling();
                }
            }
            else
            {
                System.out.println("Failed to create output file.");
                return;
            }        
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        // Write the end marker and close the output file
        out.println("*end");
        out.close();
        System.out.println("File " + inFileStr + " successfully converted.");
        return;
    }
  
    // Get a DOM document from the input xml file
    private Document getDocument(String name)
    {
        try
        {
            DocumentBuilderFactory factory =
                    DocumentBuilderFactory.newInstance();
            factory.setIgnoringComments(true);
            
            factory.setIgnoringElementContentWhitespace(true);
            factory.setValidating(true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            builder.setErrorHandler(new MyHandler());
            Document doc = builder.parse(new InputSource(name));
            return SAXException ? null : doc;
        }
        catch (Exception e)
        {
            System.out.println("exception in getDocument.");
            System.out.println(e.getMessage());
        }
        return null;
    }
    
    private class MyHandler implements ErrorHandler
    {
        private void doMessage(SAXParseException e, String s)
        {
            System.out.println("XML parse " + s + " at line " + 
                    e.getLineNumber() + ": " + e.getMessage());
            SAXException = true;
        }
        public void fatalError(SAXParseException e)
        {
            doMessage(e, "fatal error");
        }
        public void error(SAXParseException e)
        {
            doMessage(e, "error");
        }
        public void warning(SAXParseException e)
        {
            doMessage(e, "warning");
        }
    }
}
