/*
 * DaiEnv.java
 *
 * Created on 13 May 2007, 11:05
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package mapchecker;

/**
 *
 * @author Jeff
 */
public class DaiEnv
{
    public int     x;
    public int     y;
    public String  arch;
    public String  name;
    
    /** Creates a new instance of DaiEnv */
    public DaiEnv(int pX, int pY, String pA)
    {
        x    = pX;
        y    = pY;
        arch = pA;
        name = null;
    }    
}
