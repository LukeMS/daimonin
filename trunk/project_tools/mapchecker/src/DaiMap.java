/*
 * DaiMap.java
 *
 * Created on 28 April 2007, 21:32
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

import java.io.*;
import java.util.*;

/**
 *
 * @author Jeff
 */
public class DaiMap
{
    private File        file;
    private String      path;
    private String      name;
    private int         width = 0;
    private int         height = 0;
    private int         difficulty = 0;
    private int         darkness = 0;
    private boolean     outdoor = false;
    private String[]    tilePaths = new String[8];
    private boolean[]   tilePathAbs = new boolean[8];
    private Map<String, ArchExit> exitMap;
    private ArrayList<DaiExit> exits = new ArrayList<DaiExit>();
    private ArrayList<DaiEvent> events = new ArrayList<DaiEvent>();
    private ArrayList<DaiWaypoint> waypoints = new ArrayList<DaiWaypoint>();
    
    private String      mapRootDir;
    private File        parentDir;
    private boolean     valid = false;
    private int         envLevel = 0;
//    private DaiEnv      curEnv = new DaiEnv(0, 0, "");
    private DaiEnv      curEnv;
    private String      curArch = "";
    private EnvStack envDesc = new EnvStack();
    private enum ArchType {NONE, EXIT, EVENT, WAYPOINT};
    private String      line;
    private BufferedReader in = null;
    
    /**
     * Creates a new instance of DaiMap
     */
    public DaiMap(File fParam, String rootDir, Map<String, ArchExit> exMap)
    {
        int     i;
        String  s;
        String  relName;
        String  archName;

        file = fParam;
        parentDir = file.getParentFile();
        mapRootDir = rootDir;
        exitMap = exMap;
        
       for (i = 0; i < 8; i++)
           tilePathAbs[i] = false;
        
        try
        {
            in = new BufferedReader(new FileReader(file));
        }
        catch (FileNotFoundException ex)
        {
            ex.printStackTrace();
        }
        try
        {
            path = file.getCanonicalPath();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        
        line = getLine();

        // First line must be "arch map"
        if ((line != null) && line.equals("arch map"))
        {
            // Build map info
            while (line != null)
            {
                line = getLine();
                if (line.startsWith("name "))
                    name = line.substring(5);
                else if (line.startsWith("width "))
                    width = Integer.parseInt(line.substring(6));
                else if (line.startsWith("height "))
                    height = Integer.parseInt(line.substring(7));
                else if (line.startsWith("difficulty "))
                    difficulty = Integer.parseInt(line.substring(11));
                else if (line.startsWith("outdoor "))
                {
                    i = Integer.parseInt(line.substring(8));
                    if (i == 1)
                        outdoor = true;
                }
                else if (line.startsWith("darkness "))
                    darkness = Integer.parseInt(line.substring(9));
                else if (line.startsWith("tile_path_"))
                {
                    i = Integer.parseInt(line.substring(10, 11));
                    relName = line.substring(12);

                    // Check for absolute reference
                    if (relName.substring(0, 1).equals("/"))
                        tilePathAbs[i-1] = true;
                    tilePaths[i-1] = getNormalisedPath(relName);
                }
                else if (line.equals("end"))
                {
                    valid = true;
                    break;
                }
            }

            // Build exits, events and waypoints
            if (valid)
            {
                line = getLine();
                while (line != null)
                {
                    s = line.substring(5);
                    if (line.equals("end"))
                    {
                        envLevel--;
                        assert (envLevel >= 0);
                        curEnv = envDesc.pop();
                    }
                    else if (line.startsWith("arch "))
                    {
                        processArch(0, 0);
                    }
                    line = getLine();
                }
            }
        }
        try
        {
            // Essential to close file to release resources
            in.close();
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
    }
    
    private void processArch(int curX, int curY)
    {
        int         dstX = 0;
        int         dstY = 0;
        int         subType = 0;
        String      slaying = "";
        String      race = "";
        ArchExit    archEx = null;
        ArchType    curArchType = ArchType.NONE;
        String      desc;
        boolean     isAbs = false;

        String archName = line.substring(5);   // arch name
        envLevel++;
        curArch = archName;
        curEnv = new DaiEnv(curX, curY, archName);
        envDesc.push(curEnv);
        curArchType = ArchType.NONE;
        if (exitMap.containsKey(archName))
        {
            curArchType = ArchType.EXIT;
            archEx = exitMap.get(archName);
        }
        else if (archName.equals("event_obj"))
            curArchType = ArchType.EVENT;
        else if (archName.equals("waypoint"))
            curArchType = ArchType.WAYPOINT;
        
        while (line != null)
        {
            line = getLine();
            if (line.startsWith("arch "))
                processArch(curX, curY);
            else if (line.equals("end"))
            {
                desc = envDesc.getDesc();
                switch (curArchType)
                {
                    case NONE:
                        // Do nothing
                        break;
                        
                    case EXIT:
                        exits.add(new DaiExit(envLevel, desc, curEnv.x, curEnv.y, slaying, isAbs, dstX, dstY,
                                                archEx.getWalkOn(), archEx.getFlyOn()));
                        break;
                        
                    case EVENT:
                        events.add(new DaiEvent(envLevel, desc, curEnv.x, curEnv.y, subType, race, isAbs));
                        break;
                        
                    case WAYPOINT:
                        waypoints.add(new DaiWaypoint(envLevel, desc, curEnv.x, curEnv.y, slaying, isAbs, dstX, dstY));
                        break;
                }
                envLevel--;
                assert envLevel >= 0;
                curEnv = envDesc.pop();
                return;
            }
            // Always set x & y if they occur
            else if (line.startsWith("x "))
            {
                curX = Integer.parseInt(line.substring(2));
                curEnv.x = curX;
            }
            else if (line.startsWith("y "))
            {
                curY = Integer.parseInt(line.substring(2));
                curEnv.y = curY;
            }
            else if (line.startsWith("name "))
                curEnv.name = line.substring(5);
            else if (curArchType == ArchType.NONE)      // shortcut
                continue;
            else if (line.startsWith("hp "))
                dstX = Integer.parseInt(line.substring(3));
            else if (line.startsWith("sp "))
                dstY = Integer.parseInt(line.substring(3));
            else if (line.startsWith("sub_type "))
                subType = Integer.parseInt(line.substring(9));
            else if (line.startsWith("slaying ") && (curArchType != ArchType.EVENT))
            {
                isAbs = isAbsPath(8);
                slaying = getNormalisedPath(line.substring(8));
           }
           else if (line.startsWith("race ") && (curArchType == ArchType.EVENT))
           {
                isAbs = isAbsPath(5);
                race = getNormalisedPath(line.substring(5));
           }
        }
    }
    
    private String getLine()
    {
        String localLine = null;
        try
        {
            localLine = in.readLine();
            
            // Eat message blocks
            if ((localLine != null) && localLine.startsWith("msg"))
            {
                while ((localLine != null) && !localLine.startsWith("endmsg"))
                    localLine = in.readLine();
                localLine = in.readLine();
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }
        return localLine;
    }

    private boolean isAbsPath(int n)
    {
        return (line.substring(n, n+1).equals("/"));
    }
    
    private String getNormalisedPath(String s)
    {
        File    f;
        String  localPath = "";
        if (s.substring(0, 1).equals("/"))
        {
            f = new File(mapRootDir, s.substring(1));
        }
        else
        {
            f = new File(parentDir, s);
        }
        try
        {
            localPath = f.getCanonicalPath();
        }
        catch (IOException ex)
        {
//            ex.printStackTrace();
            localPath = s;
        }
        return localPath;
    }
    /** Returns true if the file is a valid map file */
    public boolean isValid()
    {
        return valid;
    }
    
    public String getPath()
    {
        return path;
        
    }
    
    public String getTilePath(int n)
    {
        return tilePaths[n];
    }
    
    public boolean getTilePathAbs(int n)
    {
        return tilePathAbs[n];
    }
    
    public File getFile()
    {
        return file;
    }
    
    public DaiExit getExit(int n)
    {
        return exits.get(n);
    }
    
    public int getNumExits()
    {
        return exits.size();
    }
    
    public DaiEvent getEvent(int n)
    {
        return events.get(n);
    }
    
    public int getNumEvents()
    {
        return events.size();
    }
    
    public DaiWaypoint getWaypoint(int n)
    {
        return waypoints.get(n);
    }

    public int getNumWaypoints()
    {
        return waypoints.size();
    }
    
    public int getWidth()
    {
        return width;
    }
    
    public int getHeight()
    {
        return height;
    }
}
