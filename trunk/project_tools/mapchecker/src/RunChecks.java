/*
 * RunChecks.java
 *
 * Created on 22 April 2007, 19:49
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

import java.text.SimpleDateFormat;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import java.io.*;
import java.util.*;

/**
 *
 * @author Jeff
 */
public class RunChecks extends Thread
{
    private JTextArea progress;
    private File logF;
    private String logFilePath;
    private PrintWriter log;
    private File mapF;
    private String archRootDir;
    private String mapFilePath;
    private String mapRootDir;
    private int mapRootLen;
    private boolean checkActive = false;
    private ArrayList<DaiMap> maps = new ArrayList<DaiMap>();
    private Map<String, Integer> mapToMap = new HashMap<String, Integer>();
    private ArrayList<DaiObject> archList = new ArrayList<DaiObject>();
    private Map<String, DaiObject> archMap = new HashMap<String, DaiObject>();
    private Map<String, ArchExit> exitMap = new HashMap<String, ArchExit>();
    private Map<String, DaiArtifact> artMap = new HashMap<String, DaiArtifact>();
    private int  mapCount = 0;
    private final int[] crossRef = {2, 3, 0, 1, 6, 7, 4, 5};
    private final String[] tileCardinal = {"N", "E", "S", "W", "NE", "SE", "SW", "NW"};
    private boolean checkPathLogged = false;
    private int warnings = 0;
    private int errors = 0;
    private int archCount = 0;
    private JTextField mapT;
    private String[] scriptPathExceptions = {"scripts", "scripts_private", "special", "dmbuster"};

    // For map square and artifact error reporting
    private boolean errorReported = false;

    // For map square checks
    private BufferedReader inMap;
    private String line;
    private int mapSizeX;
    private int mapSizeY;
    private boolean ignoreMissingFloors;
    
    private String version;
    
    /** Creates a new instance of RunChecks */
    public RunChecks(File mapFile, File logFile, String rootDir, int rootLen, String archRoot, JTextField mapText,
                        JTextArea progressText, boolean ignore, String v)
    {
        mapF = mapFile;
        logF = logFile;
        archRootDir = archRoot;
        mapRootDir = rootDir;
        mapRootLen = rootLen;
        mapT = mapText;
        progress = progressText;
        ignoreMissingFloors = ignore;
        version = v;
    }
    
    @Override
    public void run()
    {
        checkActive = true;

        try
        {
            logFilePath = logF.getCanonicalPath();
        } catch (IOException ex)
        {
            ex.printStackTrace();
        }
        Progress("Creating log file...");
        if (logF.isDirectory())
        {
            // If we got here, it must be from the command line, since the
            // GUI handles this case. Just give error and return.
            Progress("Log file cannot be a directory");
            return;
        }
        else if (logF.exists())
        {
            // If we got here, it must be from the command line, since the file
            // is deleted earlier when using the GUI.
            // Just delete the old file now.
            logF.delete();
            Progress("Deleted old log file.");
        }
        try
        {
            if (logF.createNewFile())
            {
                Progress("Log file created.");
                log = new PrintWriter(new BufferedWriter(new FileWriter(logF)));
                SimpleDateFormat dateFormatter =  new SimpleDateFormat("yyyy.MM.dd kk:mm:ss");
                String date = dateFormatter.format(new Date());
                log.println("MapChecker " + version + " - log file created on " + date);
            }
            else
            {
                Progress("Failed to create log file");
                return;
            }        
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        try
        {
            
            mapFilePath = mapF.getCanonicalPath();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        
        // Build the archetypes and exit lists
        LogProgress("Building archetypes list from archetypes file...", true);
        buildArchList();
        LogProgress(archCount + " archetypes found.", false);
        
        // Check that other_arch exists where specified
        LogProgress("Checking other_arch in archetypes", true);
        checkOtherArch();

        // Build the artifacts list
        LogProgress("Building artifacts list from artifacts files...", true);
        doArtFile(archRootDir + "artifacts");
        appendArtifacts(new File(mapRootDir));

        // Build the map tree
        LogProgress("Checking maps in " + mapFilePath + " and below.", true);
        LogProgress("Collecting map info...", false);
        if (buildTree(mapF))
        {
            LogProgress("...done.", false);
            
            // Set number of maps
            mapCount = maps.size();
            LogProgress(mapCount + " maps found.", false);
            
            // Check maps
            LogProgress("Checking tile paths...", true);
            for (DaiMap map : maps)
                checkTilePaths(map);
            LogProgress("Checking exits...", true);
            for (DaiMap map : maps)
                checkExits(map);
            LogProgress("Checking events...", true);
            for (DaiMap map : maps)
                checkEvents(map);
            LogProgress("Checking waypoints...", true);
            for (DaiMap map : maps)
                checkWaypoints(map);
            LogProgress("Checking map squares...", true);
            for (DaiMap map : maps)
                checkMapsquares(map);
        }
        else
        {
            FinishCheck("Failed to build tree.");
            return;
        }
        
        // Map checking complete
        LogProgress("\r\n" + warnings + " warnings\r\n" + errors + " errors\r\n", false);
        FinishCheck("Map checks completed.");
    }
    
    private void Progress(String text)
    {
        if (progress != null)
        {
            progress.append(text + "\n");
            progress.setCaretPosition(progress.getDocument().getLength());
        }
        else
            System.out.println(text);
    }

    private void LogProgress(String text, boolean logLine)
    {
        Progress(text);
        if (log != null)
        {
            if (logLine)
                log.println();
            log.println(text);
        }
    }
    
    public boolean getActive()
    {
        return checkActive;
    }
    
    public boolean Shutdown()
    {
        FinishCheck("Check interrupted by Exit.");
        checkActive = false;
        return true;
    }
    
    private void FinishCheck (String strReason)
    {
        LogProgress(strReason, false);
        if (log != null)
        {
            log.println("Log file closed.");
            log.close();
        }
        checkActive = false;
    }
    
    private void buildArchList()
    {
        BufferedReader in;
        String archLine = null;
        String object = "";
        String name   = "";
        String title  = "";
        String other_arch = "";
        boolean walk_on = false;
        boolean fly_on = false;
        boolean system = false;
        int type    = 0;
        int layer   = 0;
        DaiObject obj;
        final int exitType = 66;
        final String s = archRootDir + "archetypes";
        try
        {
            in = new BufferedReader(new FileReader(new File(s)));
        }
        catch (FileNotFoundException ex)
        {
            log.println("Could not open archetypes file " + s);           
            return;
        }

        archLine = getLine(in);
        while (archLine != null)
        {

            if (archLine.startsWith("Object"))
            {
                object = archLine.substring(7);
                if (object.startsWith("floor"))
                {
                    int i = 0;
                }
                walk_on    = false;
                fly_on     = false;
                system     = false;
                type       = 0;
                layer      = 0;
                name       = "";
                title      = "";
                other_arch = "";
            }
            else if (archLine.startsWith("name"))
                name = archLine.substring(5);
            else if (archLine.startsWith("title"))
                title = archLine.substring(6);
            else if (archLine.startsWith("other_arch"))
                other_arch = archLine.substring(11);
            else if (archLine.startsWith("layer"))
                layer = Integer.parseInt(archLine.substring(6));
            else if (archLine.startsWith("type"))
                type = Integer.parseInt(archLine.substring(5));
            else if (archLine.startsWith("walk_on"))
                walk_on = Integer.parseInt(archLine.substring(8)) == 1;
            else if (archLine.startsWith("fly_on"))
                fly_on = Integer.parseInt(archLine.substring(7)) == 1;
            else if (archLine.startsWith("sys_object"))
                system = Integer.parseInt(archLine.substring(11)) == 1;
            else if (archLine.equals("end"))
            {
                if (type == exitType)
                    exitMap.put(object, new ArchExit(object, walk_on, fly_on));
                obj = new DaiObject(object, name, title, other_arch, type, layer, 0, system);
                archList.add(obj);
                archMap.put(object, obj);
                archCount++;
            }
            archLine = getLine(in);
        }

        try
        {
            in.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }
 
    private void checkOtherArch()
    {
        String other;
        
        for (DaiObject arch : archList)
        {
            other = arch.getOtherArch();
            if ((other.length() > 0) && !archMap.containsKey(other))
            {
                log.println("**Error: " + arch.getObject() + " references other_arch " + other + ", but this does not exist.");
                errors++;
            }
        }
    }

    // Append artifacts from .art files
    private void appendArtifacts(File mapRoot)
    {
        String s;
        
        File[]  dirList = mapRoot.listFiles();
        if (dirList == null)
        {
            FinishCheck("Could not list files in directory " + mapRoot.getPath());
        }
        else
        {
            for (File f : dirList)
            {
                s = f.getName();
                if (f.isDirectory())
                {
                    if (s.equals(".svn"))
                        continue;           // ignore .svn directories
                    else
                        appendArtifacts(f); // recurse down the tree
                }
                else
                {
                    if (s.contains(".art"))
                    {
                        // This is an artifacts file - process it
                        doArtFile(f.getPath());
                    }
                }
            }
        }
    }
 
    private String getLine(BufferedReader in)
    {
        String archLine = null;
        try
        {
            archLine = in.readLine();

            // Eat message blocks
            if ((archLine != null) && archLine.startsWith("msg"))
            {
                while ((archLine != null) && !archLine.startsWith("endmsg"))
                    archLine = in.readLine();
                archLine = in.readLine();
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        return archLine;
    }
    
    private boolean buildTree(File map)
    {
        boolean ok = false;
        String  s;
        DaiMap  m;
        
        File[]  dirList = map.listFiles();
        if (dirList == null)
        {
            FinishCheck("Could not list files in directory " + map.getPath());
        }
        else
        {
            for (File f : dirList)
            {
                s = f.getName();
                if (f.isDirectory())
                {
                    if (s.equals(".svn"))
                        continue;       // ignore .svn directories
                    else
                        buildTree(f);   // recurse down the tree
                }
                else
                {
                    if (s.contains("."))
                    {
                        continue;       // ignore files with extensions - these are not maps
                    }
                    else
                    {
                        // This should be a map file
                        m = new DaiMap(f, mapRootDir, exitMap);
                        if (m.isValid())
                        {
                            maps.add(m);
                            mapToMap.put(m.getPath(), mapCount);
                            if (mapT != null)
                            {
                                mapT.setText(m.getPath().substring(mapRootLen));
                                mapT.setCaretPosition(mapT.getDocument().getLength());
                            }
                            mapCount++;
                        }
                    }
                }
            }
            ok = true;
        }
        return ok;
    }

    // Process a .art file
    private void doArtFile(String s)
    {
        BufferedReader  in = null;
        DaiObject arch;
        String artLine = null;
        String artifact = "";
        String def_arch   = "";
        String name = "";
        String title = "";
        int layer = 0;
        int type = 0;
 
        errorReported = false;
        LogProgress("Processing artifacts file " + s, false);
        
        try
        {
            in = new BufferedReader(new FileReader(new File(s)));
        }
        catch (FileNotFoundException ex)
        {
            log.println("Could not open artifacts file " + s);
            return;
        }

        artLine = getLine(in);
        while (artLine != null)
        {
            if (artLine.startsWith("#"))
            {
                // ignore comment
            }
            else if (artLine.startsWith("Allowed"))
            {
                artifact = "";
                def_arch = "";
                name = "";
                title = "";
                layer = 0;
                type = 0;
            }
            else if (artLine.startsWith("artifact"))
                artifact = artLine.substring(9);
            else if (artLine.startsWith("def_arch"))
                def_arch = artLine.substring(9);
            else if (artLine.startsWith("name"))
                name = artLine.substring(5);
            else if (artLine.startsWith("title"))
                title = artLine.substring(6);
            else if (artLine.startsWith("layer"))
                layer = Integer.parseInt(artLine.substring(6));
            else if (artLine.startsWith("type"))
                type = Integer.parseInt(artLine.substring(5));
            else if (artLine.equals("end"))
            {
                if (archMap.containsKey(def_arch))
                {
                    arch = archMap.get(def_arch);
                    if (artMap.containsKey(artifact))
                    {
                        logArtFileError(s, artifact, "duplicates artifact in " + artMap.get(artifact).getPath());
                    }
                    else if (archMap.containsKey(artifact))
                    {
                        logArtFileError(s, artifact, "has same name as an archetype.");
                    }
                    else
                    {
                        if (name.isEmpty())
                            name = arch.getName();
                        if (title.isEmpty())
                            title = arch.getTitle();
                        artMap.put(artifact, new DaiArtifact(artifact, name, title, def_arch, s, type, layer));
                    }
                }
                else
                {
                    logArtFileError(s, artifact, "references default archetype " + def_arch
                            + "\r\n\tbut archetype does not exist.");
                }
            }
            artLine = getLine(in);
        }
        try
        {
            in.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }
    
    // Report an artifact file error
    void logArtFileError(String path, String arch, String text)
    {
        if (!errorReported)
        {
            errorReported = true;
            log.println("\r\nArtifact file " + path);
        }
        
        log.println("**Error: artifact " + arch + " " + text);
        errors++;
    }
    
    // Check a single map
    private void checkTilePaths(DaiMap map)
    {
        DaiMap  refMap;
        String  refPath;
        String  shortRefPath;
        String  thisPath;
        String  shortPath;
        String  backRef;
        String  shortBackRef;
        int     tileIndex;
        int     mapIndex;
        
        thisPath = map.getPath();
        shortPath = thisPath.substring(mapRootLen);
        if (mapT != null)
        {
            mapT.setText(shortPath);
            mapT.setCaretPosition(mapT.getDocument().getLength());
        }
    
        // Check tile paths
        for (tileIndex = 0; tileIndex < 8; tileIndex++)
        {
            refPath = map.getTilePath(tileIndex);
            if (refPath != null)
            {
                shortRefPath = refPath.substring(mapRootLen);
                if (!mapToMap.containsKey(refPath))
                {
                    logTileFirstPart(shortPath, tileIndex, shortRefPath);
                    log.println("\tbut map does not exist.");
                    errors++;
                }
                else
                {
                    mapIndex = mapToMap.get(refPath);
                    refMap = maps.get(mapIndex);
                    backRef = refMap.getTilePath(crossRef[tileIndex]);
                    if (backRef == null)
                    {
                        logTileFirstPart(shortPath, tileIndex, shortRefPath);
                        log.println("\tbut that map has no "
                                + tileCardinal[crossRef[tileIndex]] + " connection.");
                        errors++;
                    }
                    else if (!backRef.equals(thisPath))
                    {
                        shortBackRef = backRef.substring(mapRootLen);
                        logTileFirstPart(shortPath, tileIndex, shortRefPath);
                        log.println("\tbut that map's " + tileCardinal[crossRef[tileIndex]] +
                                " tile path connects to " + shortBackRef);
                        errors++;
                    }
                }
                
                if (map.getTilePathAbs(tileIndex))
                {
                    // Warn absolute reference
                    log.println();
                    log.println("**Warning: map " + shortPath + " " + tileCardinal[tileIndex] + " tile path");
                    log.println("\tis an absolute reference.");
                    warnings++;
                }
            }
        }
    }

    private void logTileFirstPart(String shortPath, int tileIndex, String shortRefPath)
    {
        log.println();
        log.println("**Error: map " + shortPath + " " + tileCardinal[tileIndex] + " tile path");
        log.println("\tconnects to map " + shortRefPath + ",");
    }

    private void checkExits(DaiMap map)
    {
        DaiMap  refMap;
        String  refPath;
        String  shortRefPath;
        String  thisPath;
        String  shortPath;
        int     exitIndex;
        int     mapIndex;
        DaiExit exit;
        
        thisPath = map.getPath();
        shortPath = thisPath.substring(mapRootLen);
        if (mapT != null)
        {
            mapT.setText(shortPath);
            mapT.setCaretPosition(mapT.getDocument().getLength());
        }
    
        // Check exits
        for (exitIndex = 0; exitIndex < map.getNumExits(); exitIndex++)
        {
            exit = map.getExit(exitIndex);
            refPath = exit.getRefPath();
            if (refPath.length() > 0)
            {
                shortRefPath = refPath.substring(mapRootLen);
                if (!mapExists(refPath))
                {
                    logExitFirstPart("**Error", exit, shortPath, shortRefPath);
                    log.println("\tbut map does not exist.");
                    errors++;
                }
                else
                {
                }
                
                if (exit.isAbs())
                {
                    // Warn absolute reference
                    logExitFirstPart("**Warning", exit, shortPath, shortRefPath);
                    log.println("\tbut reference is absolute.");
                    warnings++;
                }
            }
        }
    }
    
    private void logExitFirstPart(String which, DaiExit exit, String shortPath, String shortRefPath)
    {
        log.println();
        log.println(which + ": Exit on map " + shortPath + " [" + exit.getEnvDesc() + "]");
        if (shortRefPath != null)
            log.println("\treferences map " + shortRefPath + ",");
    }
    
    private void checkEvents(DaiMap map)
    {
        DaiMap  refMap;
        String  scriptPath;
        String  shortScriptPath;
        String  thisPath;
        String  shortPath;
        String  firstDir;
        int     i;
        int     eventIndex;
        int     mapIndex;
        DaiEvent event;
        File    f;
        boolean ignoreWarning;
        
        
        thisPath = map.getPath();
        shortPath = thisPath.substring(mapRootLen);
        if (mapT != null)
        {
            mapT.setText(shortPath);
            mapT.setCaretPosition(mapT.getDocument().getLength());
        }
    
        // Check events
        for (eventIndex = 0; eventIndex < map.getNumEvents(); eventIndex++)
        {
            event = map.getEvent(eventIndex);
            scriptPath = event.getScriptPath();
            if (scriptPath.length() > 0)
            {
                f = new File(scriptPath);
                shortScriptPath = scriptPath.substring(mapRootLen);
                if (!f.exists())
                {
                    logEventFirstPart("**Error", event, shortPath, shortScriptPath);
                    log.println("\tbut script file does not exist.");
                    errors++;
                }
                else
                {
                }
                
                if (event.isAbs())
                {
                    // Warn absolute reference - but only if first directory
                    // is not in exceptions
                    ignoreWarning = false;
                    firstDir = shortScriptPath.substring(1);
                    i = firstDir.indexOf(File.separatorChar);
                    if (i > 0)      // trap problems
                    {
                        firstDir = firstDir.substring(0, i);
                        for (String s : scriptPathExceptions)
                        {
                            if (firstDir.equalsIgnoreCase(s))
                            {
                                ignoreWarning = true;
                                break;
                            }
                        }
                    }

                    if (!ignoreWarning)
                    {
                    logEventFirstPart("**Warning", event, shortPath, shortScriptPath);
                    log.println("\tbut reference is absolute.");
                    warnings++;
                    }
                }
            }
        }
    }
    
    private void logEventFirstPart(String which, DaiEvent event, String shortPath, String shortScriptPath)
    {
        log.println("\r\n" + which + ": Event on map " + shortPath + " [" + event.getEnvDesc() + "]");
        if (shortScriptPath != null)
            log.println("\treferences script " + shortScriptPath + ",");
    }

    private void checkWaypoints(DaiMap map)
    {
        DaiMap  refMap;
        String  refPath;
        String  shortRefPath;
        String  thisPath;
        String  shortPath;
        int     waypointIndex;
        int     mapIndex;
        DaiWaypoint waypoint;
        
        thisPath = map.getPath();
        shortPath = thisPath.substring(mapRootLen);
        if (mapT != null)
        {
            mapT.setText(shortPath);
            mapT.setCaretPosition(mapT.getDocument().getLength());
        }
    
        // Check exits
        for (waypointIndex = 0; waypointIndex < map.getNumWaypoints(); waypointIndex++)
        {
            waypoint = map.getWaypoint(waypointIndex);
            refPath = waypoint.getRefPath();
            if (refPath.length() > 0)
            {
                shortRefPath = refPath.substring(mapRootLen);
                if (!mapExists(refPath))
                {
                    logWaypointFirstPart("**Error", waypoint, shortPath, shortRefPath);
                    log.println("\tbut map does not exist.");
                    errors++;
                }
                else
                {
                }
                
                if (waypoint.isAbs())
                {
                    // Warn absolute reference
                    logWaypointFirstPart("**Warning", waypoint, shortPath, shortRefPath);
                    log.println("\tbut reference is absolute.");
                    warnings++;
                }
            }
        }
    }

    private void logWaypointFirstPart(String which, DaiWaypoint waypoint, String shortPath, String shortRefPath)
    {
        log.println("\r\n" + which + ": Waypoint on map " + shortPath + " [" + waypoint.getEnvDesc() + "]");
        if (shortRefPath != null)
            log.println("\treferences map " + shortRefPath + ",");
    }
    
    private void checkMapsquares(DaiMap map)
    {
        String          thisPath;
        String          shortPath;
        int             curX = 0;
        int             curY = 0;
        int             x, y;
        int             i, j;
        DaiObjectList   list;
        int             listSize;
        int             count;
        int             type;
        int             iLayer;
        boolean         valid = true;
        boolean         error;
        boolean         hasSpawnPoint;
        boolean         hasMob;
        boolean         hasSpMob;
        DaiObject       iObj, jObj;
        
        final int       typeFloor = 71;
        final int       typeSpawn = 81;
        final int       typeMob   = 80;
        final int       typeSpMob = 83;
        
        // No error reported for this map yet
        errorReported = false;
        
        // Store map dimensions
        mapSizeX = map.getWidth();
        mapSizeY = map.getHeight();
        
        // Create and initialize the list
        DaiObjectList[][] objlist = new DaiObjectList[mapSizeX][mapSizeY];
        for (x = 0; x < mapSizeX; x++)
            for (y = 0; y < mapSizeY; y++)
                objlist[x][y] = new DaiObjectList();

        thisPath = map.getPath();
        shortPath = thisPath.substring(mapRootLen);
        if (mapT != null)
        {
            mapT.setText(shortPath);
            mapT.setCaretPosition(mapT.getDocument().getLength());
        }
        
        try
        {
            inMap = new BufferedReader(new FileReader(map.getFile()));
        }
        catch (FileNotFoundException ex)
        {
            ex.printStackTrace();
        }
    
        line = getLine(inMap);

        // First line must be "arch map"
        if ((line != null) && line.equals("arch map"))
        {
            // Build map info
            line = getLine(inMap);
            while (valid && line != null)
            {
                if (line.startsWith("arch "))
                {
                    valid = processArch(objlist, shortPath, 0, 0, 0);
                }
                line = getLine(inMap);
            }
        }
        else
        {
            valid = false;
        }
        
        try
        {
            // Essential to close file to release resources
            inMap.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        if (!valid)
        {
            log.println("\r\nError: Map "+ shortPath + " has errors and cannot be processed.");
            errors++;
            
        }
        
        // OK. We now have an array with a list of objects on each square.
        // Let's do some checks...
        for (x = 0; x < mapSizeX; x++)
        {
            for (y = 0; y < mapSizeY; y++)
            {
                list = objlist[x][y];
                listSize = list.size();
                
                // Ignore totally empty squares
                if (listSize > 0)
                {
                    // For each square with objects, count the number of floors (= layer 1).
                    // At the same time, look for spawn points and mobs.
                    count = 0;
                    hasSpawnPoint   = false;
                    hasMob          = false;
                    hasSpMob        = false;
                    for (DaiObject obj : list)
                    {
                        type = obj.getType();
                        if (type == typeSpawn)
                            hasSpawnPoint = true;
                        else if (type == typeMob)
                            hasMob = true;
                        else if (type == typeSpMob)
                            hasSpMob = true;
                        else if (obj.getLayer() == 1)
                            count++;
                        
                        // Check sys object on layer 0
                        if (obj.isSysObject() && (obj.getLayer() != 0))
                        {
                            logMapsquareError("Warning", shortPath, x, y, "system object not on layer 0");
                            warnings++;
                        }
                    }
                    if (count == 0)
                    {
                        // No floor
                        if (!ignoreMissingFloors)
                        {
                            logMapsquareError("Warning", shortPath, x, y, "has objects but no floor");
                            warnings++;
                        }
                    }
                    else if (count > 1)
                    {
                        // More than 1 floor
                        logMapsquareError("Error", shortPath, x, y, "more than one floor");
                        errors++;
                    }

                    // Check spawn points and mobs
                    if (hasSpawnPoint && !hasSpMob)
                    {
                        // Empty spawn point
                        logMapsquareError("Warning", shortPath, x, y, "empty spawn point");
                        warnings++;
                    }
                    if (hasMob)
                    {
                        // Missing spawn point
                        logMapsquareError("Error", shortPath, x, y, "monster/NPC not in spawn point");
                        errors++;
                    }

                    // Look for map-level objects on same layer (apart from 0 and 1)
                    error = false;
                    for (i = 0; i < listSize - 1; i++)
                    {
                        iObj = list.get(i);
                        iLayer = iObj.getLayer();
                        if ((iObj.getEnvLevel() > 0) || (iLayer < 2))
                            continue;
                        for (j = i + 1; j < listSize; j++)
                        {
                            jObj = list.get(j);
                            if ((jObj.getEnvLevel() == 0) && (jObj.getLayer() == iLayer))
                            {
                                logMapsquareError("Warning", shortPath, x, y, "more than one object on same layer");
                                warnings++;
                                error = true;
                                break;
                            }
                        }
                        if (error)
                            break;
                    }
                }
            }
        }
    }

    private boolean processArch(DaiObjectList[][]list, String shortPath, int curX, int curY, int envLevel)
    {
        DaiObject arch     = null;
        DaiArtifact art    = null;
        String  object     = line.substring(5);   // object name
        String  name       = "";
        String  title      = "";
        String  other_arch = "";
        String def_arch    = "";
        int     type       = 0;
        int     layer      = 0;
        boolean system     = false;
        boolean valid      = true;
        boolean exists     = true;
        
        // Get defaults from arch
        if (archMap.containsKey(object))
        {
            arch  = archMap.get(object);
            name  = arch.getName();
            title = arch.getTitle();
            type  = arch.getType();
            layer = arch.getLayer();
        }
        else if (artMap.containsKey(object))
        {
            // Get info from artifact
            art   = artMap.get(object);
            arch  = archMap.get(art.getDefArch());
            name  = art.getName();
            title = art.getTitle();
            type  = art.getType();
            layer = art.getLayer();
        }
        else
        {
            // Delay error and continue processing this arch's attributes
            exists = false;
        }
        
        if (exists)
        {
            other_arch = arch.getOtherArch();
            system  = arch.isSysObject();
        }

        while (line != null)
        {
            line = getLine(inMap);
            if (line.startsWith("arch "))
            {
                if (!processArch(list, shortPath, curX, curY, envLevel + 1))
                    return false;
            }
            else if (line.equals("end"))
            {
                if (!exists)
                {
                    logMapsquareError("Error", shortPath, curX, curY, "object " + object + " does not exist!");
                    errors++;
                }

                if ((curX < 0) || (curY < 0) || (curX >= mapSizeX) || (curY >= mapSizeY))
                    valid = false;
                else
                    list[curX][curY].add(new DaiObject(object, name, title, other_arch, type, layer, envLevel, system));
                return valid;
            }
            else if (line.startsWith("x "))
                curX = Integer.parseInt(line.substring(2));
            else if (line.startsWith("y "))
                curY = Integer.parseInt(line.substring(2));
            else if (line.startsWith("name "))
                name = line.substring(5);
            else if (line.startsWith("title "))
                title = line.substring(6);
            else if (line.startsWith("type "))
                type = Integer.parseInt(line.substring(5));
            else if (line.startsWith("layer "))
                layer = Integer.parseInt(line.substring(6));
            else if (line.startsWith("sys_object "))
                system = Integer.parseInt(line.substring(11)) == 1;
        }
        return false;
    }
    
    private void logMapsquareError(String which, String shortPath, int x, int y, String text)
    {
        if (!errorReported)
        {
            errorReported = true;
            log.println("\r\nMap "+ shortPath);
        }
        
        log.println("**" + which + ": square (" + x + ", " + y + ") " + text);
    }
    
    private boolean mapExists(String refPath)
    {
        boolean ok = false;
        File    f;
        
        ok = mapToMap.containsKey(refPath);
        if (!ok)
        {
            f = new File(refPath);
            ok = f.exists();
        }
        
        return ok;
    }
}
