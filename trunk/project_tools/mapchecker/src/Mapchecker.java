/*
 * Mapchecker.java
 *
 * Created on 21 April 2007, 20:44
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.event.*;
import com.jtechlabs.ui.widget.directorychooser.*;

/**
 *
 * @author Jeff
 */
public class Mapchecker extends JFrame
{    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args)
    {
        String topDirStr = new String("");
        String logFileStr = new String("");
        
        // If we have a command line input, do what is necessary
        if (args.length > 0)
        {
            topDirStr = args[0];
            if (args.length > 1)
                logFileStr = args[1];
            else
            {
                System.out.println("Usage: java -jar mapchecker.jar top_level_directory log_file_path");
                System.exit(0);
            }
        }
        
        new Mapchecker(topDirStr, logFileStr);
    }
 
    /**
     * Creates a new instance of Mapchecker
     */
    private JTextField topLevel;
    private JTextField logText;
    private JTextField mapText;
    private JCheckBox ignoreMissingFloors;
    private JButton buttonBrowse;
    private JButton buttonCheck;
    private JButton buttonExit;
    private JTextArea progressText;
    private JScrollPane progressScroll;
    private int browseResult;
    private String topLevelDir;
    private String mapRootDir;
    private int mapRootLen = 0;
    private String logPath;
    private String archRoot;
    private File mapFile;
    private File logFile;
    private File archFile;
    private File artFile;
    private RunChecks checker;
    private boolean mapFileSet = false;
    private boolean logFileSet = false;
    private boolean commandLine = false;
    
    final String version = "V1.8";
    
    public Mapchecker(String topDirStr, String logFileStr)
    {
        topLevelDir = topDirStr;
        logPath = logFileStr;
        
        // If we have command line input, no need to show the GUI
        if ((topLevelDir.length() > 0) && (logPath.length() > 0))
        {
            commandLine = true;
            mapFile = new File(topLevelDir);
            logFile = new File(logPath);
            
            if (checkMapPath())
            {
                checker = new RunChecks(mapFile, logFile, mapRootDir, mapRootLen, archRoot, null, null, false, version);
                checker.start();
                try
                {
                    // Allow time for thread to start
                    Thread.sleep(1000);
                } catch (InterruptedException ex)
                {
                    ex.printStackTrace();
                }

                while (checker.getActive())
                {
                    try
                    {
                        Thread.sleep(100);
                    } catch (InterruptedException ex)
                    {
                        ex.printStackTrace();
                    }
                }
            }
                
            System.exit(0);
        }
        else
        {
         
            this.setSize(350, 400);
            this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
            this.setTitle("Daimonin Map Checker " + version);

            TBrowseListener tbl = new TBrowseListener();
            LBrowseListener lbl = new LBrowseListener();
            TopLevelListener tldl = new TopLevelListener();
            LogTextListener ltdl = new LogTextListener();
            CheckListener cl = new CheckListener();
            ExitListener el = new ExitListener();

            addWindowListener(new WindowAdapter()
            {
                @Override
                public void windowClosing(WindowEvent e)
                {
                    buttonExit.doClick();
                }
            } ) ;

            Box box0 = Box.createVerticalBox();
            Box box1 = Box.createHorizontalBox();
            Box box2 = Box.createHorizontalBox();
            Box box3 = Box.createHorizontalBox();
            Box box4 = Box.createHorizontalBox();
            Box box5 = Box.createHorizontalBox();
            Box box6 = Box.createHorizontalBox();
            Box box7 = Box.createHorizontalBox();

            box0.add(Box.createVerticalStrut(20));

            box1.add(Box.createHorizontalStrut(10));
            box1.add(new JLabel("Check:   "));
            box1.add(Box.createHorizontalGlue());        
            topLevel = new JTextField(32);
            topLevel.setMaximumSize(new Dimension(300, 25));
            topLevel.getDocument().addDocumentListener(tldl);
            box1.add(topLevel);
            box1.add(Box.createHorizontalStrut(10));
            buttonBrowse = new JButton("...");
            buttonBrowse.addActionListener(tbl);
            box1.add(buttonBrowse);
            box1.add(Box.createHorizontalStrut(10));

            box2.add(Box.createHorizontalStrut(10));
            box2.add(new JLabel("Log file: "));
            box2.add(Box.createHorizontalGlue());        
            logText = new JTextField(32);
            logText.setMaximumSize(new Dimension(300, 25));
            logText.getDocument().addDocumentListener(ltdl);
            box2.add(logText);
            box2.add(Box.createHorizontalStrut(10));
            buttonBrowse = new JButton("...");
            buttonBrowse.addActionListener(lbl);
            box2.add(buttonBrowse);
            box2.add(Box.createHorizontalStrut(10));
            
            box3.add(Box.createHorizontalStrut(10));
            box3.add(new JLabel("Map: "));
            box3.add(Box.createHorizontalGlue());        
            mapText = new JTextField(32);
            mapText.setMaximumSize(new Dimension(300, 25));
            box3.add(mapText);
            box3.add(Box.createHorizontalStrut(10));

            box4.add(Box.createHorizontalStrut(10));
            box4.add(new JLabel("Progress:"));
            box4.add(Box.createHorizontalGlue());        
            progressText = new JTextArea(10, 20);
            progressText.setEditable(false);
            progressScroll = new JScrollPane(progressText, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
            box5.add(Box.createHorizontalStrut(10));
            box5.add(progressScroll);
            box5.add(Box.createHorizontalStrut(10));
            
            ignoreMissingFloors = new JCheckBox("Ignore missing floors");
            box6.add(ignoreMissingFloors);

            buttonExit = new JButton("Exit");
            buttonExit.addActionListener(el);
            buttonCheck = new JButton("Check Maps");
            buttonCheck.addActionListener(cl);
            buttonCheck.setEnabled(false);
            box7.add(Box.createHorizontalStrut(60));
            box7.add(buttonCheck);
            box7.add(Box.createHorizontalGlue());        
            box7.add(buttonExit);
            box7.add(Box.createHorizontalStrut(10));

            box0.add(box1);
            box0.add(Box.createVerticalStrut(10));
            box0.add(box2);
            box0.add(Box.createVerticalStrut(10));
            box0.add(box3);
            box0.add(Box.createVerticalStrut(10));
            box0.add(box4);
            box0.add(Box.createVerticalStrut(10));
            box0.add(box5);
            box0.add(Box.createVerticalStrut(10));
            box0.add(box6);
            box0.add(Box.createVerticalStrut(10));
            box0.add(Box.createVerticalGlue());
            box0.add(box7);
            box0.add(Box.createVerticalStrut(10));
            this.add(box0);
            this.setResizable(false);
            this.setLocationRelativeTo(null);
            this.setVisible(true);
            
            // Add help to progress box
            progressText.append("Browse for directory at head of map tree to be checked\r\nin 'Check' window," +
                                " then browse for log file name in\r\n'Log file' window." +
                                "\r\n\r\nYou can check from top-level /maps directory\r\nor from any directory below this." +
                                "\r\n\r\nWhen these are set, click on 'Check Maps' button.\r\n\r\n");
        }
    }

    private class TBrowseListener implements ActionListener
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
 //           final JDirectoryChooser dc = new JDirectoryChooser();
            File f = JDirectoryChooser.showDialog(null, null, "Select folder", "Choose top-level map directory",
                        /*JDirectoryChooser.ACCESS_NEW*/0);

            if (f != null)
            {
                topLevelDir = f.getPath();
                topLevel.setText(topLevelDir);
                mapFileSet = true;
                if (logFileSet)
                    buttonCheck.setEnabled(true);
            }
        }
    }

    private class TopLevelListener implements DocumentListener
    {
        @Override
        public void insertUpdate(DocumentEvent e)
        {
            topLevelDir = topLevel.getText();
            mapFileSet = true;
            if (logFileSet)
                buttonCheck.setEnabled(true);
        }

        @Override
        public void changedUpdate(DocumentEvent e)
        {
            boolean tf = true;
            
            topLevelDir = topLevel.getText();
            if (topLevel.getDocument().getLength() == 0)
                tf = false;
            mapFileSet = tf;
            buttonCheck.setEnabled(tf);
        }
        
        @Override
        public void removeUpdate(DocumentEvent e)
        {            
            boolean tf = true;
            
            topLevelDir = topLevel.getText();
            if (topLevel.getDocument().getLength() == 0)
                tf = false;
            mapFileSet = tf;
            buttonCheck.setEnabled(tf);
        }
    }

    private class LBrowseListener implements ActionListener
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            JFileChooser fc = new JFileChooser();
            fc.setDialogTitle("Select log file");
            fc.setApproveButtonText("Select");
//            fc.setDialogType(JFileChooser.SAVE_DIALOG);
            int result = fc.showSaveDialog(null);
            if (result == JFileChooser.APPROVE_OPTION)
            {
                File temp;
                temp = new File("");
                temp = fc.getSelectedFile();
                logPath = temp.getPath();
                logText.setText(logPath);
                logFileSet = true;
                if (mapFileSet)
                    buttonCheck.setEnabled(true);
            }
        }
    }

    private class LogTextListener implements DocumentListener
    {
        @Override
        public void insertUpdate(DocumentEvent e)
        {
                logFileSet = true;
                logPath = logText.getText();
                if (mapFileSet)
                    buttonCheck.setEnabled(true);
        }
        
        @Override
        public void changedUpdate(DocumentEvent e)
        {
            if (logText.getDocument().getLength() == 0)
            {
                logFileSet = false;
                buttonCheck.setEnabled(false);
            }
            else
            {
                logFileSet = true;
                logPath = logText.getText();
                if (mapFileSet)
                    buttonCheck.setEnabled(true);
            }
        }
        
        @Override
        public void removeUpdate(DocumentEvent e)
        {           
            if (logText.getDocument().getLength() == 0)
            {
                logFileSet = false;
                buttonCheck.setEnabled(false);
            }
            else
            {
                logFileSet = true;
                logPath = logText.getText();
                if (mapFileSet)
                    buttonCheck.setEnabled(true);
            }
        }
    }

    private class CheckListener implements ActionListener
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            boolean ok = true;
            
            buttonCheck.setEnabled(false);
            if (checkMapPath())
            {
                mapFile = new File(topLevelDir);
                logFile = new File(logPath);
                if (logFile.isDirectory())
                {
                    reportError("Log file is a directory");
                    ok = false;
                }
                else if (logFile.exists())
                {
                    ok = JOptionPane.showConfirmDialog(null,
                            "Log file already exists: do you want to overwrite it?",
                            "File exists",
                            JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE) == JOptionPane.YES_OPTION;
                    if (ok)
                        logFile.delete();
                }
                if (ok)
                {
                    boolean ignore = ignoreMissingFloors.isSelected();
                    checker = new RunChecks(mapFile, logFile, mapRootDir, mapRootLen, archRoot,
                                            mapText, progressText, ignore, version);
                    checker.start();
                }
            }
        }
    }

    private class ExitListener implements ActionListener
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            boolean allowExit = false;
            if ((checker != null) && checker.getActive())
            {
                if (JOptionPane.showConfirmDialog(null,
                    "This will terminate the active check.\nAre you sure?",
                    "Exit",
                    JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE) == JOptionPane.YES_OPTION)
                {
                    if (checker.Shutdown())
                        allowExit = true;
                }
            }
            else
                allowExit = true;
           
            if (allowExit)
                System.exit(0);
        }
    }
    
    private boolean checkMapPath()
    {
        boolean valid = false;
        int pos = -1;

        // Strip off trailing slash or backslash
        if (topLevelDir.matches(".+[\\\\/]$"))
        {
            topLevelDir = topLevelDir.substring(0, topLevelDir.length() - 1);
            topLevel.setText(topLevelDir);
        }

        if (topLevelDir.matches(".+[\\\\/]maps$"))
        {
            // Path ends with /maps or \maps
            mapRootDir = topLevelDir;
            valid = true;
        }
        else if (topLevelDir.matches(".+[\\\\/]maps[\\\\/]{1}.+"))
        {
            // Path contains /maps/ or \maps\
            pos = topLevelDir.indexOf("/maps/");
            if (pos == -1)
                pos = topLevelDir.indexOf("\\maps\\");
            assert (pos >= 0);
            mapRootDir = topLevelDir.substring(0, pos + 5);
            valid = true;
        }
        
        if (!valid)
            reportError("Map path must contain 'maps' directory");
        else
        {
            mapRootLen = mapRootDir.length();

            // Locate archetypes and artifacts files
            archRoot = mapRootDir.substring(0, mapRootLen-5) + File.separator + "arch" + File.separator;
            String s =  archRoot + "archetypes";
            archFile = new File(s);
            if (!archFile.exists())
            {
                reportError("Can't find archetypes file " + s);
                valid = false;
            }
            s = archRoot + "artifacts";
            artFile = new File(s);
            if (!artFile.exists())
            {
                reportError("Can't find artifacts file " + s);
                valid = false;
            }
        }
        
        return valid;
    }
    
    private void reportError(String s)
    {
        if (commandLine)
            System.out.println(s);
        else
            JOptionPane.showMessageDialog(null, s);
   }
}
