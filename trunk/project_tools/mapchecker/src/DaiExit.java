/*
 * DaiExit.java
 *
 * Created on 29 April 2007, 15:07
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class DaiExit
{
    private int     envLevel;
    private String  envDesc;
    private int     x;
    private int     y;
    private String  toPath;
    private boolean absPath;
    private int     toX;
    private int     toY;
    private boolean walk_on;
    private boolean fly_on;
    
    /**
     * Creates a new instance of DaiExit
     */
    public DaiExit(int pEL, String pED, int pX, int pY, String pToPath, boolean pAbs, int pToX, int pToY, boolean pW, boolean pF)
    {
        envLevel = pEL;
        envDesc  = pED;
        x        = pX;
        y        = pY;
        toPath   = pToPath;
        absPath  = pAbs;
        toX      = pToX;
        toY      = pToY;
        walk_on  = pW;
        fly_on   = pF;
    }
    
    public boolean isAuto()
    {
        return walk_on || fly_on;
    }
    
    public boolean isAbs()
    {
        return absPath;
    }

    public String getRefPath()
    {
        return toPath;
    }
    
    public String getEnvDesc()
    {
        return envDesc;
    }
}
