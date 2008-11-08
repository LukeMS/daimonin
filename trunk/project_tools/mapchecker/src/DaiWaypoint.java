/*
 * DaiWaypoint.java
 *
 * Created on 10 May 2007, 14:33
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class DaiWaypoint
{
    private int     envLevel;
    private String  envDesc;
    private int     x;
    private int     y;
    private String  dstMap;
    private boolean absPath;
    private int     dstX;
    private int     dstY;
    
    /** Creates a new instance of DaiWaypoint */
    public DaiWaypoint(int pEL, String pED, int pX, int pY, String pDM, boolean pAbs, int pDX, int pDY)
    {
        envLevel = pEL;
        envDesc  = pED;
        x        = pX;
        y        = pY;
        dstMap   = pDM;
        absPath  = pAbs;
        dstX     = pDX;
        dstY     = pDY;
    }
    
    public boolean isAbs()
    {
        return absPath;
    }

    public String getEnvDesc()
    {
        return envDesc;
    }

    public String getRefPath()
    {
        return dstMap;
    }
}
