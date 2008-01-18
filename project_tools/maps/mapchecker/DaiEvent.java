/*
 * DaiEvent.java
 *
 * Created on 10 May 2007, 14:32
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class DaiEvent
{
    private int     envLevel;
    private String  envDesc;
    private int     x;
    private int     y;
    private int     subType;
    private String  script;
    private boolean absPath;
    
    /** Creates a new instance of DaiEvent */
    public DaiEvent(int pEL, String pED, int pX, int pY, int pST, String pS, boolean pAbs)
    {
        envLevel = pEL;
        envDesc  = pED;
        x        = pX;
        y        = pY;
        subType  = pST;
        script   = pS;
        absPath  = pAbs;
    }
    
    public boolean isAbs()
    {
        return absPath;
    }
    
    public String getScriptPath()
    {
        return script;
    }
    
    public int getSubType()
    {
        return subType;
    }

    public String getEnvDesc()
    {
        return envDesc;
    }
}
